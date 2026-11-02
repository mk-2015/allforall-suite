#include <tx/tasks.hpp>
#include <tx/sleep.hpp>

#include "task_internal.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace tx::task {
namespace {

using internal::State;
using internal::Task;

std::mutex registry_mutex;

std::unordered_map<taskid_t, std::shared_ptr<Task>> tasks;

std::atomic<taskid_t> next_id{1};

bool shutting_down = false;

thread_local Task* current_task = nullptr;

// ------------------------------------------------------------
// Registry lookup
// ------------------------------------------------------------

std::shared_ptr<Task> find_task(taskid_t id)
{
    std::lock_guard<std::mutex> lock(registry_mutex);

    auto it = tasks.find(id);

    if (it == tasks.end())
        return nullptr;

    return it->second;
}

// ------------------------------------------------------------
// ID allocation
// ------------------------------------------------------------

taskid_t generate_id_locked()
{
    // registry_mutex must be locked by the caller.
    for (;;) {
        taskid_t id = next_id.fetch_add(1);

        if (id == 0)
            continue;

        if (tasks.find(id) == tasks.end())
            return id;
    }
}

// ------------------------------------------------------------
// Task execution
// ------------------------------------------------------------

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

    bool cancelled = false;

    {
        std::lock_guard<std::mutex> lock(task->mutex);

        cancelled = task->cancel_requested.load(
            std::memory_order_acquire
        );

        if (!cancelled)
            task->finishing = true;
    }

    if (!cancelled && !failed && task->callee) {
        try {
            task->callee(result);
        }
        catch (...) {
            failed = true;
        }
    }

    if ((cancelled || failed) && result && task->deleter) {
        try {
            task->deleter(result);
        }
        catch (...) {
        }

        result = nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(task->mutex);

        if (cancelled) {
            task->state = State::Cancelled;
        }
        else if (failed) {
            task->state = State::Failed;
        }
        else {
            task->result = result;
            task->state = State::Completed;
        }
    }

    current_task = nullptr;
    task->condition.notify_all();
}

// ------------------------------------------------------------
// Submission
// ------------------------------------------------------------

taskid_t submit(
    Calling call,
    Args args,
    Callee callee,
    ResultDeleter deleter)
{
    if (!call)
        return 0;

    auto task = std::make_shared<Task>();

    task->call = call;
    task->args = args;
    task->callee = callee;
    task->deleter = deleter;

    std::lock_guard<std::mutex> lock(registry_mutex);

    if (shutting_down)
        return 0;

    task->id = generate_id_locked();

    // Register before starting the worker.
    tasks.emplace(task->id, task);

    try {
        std::thread worker(execute_task, task);
        worker.detach();
    }
    catch (...) {
        tasks.erase(task->id);
        return 0;
    }

    return task->id;
}

}

// ------------------------------------------------------------
// Wait for completion
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
// Public asynchronous submission
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
// Timed submission and wait
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

    if (!id) {
        std::lock_guard<std::mutex> lock(registry_mutex);

        return shutting_down
            ? E_TASK_SHUTTING_DOWN
            : E_TASK_FAILED;
    }

    auto task = find_task(id);

    if (!task)
        return E_TASK_NOT_FOUND;

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

            tx::sleep::sleep_exact(0, 1);
            ++elapsed;
        }

        bool finished;

        {
            std::lock_guard<std::mutex> lock(task->mutex);
            finished = task->state != State::Running;
        }

        if (!finished) {
            cancel(id);

            // Wait for the worker to stop using its arguments.
            {
                std::unique_lock<std::mutex> lock(task->mutex);

                task->condition.wait(lock, [&] {
                    return task->state != State::Running;
                });
            }

            // Delete if its result can be safely disposed of.
            del_task(id);

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

    del_task(id);

    return 0;
}

// ------------------------------------------------------------
// Result retrieval
// ------------------------------------------------------------

void* retrieve_result(taskid_t taskid, int* e)
{
    if (e)
        *e = 0;

    // Lock order: registry, then task.
    std::lock_guard<std::mutex> registry_lock(registry_mutex);

    auto it = tasks.find(taskid);

    if (it == tasks.end()) {
        if (e)
            *e = E_TASK_NOT_FOUND;

        return nullptr;
    }

    auto task = it->second;

    std::lock_guard<std::mutex> task_lock(task->mutex);

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

    void* value = task->result;
    task->result = nullptr;

    return value;
}

// ------------------------------------------------------------
// Cancellation
// ------------------------------------------------------------

void cancel(taskid_t taskid)
{
    auto task = find_task(taskid);

    if (!task)
        return;

    std::lock_guard<std::mutex> lock(task->mutex);

    if (task->state == State::Running && !task->finishing) {
        task->cancel_requested.store(
            true,
            std::memory_order_release
        );
    }
}

// ------------------------------------------------------------
// Task deletion
// ------------------------------------------------------------

int del_task(taskid_t taskid)
{
    void* result = nullptr;
    ResultDeleter deleter = nullptr;

    {
        // Lock order: registry, then task.
        std::lock_guard<std::mutex> registry_lock(registry_mutex);

        auto it = tasks.find(taskid);

        if (it == tasks.end())
            return E_TASK_NOT_FOUND;

        auto task = it->second;

        if (current_task == task.get())
            return E_TASK_DEADLOCK;

        std::lock_guard<std::mutex> task_lock(task->mutex);

        if (task->state == State::Running)
            return E_TASK_NOT_FINISHED;

        if (task->result && !task->deleter)
            return E_TASK_RESULT_NOT_RETRIEVED;

        result = task->result;
        deleter = task->deleter;

        task->result = nullptr;

        tasks.erase(it);
    }

    if (result && deleter) {
        try {
            deleter(result);
        }
        catch (...) {
            // Ignore cleanup exceptions.
        }
    }

    return 0;
}

// ------------------------------------------------------------
// Cooperative cancellation query
// ------------------------------------------------------------

bool cancellation_requested()
{
    return current_task &&
        current_task->cancel_requested.load(
            std::memory_order_acquire
        );
}

// ------------------------------------------------------------
// Shutdown
// ------------------------------------------------------------

int shutdown()
{
    if (current_task)
        return E_TASK_DEADLOCK;

    std::vector<std::shared_ptr<Task>> snapshot;

    {
        std::lock_guard<std::mutex> lock(registry_mutex);

        shutting_down = true;

        for (auto& entry : tasks)
            snapshot.push_back(entry.second);
    }

    // Request cancellation.
    for (auto& task : snapshot) {
        std::lock_guard<std::mutex> lock(task->mutex);

        if (task->state == State::Running) {
            task->cancel_requested.store(
                true,
                std::memory_order_release
            );
        }
    }

    // Wait for every worker to finish.
    for (auto& task : snapshot) {
        std::unique_lock<std::mutex> lock(task->mutex);

        task->condition.wait(lock, [&] {
            return task->state != State::Running;
        });
    }

    // Clean up results and registry.
    for (auto& task : snapshot) {
        std::lock_guard<std::mutex> lock(task->mutex);

        if (task->result && !task->deleter)
            return E_TASK_RESULT_NOT_RETRIEVED;
    }

    for (auto& task : snapshot) {
        void* result = nullptr;
        ResultDeleter deleter = nullptr;

        {
            std::lock_guard<std::mutex> lock(task->mutex);

            result = task->result;
            deleter = task->deleter;

            task->result = nullptr;
        }

        if (result && deleter) {
            try {
                deleter(result);
            }
            catch (...) {
                // Ignore cleanup exceptions.
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(registry_mutex);
        tasks.clear();
    }

    return 0;
}

} // namespace tx::task