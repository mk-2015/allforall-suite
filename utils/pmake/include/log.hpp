
#pragma once

#include <string>
#include <format>
#include <utility>

namespace pmake::log {

inline constexpr int INFO    = 1;
inline constexpr int DEBUG   = 2;
inline constexpr int WARNING = 3;
inline constexpr int ERROR   = 4;
inline constexpr int FATAL   = 5;

using loglvl = int;

void slog(loglvl log_level, std::string string, bool tabbed = false);

template<typename... Args>
void slogf(
    loglvl log_level,
    std::format_string<Args...> format,
    Args&&... args
)
{
    slog(
        log_level,
        std::format(format, std::forward<Args>(args)...)
    );
}

template<typename... Args>
void slogf_tabbed(
    loglvl log_level,
    std::format_string<Args...> format,
    Args&&... args
)
{
    slog(
        log_level,
        std::format(format, std::forward<Args>(args)...),
        true
    );
}

}
