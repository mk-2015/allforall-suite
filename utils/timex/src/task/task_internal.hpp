#pragma once

#include <tx/tasks.hpp>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace tx::task::internal {

enum class State {
    Running,
    Completed,
    Failed,
    Cancelled
};

struct Task {
    taskid_t id = 0;

    bool finishing = false;
    Calling call = nullptr;
    Args args = nullptr;

    Callee callee = nullptr;
    ResultDeleter deleter = nullptr;

    void* result = nullptr;

    State state = State::Running;

    std::atomic<bool> cancel_requested{false};

    bool result_retrieved = false;

    std::mutex mutex;
    std::condition_variable condition;
};

}