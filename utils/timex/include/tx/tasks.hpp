#pragma once

#include <cstdint>

namespace tx::task {

using taskid_t = std::uint64_t;

using Calling = void* (*)(void*);
using Args = void*;
using Callee = void (*)(void*);
using ResultDeleter = void (*)(void*);

// Error codes
constexpr int E_TASK_NOT_FINISHED       = 1;
constexpr int E_TASK_NOT_FOUND          = 2;
constexpr int E_TASK_CANCELLED          = 3;
constexpr int E_TASK_TIMEOUT            = 4;
constexpr int E_TASK_FAILED             = 5;
constexpr int E_TASK_RESULT_RETRIEVED   = 6;
constexpr int E_TASK_INVALID_ARGUMENT  = 7;
constexpr int E_TASK_RESULT_NOT_RETRIEVED = 8;
constexpr int E_TASK_DEADLOCK           = 9;
constexpr int E_TASK_SHUTTING_DOWN      = 10;

// Submit a task asynchronously.
taskid_t wait_for(
    Calling call,
    Args args,
    Callee callee = nullptr,
    ResultDeleter deleter = nullptr
);

// Wait for an existing task to finish.
int wait_for(taskid_t taskid);

// Submit a task and wait for its completion.
// timeout = 0 means wait indefinitely.
int wait_until(
    Calling call,
    Args args,
    void** result,
    int timeout = 0,
    Callee callee = nullptr,
    ResultDeleter deleter = nullptr
);

// Retrieve a completed task's result.
// Ownership of the result transfers to the caller.
void* retrieve_result(taskid_t taskid, int* e = nullptr);

// Request cooperative cancellation.
void cancel(taskid_t taskid);

// Delete completed task metadata and any safely disposable result.
int del_task(taskid_t taskid);

// Check whether the currently executing task was asked to cancel.
bool cancellation_requested();

// Stop the task system and join its workers.
int shutdown();

}