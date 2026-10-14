#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <threads.h>

#include <ut/types.h>

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

typedef enum {
    LOG_OUTPUT_STDOUT,
    LOG_OUTPUT_STDERR,
    LOG_OUTPUT_FILE
} log_output_t;

typedef struct log_queue_entry {
    log_level_t level;
    char *message;
} log_queue_entry_t;

typedef struct {
    log_output_t output;
    const char *file_path;
    int async;
    int running;
    size_t queue_capacity;
    size_t queue_size;
    size_t queue_head;
    size_t queue_tail;
    log_queue_entry_t *queue;
    mtx_t queue_lock;
    cnd_t queue_signal;
    thrd_t worker;
} log_config_t;

log_config_t *start_log(log_output_t output, const char *file_path);
log_config_t *start_log_async(log_output_t output, const char *file_path, size_t queue_capacity);
int start_async_log(log_config_t *config);
void log_message(log_config_t *config, log_level_t level, const char *format, ...);
void log_message_async(log_config_t *config, log_level_t level, const char *format, ...);
void async_log_message(log_config_t *config, log_level_t level, const char *format, ...);
int snlog_message(log_config_t *config, log_level_t level, char *buffer, size_t buffer_size, const char *format, ...);
void flush_log(log_config_t *config);
void end_log(log_config_t *config);