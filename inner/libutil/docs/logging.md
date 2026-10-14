# Logging API

## Overview

The logging layer supports both direct writes and asynchronous background logging. It uses C11 threads via `threads.h` for queue processing and keeps the public API small and easy to use.

## Sync usage

```c
#include <ut/logging.h>

int main(void) {
    log_config_t *log = start_log(LOG_OUTPUT_STDOUT, NULL);
    log_message(log, LOG_LEVEL_INFO, "startup complete");
    end_log(log);
    return 0;
}
```

## Async usage

```c
#include <ut/logging.h>

int main(void) {
    log_config_t *log = start_log_async(LOG_OUTPUT_FILE, "app.log", 128);
    log_message_async(log, LOG_LEVEL_WARN, "background task started");
    log_message_async(log, LOG_LEVEL_ERROR, "temporary failure: %d", 42);
    flush_log(log);
    end_log(log);
    return 0;
}
```

## API

- `log_config_t *start_log(log_output_t output, const char *file_path);`
- `log_config_t *start_log_async(log_output_t output, const char *file_path, size_t queue_capacity);`
- `int start_async_log(log_config_t *config);`
- `void log_message(log_config_t *config, log_level_t level, const char *format, ...);`
- `void log_message_async(log_config_t *config, log_level_t level, const char *format, ...);`
- `void async_log_message(log_config_t *config, log_level_t level, const char *format, ...);`
- `int snlog_message(log_config_t *config, log_level_t level, char *buffer, size_t buffer_size, const char *format, ...);`
- `void flush_log(log_config_t *config);`
- `void end_log(log_config_t *config);`

## Notes

- `LOG_OUTPUT_STDOUT` and `LOG_OUTPUT_STDERR` write immediately to the selected console stream.
- `LOG_OUTPUT_FILE` appends to the configured log file.
- Async logs are processed by a worker thread and are queued before writing.
- `flush_log` waits until the queue is empty before returning.
- `end_log` shuts down the worker thread and releases all resources.
