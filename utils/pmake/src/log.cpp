
#include <iostream>
#include <string>
#include <mutex>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <memory>

#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include <log.hpp>

namespace pmake::log {

namespace {

std::mutex log_mutex;

constexpr const char* RESET = "\033[0m";

constexpr const char* COLOR_INFO    = "\033[32m";
constexpr const char* COLOR_DEBUG   = "\033[36m";
constexpr const char* COLOR_WARNING = "\033[33m";
constexpr const char* COLOR_ERROR   = "\033[31m";
constexpr const char* COLOR_FATAL   = "\033[1;31m";
constexpr const char* COLOR_DEFAULT = "\033[37m";

bool colors_enabled(std::FILE* stream)
{
    if (std::getenv("NO_COLOR") != nullptr)
        return false;

#ifdef _WIN32
    return _isatty(_fileno(stream)) != 0;
#else
    return isatty(fileno(stream)) != 0;
#endif
}

const char* get_color(loglvl level)
{
    switch (level)
    {
        case INFO:
            return COLOR_INFO;

        case DEBUG:
            return COLOR_DEBUG;

        case WARNING:
            return COLOR_WARNING;

        case ERROR:
            return COLOR_ERROR;

        case FATAL:
            return COLOR_FATAL;

        default:
            return COLOR_DEFAULT;
    }
}

std::FILE* get_stream(loglvl level)
{
    if (level >= WARNING)
        return stderr;

    return stdout;
}

} // namespace

void slog(loglvl log_level, std::string string, bool tabbed)
{
    std::lock_guard<std::mutex> lock(log_mutex);

    std::FILE* stream = get_stream(log_level);
    bool use_color = colors_enabled(stream);
    const char* color = get_color(log_level);

    if (use_color)
        std::fputs(color, stream);  

    if (tabbed)
        std::fputs("    ", stream);
    else
        std::fputs("==", stream);

    std::fputs(">", stream);

    if (use_color)
        std::fputs(RESET, stream);

    std::fputc(' ', stream);

    if (!string.empty())
        std::fwrite(string.data(), 1, string.size(), stream);

    std::fputc('\n', stream);
    std::fflush(stream);
}

} // namespace pmake::log
