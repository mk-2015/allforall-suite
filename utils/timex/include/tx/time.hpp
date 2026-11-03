#pragma once

#include <tx/tx.hpp>
#include <tx/zones.hpp>
#include <tx/sleep.hpp>

#ifndef _A
#include "tx.hpp"
#include "zones.hpp"
#include "sleep.hpp"
#endif

#include <string>

namespace tx::time
{

enum class TimeType
{
    UTC,
    GMT,
    UNIX,
    Local
};

tx::Second time_now(TimeType type = TimeType::UTC);
tx::Nanosecond time_now_ns(TimeType type = TimeType::UTC);

tx::Time time_now_time(TimeType type = TimeType::UTC);
tx::Timedate time_now_timedate(TimeType type = TimeType::UTC);

/*
 * TimeX formatting tokens:
 *
 * %Y   = Year
 * %y   = Year (2 digits)
 * %m   = Month (01-12)
 * %d   = Day (01-31)
 * %H   = Hour (00-23)
 * %I   = Hour (01-12)
 * %M   = Minute (00-59)
 * %S   = Second (00-59)
 * %MS  = Millisecond (000-999)
 * %f   = Microsecond (000000-999999)
 * %N   = Nanosecond (000000000-999999999)
 * %p   = AM/PM
 *
 * Remaining printf-style conversions use variadic arguments.
 */

std::string time_format(
    tx::Timedate* time,
    const std::string& format,
    TimeType type, ...
);

std::string time_format(
    tx::Time* time,
    const std::string& format,
    TimeType type, ...
);

std::string time_format(
    tx::Date* time,
    const std::string& format,
    TimeType type, ...
);

std::string time_format(
    tx::Second* time,
    const std::string& format,
    TimeType type, ...
);

std::string time_format_ns(
    tx::Nanosecond* time,
    const std::string& format,
    TimeType type = TimeType::UTC, ...
);

} // namespace tx::time