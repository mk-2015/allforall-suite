#include <tx/tx.hpp>

#include <stdexcept>
#include <limits>
#include <cstdint>

namespace tx
{

namespace
{

constexpr Nanosecond NS_PER_SECOND = 1'000'000'000ULL;
constexpr Nanosecond NS_PER_MINUTE = 60ULL * NS_PER_SECOND;
constexpr Nanosecond NS_PER_HOUR   = 60ULL * NS_PER_MINUTE;
constexpr Nanosecond NS_PER_DAY    = 24ULL * NS_PER_HOUR;

bool add_time_units(
    Time& time,
    std::uint64_t value,
    std::uint64_t units_per_day,
    Nanosecond ns_per_unit) noexcept
{
    const Nanosecond current = time.to_nanoseconds();

    const Nanosecond addition =
        (value % units_per_day) * ns_per_unit;

    const Nanosecond result =
        (current + addition) % NS_PER_DAY;

    time = Time::from_nanoseconds(result);
    return true;
}

bool sub_time_units(
    Time& time,
    std::uint64_t value,
    std::uint64_t units_per_day,
    Nanosecond ns_per_unit) noexcept
{
    const Nanosecond current = time.to_nanoseconds();

    const Nanosecond subtraction =
        (value % units_per_day) * ns_per_unit;

    const Nanosecond result =
        (current + NS_PER_DAY - subtraction) % NS_PER_DAY;

    time = Time::from_nanoseconds(result);
    return true;
}

}

// ============================================
// Constructor
// ============================================

Time::Time(
    Hour hour,
    Minute minute,
    Second second,
    Millisecond millisecond,
    Microsecond microsecond,
    Nanosecond nanosecond)
{
    if (!set(hour, minute, second, millisecond,
             microsecond, nanosecond))
        throw std::invalid_argument("invalid time");
}

// ============================================
// Getters
// ============================================

Hour Time::hour() const noexcept { return hours; }
Minute Time::minute() const noexcept { return minutes; }
Second Time::second() const noexcept { return seconds; }

Millisecond Time::millisecond() const noexcept { return milliseconds; }
Microsecond Time::microsecond() const noexcept { return microseconds; }
Nanosecond Time::nanosecond() const noexcept { return nanoseconds; }

// ============================================
// Setters
// ============================================

bool Time::set_hour(Hour value) noexcept
{
    if (value > 23)
        return false;

    hours = value;
    return true;
}

bool Time::set_minute(Minute value) noexcept
{
    if (value > 59)
        return false;

    minutes = value;
    return true;
}

bool Time::set_second(Second value) noexcept
{
    if (value > 59)
        return false;

    seconds = value;
    return true;
}

bool Time::set_millisecond(Millisecond value) noexcept
{
    if (value > 999)
        return false;

    milliseconds = value;
    return true;
}

bool Time::set_microsecond(Microsecond value) noexcept
{
    if (value > 999)
        return false;

    microseconds = value;
    return true;
}

bool Time::set_nanosecond(Nanosecond value) noexcept
{
    if (value > 999)
        return false;

    nanoseconds = value;
    return true;
}

bool Time::set(
    Hour hour,
    Minute minute,
    Second second,
    Millisecond millisecond,
    Microsecond microsecond,
    Nanosecond nanosecond) noexcept
{
    if (hour > 23 || minute > 59 || second > 59 ||
        millisecond > 999 || microsecond > 999 ||
        nanosecond > 999)
        return false;

    hours = hour;
    minutes = minute;
    seconds = second;
    milliseconds = millisecond;
    microseconds = microsecond;
    nanoseconds = nanosecond;

    return true;
}

// ============================================
// Validation
// ============================================

bool Time::is_valid() const noexcept
{
    return hours <= 23 &&
           minutes <= 59 &&
           seconds <= 59 &&
           milliseconds <= 999 &&
           microseconds <= 999 &&
           nanoseconds <= 999;
}

// ============================================
// Conversion
// ============================================

Nanosecond Time::to_nanoseconds() const noexcept
{
    return hours * NS_PER_HOUR +
           minutes * NS_PER_MINUTE +
           seconds * NS_PER_SECOND +
           milliseconds * 1'000'000ULL +
           microseconds * 1'000ULL +
           nanoseconds;
}

Time Time::from_nanoseconds(Nanosecond value)
{
    if (value >= NS_PER_DAY)
        throw std::invalid_argument("time exceeds one day");

    Time result;

    result.hours = value / NS_PER_HOUR;
    value %= NS_PER_HOUR;

    result.minutes = value / NS_PER_MINUTE;
    value %= NS_PER_MINUTE;

    result.seconds = value / NS_PER_SECOND;
    value %= NS_PER_SECOND;

    result.milliseconds = value / 1'000'000ULL;
    value %= 1'000'000ULL;

    result.microseconds = value / 1'000ULL;
    value %= 1'000ULL;

    result.nanoseconds = value;

    return result;
}

// ============================================
// Add time units
// ============================================

bool Time::add_hour(Hour value) noexcept
{
    return add_time_units(*this, value, 24, NS_PER_HOUR);
}

bool Time::add_minute(Minute value) noexcept
{
    return add_time_units(*this, value, 1440, NS_PER_MINUTE);
}

bool Time::add_sec(Second value) noexcept
{
    return add_time_units(*this, value, 86400, NS_PER_SECOND);
}

bool Time::add_ms(Millisecond value) noexcept
{
    return add_time_units(*this, value, 86400000, 1'000'000ULL);
}

bool Time::add_us(Microsecond value) noexcept
{
    return add_time_units(*this, value, 86400000000ULL, 1'000ULL);
}

bool Time::add_ns(Nanosecond value) noexcept
{
    return add_time_units(*this, value, NS_PER_DAY, 1);
}

// ============================================
// Subtract time units
// ============================================

bool Time::sub_hour(Hour value) noexcept
{
    return sub_time_units(*this, value, 24, NS_PER_HOUR);
}

bool Time::sub_minute(Minute value) noexcept
{
    return sub_time_units(*this, value, 1440, NS_PER_MINUTE);
}

bool Time::sub_sec(Second value) noexcept
{
    return sub_time_units(*this, value, 86400, NS_PER_SECOND);
}

bool Time::sub_ms(Millisecond value) noexcept
{
    return sub_time_units(*this, value, 86400000, 1'000'000ULL);
}

bool Time::sub_us(Microsecond value) noexcept
{
    return sub_time_units(*this, value, 86400000000ULL, 1'000ULL);
}

bool Time::sub_ns(Nanosecond value) noexcept
{
    return sub_time_units(*this, value, NS_PER_DAY, 1);
}

// ============================================
// Operators
// Operand unit: nanoseconds
// ============================================

Time Time::operator+(Nanosecond value) const
{
    Time result = *this;
    result.add_ns(value);
    return result;
}

Time Time::operator-(Nanosecond value) const
{
    Time result = *this;
    result.sub_ns(value);
    return result;
}

Time& Time::operator+=(Nanosecond value)
{
    add_ns(value);
    return *this;
}

Time& Time::operator-=(Nanosecond value)
{
    sub_ns(value);
    return *this;
}

} // namespace tx