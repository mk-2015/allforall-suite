#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ut/logging.h>
#include <ut/types.h>

static const char *log_level_to_string(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "   sDEBUG   ";
        case LOG_LEVEL_INFO: return "   INFO    ";
        case LOG_LEVEL_WARN: return "   WARN    ";
        case LOG_LEVEL_ERROR: return "   ERROR   ";
        case LOG_LEVEL_FATAL: return "   FATAL   ";
        default: return "  UNKNOWN  ";
    }
}

static char *copy_string(const char *source) {
    size_t length;
    char *copy;

    if (source == NULL) {
        return NULL;
    }

    length = strlen(source) + 1;
    copy = (char *)malloc(length);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, source, length);
    return copy;
}

static void log_free_queue(log_config_t *config) {
    size_t index;

    if (config == NULL || config->queue == NULL) {
        return;
    }

    for (index = 0; index < config->queue_capacity; ++index) {
        if (config->queue[index].message != NULL) {
            free(config->queue[index].message);
            config->queue[index].message = NULL;
        }
    }

    free(config->queue);
    config->queue = NULL;
    config->queue_size = 0;
    config->queue_head = 0;
    config->queue_tail = 0;
    config->queue_capacity = 0;
}

static int log_write_to_stream(log_config_t *config, log_level_t level, const char *message) {
    FILE *out;
    int close_file;
    size_t length;

    if (config == NULL || message == NULL) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    switch (config->output) {
        case LOG_OUTPUT_STDOUT:
            out = stdout;
            close_file = 0;
            break;
        case LOG_OUTPUT_STDERR:
            out = stderr;
            close_file = 0;
            break;
        case LOG_OUTPUT_FILE:
            if (config->file_path == NULL) {
                errorno = ERROR_FAILURE;
                return -1;
            }
            out = fopen(config->file_path, "a");
            if (out == NULL) {
                errorno = ERROR_FAILURE;
                return -1;
            }
            close_file = 1;
            break;
        default:
            errorno = ERROR_FAILURE;
            return -1;
    }

    fprintf(out, "[%s] %s", log_level_to_string(level), message);
    length = strlen(message);
    if (length == 0 || message[length - 1] != '\n') {
        fputc('\n', out);
    }

    if (close_file) {
        fclose(out);
    }

    return 0;
}

static int log_worker_main(void *arg) {
    log_config_t *config = (log_config_t *)arg;
    log_queue_entry_t entry;

    if (config == NULL) {
        return thrd_error;
    }

    for (;;) {
        mtx_lock(&config->queue_lock);
        while (config->queue_size == 0 && config->running) {
            cnd_wait(&config->queue_signal, &config->queue_lock);
        }

        if (!config->running && config->queue_size == 0) {
            mtx_unlock(&config->queue_lock);
            return thrd_success;
        }

        entry = config->queue[config->queue_head];
        config->queue_head = (config->queue_head + 1U) % config->queue_capacity;
        config->queue_size--;
        mtx_unlock(&config->queue_lock);

        log_write_to_stream(config, entry.level, entry.message);
        free(entry.message);
    }
}

static int log_config_init(log_config_t *config, log_output_t output, const char *file_path, int async) {
    if (config == NULL) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    memset(config, 0, sizeof(*config));
    config->output = output;
    config->async = async;
    config->running = 0;

    if (output == LOG_OUTPUT_FILE) {
        if (file_path == NULL) {
            errorno = ERROR_FAILURE;
            return -1;
        }
        config->file_path = copy_string(file_path);
        if (config->file_path == NULL) {
            errorno = ERROR_FAILURE;
            return -1;
        }
    }

    return 0;
}

static int log_queue_message(log_config_t *config, log_level_t level, const char *message);

