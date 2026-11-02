#include <tx/tasks.hpp>
#include <tx/sleep.hpp>

#include "task_internal.hpp"

#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>

namespace tx::task {
namespace {

using internal::State;
using internal::Task;

std::mutex registry_mutex;
std::unordered_map<taskid_t, std::shared_ptr<Task>> tasks;

std::atomic<taskid_t> next_id{1};
std::atomic<bool> shutting_down{false};

thread_local Task* current_task = nullptr;

// Find a task without exposing the registry.
std::shared_ptr<Task> find_task(taskid_t id)
{
    std::lock_guard<std::mutex> lock(registry_mutex);

    auto it = tasks.find(id);

    if (it == tasks.end())
        return nullptr;

    return it->second;
}

// Generate a nonzero, unused task ID.
taskid_t generate_id()
{
    for (;;) {
        taskid_t id = next_id.fetch_add(1);

        if (id == 0)
            continue;

        std::lock_guard<std::mutex> lock(registry_mutex);

        if (tasks.find(id) == tasks.end())
            return id;
    }
}

// Join a worker without holding its task mutex.
void join_worker(const std::shared_ptr<Task>& task)
{
    std::thread worker;

    {
        std::lock_guard<std::mutex> lock(task->mutex);

        if (!task->worker.joinable() || task->worker_joined)
            return;

        if (task->worker.get_id() == std::this_thread::get_id())
            return;

        task->worker_joined = true;
        worker = std::move(task->worker);
    }

    if (worker.joinable())
        worker.join();
}

// Execute the task.
void execute_task(const std::shared_ptr<Task>& task)
{
    current_task = task.get();

    void* result = nullptr;
    bool failed = false;

    try {
        result = task->call(task->args);
    }
    catch (...) {
        failed = true;
    }

    // A cancellation request takes precedence over normal completion.
    const bool cancelled =
        task->cancel_requested.load(std::memory_order_acquire);

    {
        std::lock_guard<std::mutex> lock(task->mutex);

        if (cancelled) {
            task->state = State::Cancelled;

            if (result && task->deleter) {
                try {
                    task->deleter(result);
                }
                catch (...) {
                    // Destructors/deleters must not escape the worker.
                }
            }
        }
        else if (failed) {
            task->state = State::Failed;
        }
        else {
            task->result = result;
            task->state = State::Completed;
        }
    }

    // The completion callback runs on the worker thread.
    if (!cancelled && !failed && task->callee) {
        try {
            task->callee(result);
        }
        catch (...) {
            // A callback exception must not terminate the process.
        }
    }

    current_task = nullptr;
    task->condition.notify_all();
}

// Submit a task.
taskid_t submit(
    Calling call,
    Args args,
    Callee callee,
    ResultDeleter deleter)
{
    if (!call || shutting_down.load())
        return 0;

    auto task = std::make_shared<Task>();

    task->call = call;
    task->args = args;
    task->callee = callee;
    task->deleter = deleter;
    task->id = generate_id();

    {
        std::lock_guard<std::mutex> lock(registry_mutex);

        if (shutting_down.load())
            return 0;

        if (tasks.find(task->id) != tasks.end())
            return 0;

        try {
            task->worker = std::thread(execute_task, task);
        }
        catch (...) {
            return 0;
        }

        tasks.emplace(task->id, task);
    }

    return task->id;
}

} // namespace

// ------------------------------------------------------------
// Submit asynchronously
// ------------------------------------------------------------

taskid_t wait_for(
    Calling call,
    Args args,
    Callee callee,
    ResultDeleter deleter)
{
    return submit(call, args, callee, deleter);
}

// ------------------------------------------------------------
// Wait for an existing task
// ------------------------------------------------------------

int wait_for(taskid_t taskid)
{
    auto task = find_task(taskid);

    if (!task)
        return E_TASK_NOT_FOUND;

    if (current_task == task.get())
        return E_TASK_DEADLOCK;

    {
        std::unique_lock<std::mutex> lock(task->mutex);

        task->condition.wait(lock, [&] {
            return task->state != State::Running;
        });
    }

    join_worker(task);

    std::lock_guard<std::mutex> lock(task->mutex);

    switch (task->state) {
        case State::Completed:
            return 0;

        case State::Cancelled:
            return E_TASK_CANCELLED;

        case State::Failed:
            return E_TASK_FAILED;

        default:
            return E_TASK_NOT_FINISHED;
    }
}

// ------------------------------------------------------------
// Submit and wait with timeout
// ------------------------------------------------------------

int wait_until(
    Calling call,
    Args args,
    void** result,
    int timeout,
    Callee callee,
    ResultDeleter deleter)
{
    if (!call || !result || timeout < 0)
        return E_TASK_INVALID_ARGUMENT;

    *result = nullptr;

    taskid_t id = submit(call, args, callee, deleter);

    if (!id)
        return shutting_down.load()
            ? E_TASK_SHUTTING_DOWN
            : E_TASK_FAILED;

    auto task = find_task(id);

    if (!task)
        return E_TASK_NOT_FOUND;

    // Zero means wait forever.
    if (timeout == 0) {
        int status = wait_for(id);

        if (status != 0)
            return status;
    }
    else {
        int elapsed = 0;

        while (elapsed < timeout) {
            {
                std::lock_guard<std::mutex> lock(task->mutex);

                if (task->state != State::Running)
                    break;
            }

            // Use TimeX sleep rather than std::chrono.
            tx::sleep::sleep_exact(0, 1);

            ++elapsed;
        }

        bool finished = false;

        {
            std::lock_guard<std::mutex> lock(task->mutex);
            finished = task->state != State::Running;
        }

        if (!finished) {
            cancel(id);

            // Join before returning: the task must no longer be
            // accessing its arguments when wait_until returns.
            join_worker(task);

            return E_TASK_TIMEOUT;
        }

        int status = wait_for(id);

        if (status != 0)
            return status;
    }

    int error = 0;
    void* value = retrieve_result(id, &error);

    if (error != 0)
        return error;

    *result = value;

    // The result has been transferred, so metadata can be removed.
    del_task(id);

    return 0;
}

// ------------------------------------------------------------
// Retrieve result
// ------------------------------------------------------------

void* retrieve_result(taskid_t taskid, int* e)
{
    if (e)
        *e = 0;

    auto task = find_task(taskid);

    if (!task) {
        if (e)
            *e = E_TASK_NOT_FOUND;

        return nullptr;
    }

    std::lock_guard<std::mutex> lock(task->mutex);

    if (task->state == State::Running) {
        if (e)
            *e = E_TASK_NOT_FINISHED;

        return nullptr;
    }

    if (task->state == State::Cancelled) {
        if (e)
            *e = E_TASK_CANCELLED;

        return nullptr;
    }

    if (task->state == State::Failed) {
        if (e)
            *e = E_TASK_FAILED;

        return nullptr;
    }

    if (task->result_retrieved) {
        if (e)
            *e = E_TASK_RESULT_RETRIEVED;

        return nullptr;
    }

    task->result_retrieved = true;

    void* result = task->result;
    task->result = nullptr;

    return result;
}

// ------------------------------------------------------------
// Request cancellation
// ------------------------------------------------------------

void cancel(taskid_t taskid)
{
    auto task = find_task(taskid);

    if (!task)
        return;

    task->cancel_requested.store(true, std::memory_order_release);
    task->condition.notify_all();
}

// ------------------------------------------------------------
// Delete completed task
// ------------------------------------------------------------

int del_task(taskid_t taskid)
{
    auto task = find_task(taskid);

    if (!task)
        return E_TASK_NOT_FOUND;

    if (current_task == task.get())
        return E_TASK_DEADLOCK;

    join_worker(task);

    void* result = nullptr;
    ResultDeleter deleter = nullptr;

    {
        std::lock_guard<std::mutex> lock(task->mutex);

        if (task->state == State::Running)
            return E_TASK_NOT_FINISHED;

        if (task->result && !task->deleter)
            return E_TASK_RESULT_NOT_RETRIEVED;

        result = task->result;
        deleter = task->deleter;

        task->result = nullptr;
    }

    if (result && deleter) {
        try {
            deleter(result);
        }
        catch (...) {
            // Ignore exceptions from cleanup.
        }
    }

    std::lock_guard<std::mutex> lock(registry_mutex);

    auto it = tasks.find(taskid);

    if (it != tasks.end() && it->second == task)
        tasks.erase(it);

    return 0;
}

// ------------------------------------------------------------
// Check cancellation from inside a task
// ------------------------------------------------------------

bool cancellation_requested()
{
    return current_task &&
        current_task->cancel_requested.load(std::memory_order_acquire);
}

// ------------------------------------------------------------
// Shutdown
// ------------------------------------------------------------

int shutdown()
{
    if (current_task)
        return E_TASK_DEADLOCK;

    shutting_down.store(true);

    std::vector<std::shared_ptr<Task>> snapshot;

    {
        std::lock_guard<std::mutex> lock(registry_mutex);

        for (auto& [id, task] : tasks) {
            (void)id;
            snapshot.push_back(task);
        }
    }

    for (auto& task : snapshot) {
        task->cancel_requested.store(true);
        task->condition.notify_all();
    }

    for (auto& task : snapshot) {
        if (current_task == task.get())
            continue;

        join_worker(task);
    }

    int result = 0;

    for (auto& task : snapshot) {
        std::lock_guard<std::mutex> lock(task->mutex);

        if (task->result && !task->deleter) {
            result = E_TASK_RESULT_NOT_RETRIEVED;
        }
    }

    if (result != 0)
        return result;

    {
        std::lock_guard<std::mutex> lock(registry_mutex);
        tasks.clear();
    }

    return 0;
}

} // namespace tx::task