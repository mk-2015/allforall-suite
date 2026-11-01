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
    Timedate& value,
    std::uint64_t amount,
    std::uint64_t units_per_day,
    Nanosecond ns_per_unit) noexcept
{
    Date updated_date = value.date();
    Time updated_time = value.time();

    const Day day_delta = amount / units_per_day;

    const Nanosecond remainder =
        (amount % units_per_day) * ns_per_unit;

    const Nanosecond combined =
        updated_time.to_nanoseconds() + remainder;

    const Day carry = combined / NS_PER_DAY;

    const Nanosecond new_time =
        combined % NS_PER_DAY;

    if (day_delta > std::numeric_limits<Day>::max() - carry)
        return false;

    if (!updated_date.add_day(day_delta + carry))
        return false;

    return value.set(
        updated_date,
        Time::from_nanoseconds(new_time)
    );
}

bool sub_time_units(
    Timedate& value,
    std::uint64_t amount,
    std::uint64_t units_per_day,
    Nanosecond ns_per_unit) noexcept
{
    Date updated_date = value.date();
    Time updated_time = value.time();

    Day day_delta = amount / units_per_day;

    const Nanosecond remainder =
        (amount % units_per_day) * ns_per_unit;

    const Nanosecond current =
        updated_time.to_nanoseconds();

    Nanosecond new_time;

    if (current < remainder)
    {
        if (day_delta == std::numeric_limits<Day>::max())
            return false;

        ++day_delta;
        new_time = NS_PER_DAY - (remainder - current);
    }
    else
    {
        new_time = current - remainder;
    }

    if (!updated_date.sub_day(day_delta))
        return false;

    return value.set(
        updated_date,
        Time::from_nanoseconds(new_time)
    );
}

}

// ============================================
// Constructor
// ============================================

Timedate::Timedate(const Date& date, const Time& time)
{
    if (!set(date, time))
        throw std::invalid_argument("invalid timedate");
}

// ============================================
// Getters
// ============================================

Date Timedate::date() const noexcept
{
    return date_;
}

Time Timedate::time() const noexcept
{
    return time_;
}

// ============================================
// Setters
// ============================================

bool Timedate::set(const Date& date, const Time& time) noexcept
{
    if (!date.is_valid() || !time.is_valid())
        return false;

    date_ = date;
    time_ = time;

    return true;
}

bool Timedate::set_date(const Date& value) noexcept
{
    if (!value.is_valid())
        return false;

    date_ = value;
    return true;
}

bool Timedate::set_time(const Time& value) noexcept
{
    if (!value.is_valid())
        return false;

    time_ = value;
    return true;
}

bool Timedate::set_year(Year value) noexcept
{
    Date updated = date_;

    if (!updated.set_year(value))
        return false;

    date_ = updated;
    return true;
}

bool Timedate::set_month(Month value) noexcept
{
    Date updated = date_;

    if (!updated.set_month(value))
        return false;

    date_ = updated;
    return true;
}

bool Timedate::set_day(Day value) noexcept
{
    return date_.set_day(value);
}

bool Timedate::set_hour(Hour value) noexcept
{
    return time_.set_hour(value);
}

bool Timedate::set_minute(Minute value) noexcept
{
    return time_.set_minute(value);
}

bool Timedate::set_second(Second value) noexcept
{
    return time_.set_second(value);
}

// ============================================
// Validation
// ============================================

bool Timedate::is_valid() const noexcept
{
    return date_.is_valid() && time_.is_valid();
}

// ============================================
// Add calendar units
// ============================================

bool Timedate::add_year(Year value) noexcept
{
    return date_.add_year(value);
}

bool Timedate::add_month(Month value) noexcept
{
    return date_.add_month(value);
}

bool Timedate::add_week(Week value) noexcept
{
    return date_.add_week(value);
}

bool Timedate::add_day(Day value) noexcept
{
    return date_.add_day(value);
}

// ============================================
// Add time units
// ============================================

bool Timedate::add_hour(Hour value) noexcept
{
    return add_time_units(*this, value, 24, NS_PER_HOUR);
}

bool Timedate::add_minute(Minute value) noexcept
{
    return add_time_units(*this, value, 1440, NS_PER_MINUTE);
}

bool Timedate::add_sec(Second value) noexcept
{
    return add_time_units(*this, value, 86400, NS_PER_SECOND);
}

bool Timedate::add_ms(Millisecond value) noexcept
{
    return add_time_units(*this, value, 86400000, 1'000'000ULL);
}

bool Timedate::add_us(Microsecond value) noexcept
{
    return add_time_units(*this, value, 86400000000ULL, 1'000ULL);
}

bool Timedate::add_ns(Nanosecond value) noexcept
{
    return add_time_units(*this, value, NS_PER_DAY, 1);
}

// ============================================
// Subtract calendar units
// ============================================

bool Timedate::sub_year(Year value) noexcept
{
    return date_.sub_year(value);
}

bool Timedate::sub_month(Month value) noexcept
{
    return date_.sub_month(value);
}

bool Timedate::sub_week(Week value) noexcept
{
    return date_.sub_week(value);
}

bool Timedate::sub_day(Day value) noexcept
{
    return date_.sub_day(value);
}

// ============================================
// Subtract time units
// ============================================

bool Timedate::sub_hour(Hour value) noexcept
{
    return sub_time_units(*this, value, 24, NS_PER_HOUR);
}

bool Timedate::sub_minute(Minute value) noexcept
{
    return sub_time_units(*this, value, 1440, NS_PER_MINUTE);
}

bool Timedate::sub_sec(Second value) noexcept
{
    return sub_time_units(*this, value, 86400, NS_PER_SECOND);
}

bool Timedate::sub_ms(Millisecond value) noexcept
{
    return sub_time_units(*this, value, 86400000, 1'000'000ULL);
}

bool Timedate::sub_us(Microsecond value) noexcept
{
    return sub_time_units(*this, value, 86400000000ULL, 1'000ULL);
}

bool Timedate::sub_ns(Nanosecond value) noexcept
{
    return sub_time_units(*this, value, NS_PER_DAY, 1);
}

// ============================================
// Operators
// Operand unit: nanoseconds
// ============================================

Timedate Timedate::operator+(Nanosecond value) const
{
    Timedate result = *this;

    if (!result.add_ns(value))
        throw std::overflow_error("timedate addition failed");

    return result;
}

Timedate Timedate::operator-(Nanosecond value) const
{
    Timedate result = *this;

    if (!result.sub_ns(value))
        throw std::overflow_error("timedate subtraction failed");

    return result;
}

Timedate& Timedate::operator+=(Nanosecond value)
{
    if (!add_ns(value))
        throw std::overflow_error("timedate addition failed");

    return *this;
}

Timedate& Timedate::operator-=(Nanosecond value)
{
    if (!sub_ns(value))
        throw std::overflow_error("timedate subtraction failed");

    return *this;
}

} // namespace tx