static int log_message_impl(log_config_t *config, log_level_t level, const char *format, va_list args) {
    char *buffer;
    int written;
    va_list copy;

    if (config == NULL || format == NULL) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    va_copy(copy, args);
    written = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    if (written < 0) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    buffer = (char *)malloc((size_t)written + 1U);
    if (buffer == NULL) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    vsnprintf(buffer, (size_t)written + 1U, format, args);

    if (config->async && config->running) {
        if (log_queue_message(config, level, buffer) != 0) {
            free(buffer);
            return -1;
        }
    } else {
        if (log_write_to_stream(config, level, buffer) != 0) {
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 0;
}

static int log_queue_message(log_config_t *config, log_level_t level, const char *message) {
    log_queue_entry_t entry;
    size_t next_capacity;

    if (config == NULL || message == NULL) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    if (!config->async || config->queue == NULL || config->queue_capacity == 0) {
        return log_write_to_stream(config, level, message);
    }

    mtx_lock(&config->queue_lock);
    if (!config->running) {
        mtx_unlock(&config->queue_lock);
        errorno = ERROR_FAILURE;
        return -1;
    }

    if (config->queue_size >= config->queue_capacity) {
        next_capacity = config->queue_capacity * 2U;
        config->queue = (log_queue_entry_t *)realloc(config->queue, next_capacity * sizeof(log_queue_entry_t));
        if (config->queue == NULL) {
            mtx_unlock(&config->queue_lock);
            errorno = ERROR_FAILURE;
            return -1;
        }
        config->queue_capacity = next_capacity;
    }

    entry.level = level;
    entry.message = copy_string(message);
    if (entry.message == NULL) {
        mtx_unlock(&config->queue_lock);
        errorno = ERROR_FAILURE;
        return -1;
    }

    config->queue[config->queue_tail] = entry;
    config->queue_tail = (config->queue_tail + 1U) % config->queue_capacity;
    config->queue_size++;
    cnd_signal(&config->queue_signal);
    mtx_unlock(&config->queue_lock);
    return 0;
}

log_config_t *start_log(log_output_t output, const char *file_path) {
    log_config_t *config = (log_config_t *)calloc(1, sizeof(log_config_t));

    if (config == NULL) {
        errorno = ERROR_FAILURE;
        return NULL;
    }

    if (log_config_init(config, output, file_path, 0) != 0) {
        free(config);
        return NULL;
    }

    return config;
}

log_config_t *start_log_async(log_output_t output, const char *file_path, size_t queue_capacity) {
    log_config_t *config = (log_config_t *)calloc(1, sizeof(log_config_t));
    size_t capacity;

    if (config == NULL) {
        errorno = ERROR_FAILURE;
        return NULL;
    }

    if (log_config_init(config, output, file_path, 1) != 0) {
        free(config);
        return NULL;
    }

    capacity = queue_capacity == 0 ? 64U : queue_capacity;
    config->queue_capacity = capacity;
    config->queue = (log_queue_entry_t *)calloc(capacity, sizeof(log_queue_entry_t));
    if (config->queue == NULL) {
        errorno = ERROR_FAILURE;
        free((void *)config->file_path);
        free(config);
        return NULL;
    }

    if (mtx_init(&config->queue_lock, mtx_plain) != thrd_success) {
        errorno = ERROR_FAILURE;
        free((void *)config->file_path);
        log_free_queue(config);
        free(config);
        return NULL;
    }

    if (cnd_init(&config->queue_signal) != thrd_success) {
        errorno = ERROR_FAILURE;
        mtx_destroy(&config->queue_lock);
        free((void *)config->file_path);
        log_free_queue(config);
        free(config);
        return NULL;
    }

    config->running = 1;
    if (thrd_create(&config->worker, log_worker_main, config) != thrd_success) {
        errorno = ERROR_FAILURE;
        config->running = 0;
        cnd_destroy(&config->queue_signal);
        mtx_destroy(&config->queue_lock);
        free((void *)config->file_path);
        log_free_queue(config);
        free(config);
        return NULL;
    }

    return config;
}

int start_async_log(log_config_t *config) {
    size_t capacity;

    if (config == NULL) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    if (config->async) {
        return 0;
    }

    capacity = config->queue_capacity == 0 ? 64U : config->queue_capacity;
    config->queue_capacity = capacity;
    config->queue = (log_queue_entry_t *)calloc(capacity, sizeof(log_queue_entry_t));
    if (config->queue == NULL) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    if (mtx_init(&config->queue_lock, mtx_plain) != thrd_success) {
        errorno = ERROR_FAILURE;
        log_free_queue(config);
        return -1;
    }

    if (cnd_init(&config->queue_signal) != thrd_success) {
        errorno = ERROR_FAILURE;
        mtx_destroy(&config->queue_lock);
        log_free_queue(config);
        return -1;
    }

    config->async = 1;
    config->running = 1;
    if (thrd_create(&config->worker, log_worker_main, config) != thrd_success) {
        errorno = ERROR_FAILURE;
        config->running = 0;
        cnd_destroy(&config->queue_signal);
        mtx_destroy(&config->queue_lock);
        log_free_queue(config);
        return -1;
    }

    return 0;
}

void log_message(log_config_t *config, log_level_t level, const char *format, ...) {
    va_list args;

    if (config == NULL || format == NULL) {
        errorno = ERROR_FAILURE;
        return;
    }

    va_start(args, format);
    log_message_impl(config, level, format, args);
    va_end(args);
}

void log_message_async(log_config_t *config, log_level_t level, const char *format, ...) {
    va_list args;

    if (config == NULL || format == NULL) {
        errorno = ERROR_FAILURE;
        return;
    }

    va_start(args, format);
    log_message_impl(config, level, format, args);
    va_end(args);
}

void async_log_message(log_config_t *config, log_level_t level, const char *format, ...) {
    va_list args;

    if (config == NULL || format == NULL) {
        errorno = ERROR_FAILURE;
        return;
    }

    va_start(args, format);
    log_message_impl(config, level, format, args);
    va_end(args);
}

int snlog_message(log_config_t *config, log_level_t level, char *buffer, size_t buffer_size, const char *format, ...) {
    int prefix_len;
    char *remaining_buf;
    size_t remaining_size;
    va_list args;
    int written;

    if (config == NULL || buffer == NULL || buffer_size == 0 || format == NULL) {
        errorno = ERROR_FAILURE;
        return -1;
    }

    prefix_len = snprintf(buffer, buffer_size, "[%s] ", log_level_to_string(level));
    if (prefix_len < 0 || (size_t)prefix_len >= buffer_size) {
        return prefix_len;
    }

    remaining_buf = buffer + prefix_len;
    remaining_size = buffer_size - (size_t)prefix_len;

    va_start(args, format);
    written = vsnprintf(remaining_buf, remaining_size, format, args);
    va_end(args);

    if (written < 0) {
        return -1;
    }

    return prefix_len + written;
}

void flush_log(log_config_t *config) {
    if (config == NULL || !config->async) {
        return;
    }

    mtx_lock(&config->queue_lock);
    while (config->queue_size > 0) {
        mtx_unlock(&config->queue_lock);
        thrd_yield();
        mtx_lock(&config->queue_lock);
    }
    mtx_unlock(&config->queue_lock);
}

void end_log(log_config_t *config) {
    if (config == NULL) {
        return;
    }

    if (config->async && config->queue != NULL) {
        mtx_lock(&config->queue_lock);
        config->running = 0;
        cnd_broadcast(&config->queue_signal);
        mtx_unlock(&config->queue_lock);
        thrd_join(config->worker, NULL);
        cnd_destroy(&config->queue_signal);
        mtx_destroy(&config->queue_lock);
        log_free_queue(config);
    }

    if (config->file_path != NULL) {
        free((void *)config->file_path);
        config->file_path = NULL;
    }

    free(config);
}