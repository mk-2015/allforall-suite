#include <tx/time.hpp>

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace tx::time
{

namespace
{

struct TimeFields
{
    tx::Year year = 1970;
    tx::Month month = 1;
    tx::Day day = 1;

    tx::Hour hour = 0;
    tx::Minute minute = 0;
    tx::Second second = 0;

    tx::Millisecond millisecond = 0;
    tx::Microsecond microsecond = 0;
    tx::Nanosecond nanosecond = 0;
};

std::string padded(std::uint64_t value, int width)
{
    std::ostringstream out;
    out << std::setfill('0') << std::setw(width) << value;
    return out.str();
}

TimeFields get_fields(const tx::Date& date)
{
    TimeFields f;

    f.year = date.year();
    f.month = date.month();
    f.day = date.day();

    return f;
}

TimeFields get_fields(const tx::Time& time)
{
    TimeFields f;

    f.hour = time.hour();
    f.minute = time.minute();
    f.second = time.second();

    f.millisecond = time.millisecond();
    f.microsecond = time.microsecond();
    f.nanosecond = time.nanosecond();

    return f;
}

TimeFields get_fields(const tx::Timedate& value)
{
    TimeFields f = get_fields(value.date());

    const tx::Time t = value.time();

    f.hour = t.hour();
    f.minute = t.minute();
    f.second = t.second();

    f.millisecond = t.millisecond();
    f.microsecond = t.microsecond();
    f.nanosecond = t.nanosecond();

    return f;
}

/*
 * Convert a Unix timestamp into calendar fields.
 *
 * Local conversion uses the offset returned by tx::zone.
 * UTC, GMT and UNIX use a zero UTC offset.
 */
TimeFields timestamp_fields(
    tx::Second timestamp,
    tx::Nanosecond fractional_ns,
    TimeType type
)
{
    using namespace std::chrono;

    TimeFields result;

    tx::Offset offset = 0;

    if (type == TimeType::Local)
    {
        const std::string zone = tx::zone::current_zone();

        const auto info = tx::zone::zone_info(zone, timestamp);

        offset = info.offset;
    }

    if (timestamp >
        static_cast<tx::Second>(
            std::numeric_limits<std::int64_t>::max()
        ))
    {
        return result;
    }

    const auto seconds_value =
        static_cast<std::int64_t>(timestamp);

    const sys_seconds utc_time{
        seconds{seconds_value}
    };

    const sys_seconds adjusted_time =
        utc_time + seconds{offset};

    const auto day_point = floor<days>(adjusted_time);
    const year_month_day date{day_point};

    const hh_mm_ss tod{
        adjusted_time - day_point
    };

    result.year = static_cast<int>(date.year());
    result.month = static_cast<unsigned>(date.month());
    result.day = static_cast<unsigned>(date.day());

    result.hour = static_cast<unsigned>(tod.hours().count());
    result.minute = static_cast<unsigned>(tod.minutes().count());
    result.second = static_cast<unsigned>(tod.seconds().count());

    fractional_ns %= 1'000'000'000ULL;

    result.millisecond = fractional_ns / 1'000'000ULL;
    result.microsecond = (fractional_ns / 1'000ULL) % 1'000ULL;
    result.nanosecond = fractional_ns % 1'000ULL;

    return result;
}

std::string expand_tokens(
    const std::string& format,
    const TimeFields& f
)
{
    std::string output;
    output.reserve(format.size() + 32);

    for (std::size_t i = 0; i < format.size();)
    {
        if (format[i] != '%')
        {
            output += format[i++];
            continue;
        }

        if (i + 1 >= format.size())
        {
            output += '%';
            ++i;
            continue;
        }

        const char next = format[i + 1];

        if (next == '%')
        {
            output += "%%";
            i += 2;
            continue;
        }

        /*
         * Check longer tokens before shorter ones.
         */

        if (format.compare(i, 3, "%MS") == 0)
        {
            output += padded(f.millisecond, 3);
            i += 3;
            continue;
        }

        switch (next)
        {
            case 'Y':
                output += padded(f.year, 4);
                i += 2;
                break;

            case 'y':
                output += padded(f.year % 100, 2);
                i += 2;
                break;

            case 'm':
                output += padded(f.month, 2);
                i += 2;
                break;

            case 'd':
                output += padded(f.day, 2);
                i += 2;
                break;

            case 'H':
                output += padded(f.hour, 2);
                i += 2;
                break;

            case 'I':
            {
                const tx::Hour hour =
                    (f.hour % 12 == 0) ? 12 : f.hour % 12;

                output += padded(hour, 2);
                i += 2;
                break;
            }

            case 'M':
                output += padded(f.minute, 2);
                i += 2;
                break;

            case 'S':
                output += padded(f.second, 2);
                i += 2;
                break;

            case 'f':
            {
                const std::uint64_t microseconds =
                    f.millisecond * 1000 + f.microsecond;

                output += padded(microseconds, 6);
                i += 2;
                break;
            }

            case 'N':
            {
                const std::uint64_t nanoseconds =
                    f.millisecond * 1'000'000ULL +
                    f.microsecond * 1'000ULL +
                    f.nanosecond;

                output += padded(nanoseconds, 9);
                i += 2;
                break;
            }

            case 'p':
                output += (f.hour < 12) ? "AM" : "PM";
                i += 2;
                break;

            default:
                /*
                 * Leave unknown conversions for printf.
                 * Example: %s, %d, %x, %f.
                 */
                output += '%';
                ++i;
                break;
        }
    }

    return output;
}

/*
 * Second formatting pass.
 *
 * Only printf-style conversions remaining after TimeX token
 * expansion reach this function.
 */
std::string format_arguments(
    const std::string& format,
    va_list args
)
{
    va_list copy;
    va_copy(copy, args);

    const int required =
        std::vsnprintf(nullptr, 0, format.c_str(), copy);

    va_end(copy);

    if (required < 0)
        return {};

    std::string result(
        static_cast<std::size_t>(required) + 1,
        '\0'
    );

    va_list second_copy;
    va_copy(second_copy, args);

    std::vsnprintf(
        result.data(),
        result.size(),
        format.c_str(),
        second_copy
    );

    va_end(second_copy);

    result.resize(static_cast<std::size_t>(required));

    return result;
}

template <typename T>
std::string format_value(
    const T& value,
    const std::string& format,
    TimeType type,
    va_list args
);

template <>
std::string format_value<tx::Date>(
    const tx::Date& value,
    const std::string& format,
    TimeType,
    va_list args
)
{
    const auto fields = get_fields(value);
    const auto expanded = expand_tokens(format, fields);

    return format_arguments(expanded, args);
}

template <>
std::string format_value<tx::Time>(
    const tx::Time& value,
    const std::string& format,
    TimeType,
    va_list args
)
{
    const auto fields = get_fields(value);
    const auto expanded = expand_tokens(format, fields);

    return format_arguments(expanded, args);
}

template <>
std::string format_value<tx::Timedate>(
    const tx::Timedate& value,
    const std::string& format,
    TimeType,
    va_list args
)
{
    const auto fields = get_fields(value);
    const auto expanded = expand_tokens(format, fields);

    return format_arguments(expanded, args);
}

template <>
std::string format_value<tx::Second>(
    const tx::Second& value,
    const std::string& format,
    TimeType type,
    va_list args
)
{
    const auto fields = timestamp_fields(value, 0, type);
    const auto expanded = expand_tokens(format, fields);

    return format_arguments(expanded, args);
}

std::string format_nanoseconds(
    const tx::Nanosecond& value,
    const std::string& format,
    TimeType type,
    va_list args
)
{
    const tx::Second seconds_value =
        value / 1'000'000'000ULL;

    const tx::Nanosecond fractional =
        value % 1'000'000'000ULL;

    const auto fields =
        timestamp_fields(seconds_value, fractional, type);

    const auto expanded = expand_tokens(format, fields);

    return format_arguments(expanded, args);
}

template <typename T>
std::string dispatch_format(
    T* value,
    const std::string& format,
    TimeType type,
    va_list args
)
{
    if (value == nullptr)
        return {};

    return format_value<T>(*value, format, type, args);
}

} // namespace

std::string time_format(
    tx::Timedate* value,
    const std::string& format,
    TimeType type,
    ...
)
{
    va_list args;
    va_start(args, type);

    const auto result =
        dispatch_format(value, format, type, args);

    va_end(args);
    return result;
}

std::string time_format(
    tx::Time* value,
    const std::string& format,
    TimeType type,
    ...
)
{
    va_list args;
    va_start(args, type);

    const auto result =
        dispatch_format(value, format, type, args);

    va_end(args);
    return result;
}

std::string time_format(
    tx::Date* value,
    const std::string& format,
    TimeType type,
    ...
)
{
    va_list args;
    va_start(args, type);

    const auto result =
        dispatch_format(value, format, type, args);

    va_end(args);
    return result;
}

std::string time_format(
    tx::Second* value,
    const std::string& format,
    TimeType type,
    ...
)
{
    va_list args;
    va_start(args, type);

    const auto result =
        dispatch_format(value, format, type, args);

    va_end(args);
    return result;
}

std::string time_format_ns(
    tx::Nanosecond* value,
    const std::string& format,
    TimeType type,
    ...
)
{
    if (value == nullptr)
        return {};

    va_list args;
    va_start(args, type);

    const auto result =
        format_nanoseconds(*value, format, type, args);

    va_end(args);

    return result;
}

} // namespace tx::time