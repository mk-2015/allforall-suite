#pragma once

#include <tx/tasks.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace tx::task::internal {

enum class State {
    Running,
    Completed,
    Failed,
    Cancelled
};

struct Task {
    taskid_t id = 0;

    Calling call = nullptr;
    Args args = nullptr;
    Callee callee = nullptr;
    ResultDeleter deleter = nullptr;

    void* result = nullptr;

    State state = State::Running;

    std::atomic<bool> cancel_requested{false};

    std::mutex mutex;
    std::condition_variable condition;

    std::thread worker;

    bool result_retrieved = false;
    bool worker_joined = false;
};

}