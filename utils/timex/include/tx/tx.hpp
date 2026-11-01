#pragma once

#include <cstdint>
#include <compare>

namespace tx
{

using Year        = std::uint64_t;
using Month       = std::uint64_t;
using Week        = std::uint64_t;
using Day         = std::uint64_t;
using Hour        = std::uint64_t;
using Minute      = std::uint64_t;
using Second      = std::uint64_t;
using Millisecond = std::uint64_t;
using Microsecond = std::uint64_t;
using Nanosecond  = std::uint64_t;


// ============================================
// Time
// ============================================

class Time
{
public:
    Time() = default;

    Time(
        Hour hour,
        Minute minute,
        Second second = 0,
        Millisecond millisecond = 0,
        Microsecond microsecond = 0,
        Nanosecond nanosecond = 0
    );

    Hour hour() const noexcept;
    Minute minute() const noexcept;
    Second second() const noexcept;

    Millisecond millisecond() const noexcept;
    Microsecond microsecond() const noexcept;
    Nanosecond nanosecond() const noexcept;

    bool set_hour(Hour value) noexcept;
    bool set_minute(Minute value) noexcept;
    bool set_second(Second value) noexcept;
    bool set_millisecond(Millisecond value) noexcept;
    bool set_microsecond(Microsecond value) noexcept;
    bool set_nanosecond(Nanosecond value) noexcept;

    bool add_hour(Hour) noexcept;
    bool add_minute(Minute) noexcept;
    bool add_sec(Second) noexcept;
    bool add_ms(Millisecond) noexcept;
    bool add_us(Microsecond) noexcept;
    bool add_ns(Nanosecond) noexcept;

    bool sub_hour(Hour) noexcept;
    bool sub_minute(Minute) noexcept;
    bool sub_sec(Second) noexcept;
    bool sub_ms(Millisecond) noexcept;
    bool sub_us(Microsecond) noexcept;
    bool sub_ns(Nanosecond) noexcept;

    Time operator+(Nanosecond value) const;
    Time operator-(Nanosecond value) const;
    Time& operator+=(Nanosecond value);
    Time& operator-=(Nanosecond value);

    bool set(
        Hour hour,
        Minute minute,
        Second second = 0,
        Millisecond millisecond = 0,
        Microsecond microsecond = 0,
        Nanosecond nanosecond = 0
    ) noexcept;

    bool is_valid() const noexcept;

    Nanosecond to_nanoseconds() const noexcept;

    static Time from_nanoseconds(Nanosecond value);

    auto operator<=>(const Time&) const = default;

private:
    Hour hours = 0;
    Minute minutes = 0;
    Second seconds = 0;
    Millisecond milliseconds = 0;
    Microsecond microseconds = 0;
    Nanosecond nanoseconds = 0;
};


// ============================================
// Date
// ============================================

class Date
{
public:
    Date() = default;

    Date(Year year, Month month, Day day);

    Year year() const noexcept;
    Month month() const noexcept;
    Day day() const noexcept;
    bool set_year(Year value) noexcept;
    bool set_month(Month value) noexcept;
    bool set_day(Day value) noexcept;

    bool add_year(Year) noexcept;
    bool add_month(Month) noexcept;
    bool add_week(Week) noexcept;
    bool add_day(Day) noexcept;

    bool sub_year(Year) noexcept;
    bool sub_month(Month) noexcept;
    bool sub_week(Week) noexcept;
    bool sub_day(Day) noexcept;

    Date operator+(Day value) const;
    Date operator-(Day value) const;
    Date& operator+=(Day value);
    Date& operator-=(Day value);

    bool set(Year year, Month month, Day day) noexcept;

    bool is_valid() const noexcept;
    bool is_leap_year() const noexcept;

    static bool is_leap_year(Year year) noexcept;
    static Day days_in_month(Year year, Month month) noexcept;

    auto operator<=>(const Date&) const = default;

private:
    Year years = 1970;
    Month months = 1;
    Day days = 1;
};


// ============================================
// Timedate
// ============================================

class Timedate
{
public:
    Timedate() = default;

    Timedate(const Date& date, const Time& time);

    Date date() const noexcept;
    Time time() const noexcept;

    bool set_date(const Date& value) noexcept;
    bool set_time(const Time& value) noexcept;

    bool set_year(Year value) noexcept;
    bool set_month(Month value) noexcept;
    bool set_day(Day value) noexcept;

    bool set_hour(Hour value) noexcept;
    bool set_minute(Minute value) noexcept;
    bool set_second(Second value) noexcept;

    bool add_year(Year) noexcept;
    bool add_month(Month) noexcept;
    bool add_week(Week) noexcept;
    bool add_day(Day) noexcept;

    bool add_hour(Hour) noexcept;
    bool add_minute(Minute) noexcept;
    bool add_sec(Second) noexcept;
    bool add_ms(Millisecond) noexcept;
    bool add_us(Microsecond) noexcept;
    bool add_ns(Nanosecond) noexcept;

    bool sub_year(Year) noexcept;
    bool sub_month(Month) noexcept;
    bool sub_week(Week) noexcept;
    bool sub_day(Day) noexcept;

    bool sub_hour(Hour) noexcept;
    bool sub_minute(Minute) noexcept;
    bool sub_sec(Second) noexcept;
    bool sub_ms(Millisecond) noexcept;
    bool sub_us(Microsecond) noexcept;
    bool sub_ns(Nanosecond) noexcept;

    Timedate operator+(Nanosecond value) const;
    Timedate operator-(Nanosecond value) const;
    Timedate& operator+=(Nanosecond value);
    Timedate& operator-=(Nanosecond value);

    bool set(const Date& date, const Time& time) noexcept;

    bool is_valid() const noexcept;

    auto operator<=>(const Timedate&) const = default;

private:
    Date date_;
    Time time_;
};

} // namespace tx