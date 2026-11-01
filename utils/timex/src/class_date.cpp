#include <tx/tx.hpp>

#include <stdexcept>
#include <limits>

namespace tx
{

namespace
{

bool next_day(Date& date) noexcept
{
    const Year year = date.year();
    const Month month = date.month();
    const Day day = date.day();

    if (day < Date::days_in_month(year, month))
        return date.set_day(day + 1);

    if (month < 12)
        return date.set(year, month + 1, 1);

    if (year == std::numeric_limits<Year>::max())
        return false;

    return date.set(year + 1, 1, 1);
}

bool previous_day(Date& date) noexcept
{
    const Year year = date.year();
    const Month month = date.month();
    const Day day = date.day();

    if (day > 1)
        return date.set_day(day - 1);

    if (month > 1)
    {
        const Month previous_month = month - 1;

        return date.set(
            year,
            previous_month,
            Date::days_in_month(year, previous_month)
        );
    }

    if (year == 0)
        return false;

    return date.set(year - 1, 12, 31);
}

}

// ============================================
// Constructor
// ============================================

Date::Date(Year year, Month month, Day day)
{
    if (!set(year, month, day))
        throw std::invalid_argument("invalid date");
}

// ============================================
// Getters
// ============================================

Year Date::year() const noexcept { return years; }
Month Date::month() const noexcept { return months; }
Day Date::day() const noexcept { return days; }

// ============================================
// Setters
// ============================================

bool Date::set_year(Year value) noexcept
{
    if (days > days_in_month(value, months))
        return false;

    years = value;
    return true;
}

bool Date::set_month(Month value) noexcept
{
    if (value < 1 || value > 12 ||
        days > days_in_month(years, value))
        return false;

    months = value;
    return true;
}

bool Date::set_day(Day value) noexcept
{
    if (value < 1 || value > days_in_month(years, months))
        return false;

    days = value;
    return true;
}

bool Date::set(Year year, Month month, Day day) noexcept
{
    if (month < 1 || month > 12 ||
        day < 1 || day > days_in_month(year, month))
        return false;

    years = year;
    months = month;
    days = day;

    return true;
}

// ============================================
// Validation
// ============================================

bool Date::is_valid() const noexcept
{
    return months >= 1 && months <= 12 &&
           days >= 1 && days <= days_in_month(years, months);
}

// ============================================
// Leap year
// ============================================

bool Date::is_leap_year() const noexcept
{
    return is_leap_year(years);
}

bool Date::is_leap_year(Year year) noexcept
{
    return year % 400 == 0 ||
           (year % 4 == 0 && year % 100 != 0);
}

// ============================================
// Days in month
// ============================================

Day Date::days_in_month(Year year, Month month) noexcept
{
    switch (month)
    {
        case 1: case 3: case 5: case 7:
        case 8: case 10: case 12:
            return 31;

        case 4: case 6: case 9: case 11:
            return 30;

        case 2:
            return is_leap_year(year) ? 29 : 28;

        default:
            return 0;
    }
}

// ============================================
// Add year
// ============================================

bool Date::add_year(Year value) noexcept
{
    if (value > std::numeric_limits<Year>::max() - years)
        return false;

    const Year new_year = years + value;

    const Day max_day = days_in_month(new_year, months);
    const Day new_day = days > max_day ? max_day : days;

    years = new_year;
    days = new_day;

    return true;
}

// ============================================
// Subtract year
// ============================================

bool Date::sub_year(Year value) noexcept
{
    if (value > years)
        return false;

    const Year new_year = years - value;

    const Day max_day = days_in_month(new_year, months);
    const Day new_day = days > max_day ? max_day : days;

    years = new_year;
    days = new_day;

    return true;
}

// ============================================
// Add month
// ============================================

bool Date::add_month(Month value) noexcept
{
    const Month remaining = value % 12;
    const Year year_delta = value / 12;

    const Month month_index = months - 1 + remaining;

    const Year carry = month_index / 12;
    const Month new_month = month_index % 12 + 1;

    if (year_delta > std::numeric_limits<Year>::max() - years)
        return false;

    const Year intermediate = years + year_delta;

    if (carry > std::numeric_limits<Year>::max() - intermediate)
        return false;

    const Year new_year = intermediate + carry;

    const Day max_day = days_in_month(new_year, new_month);
    const Day new_day = days > max_day ? max_day : days;

    years = new_year;
    months = new_month;
    days = new_day;

    return true;
}

// ============================================
// Subtract month
// ============================================

bool Date::sub_month(Month value) noexcept
{
    const Year year_delta = value / 12;
    const Month remaining = value % 12;

    Year new_year = years;
    Month new_month;

    if (remaining < months)
    {
        new_month = months - remaining;
    }
    else
    {
        if (new_year == 0)
            return false;

        --new_year;
        new_month = 12 - (remaining - months);
    }

    if (year_delta > new_year)
        return false;

    new_year -= year_delta;

    const Day max_day = days_in_month(new_year, new_month);
    const Day new_day = days > max_day ? max_day : days;

    years = new_year;
    months = new_month;
    days = new_day;

    return true;
}

// ============================================
// Add week
// ============================================

bool Date::add_week(Week value) noexcept
{
    if (value > std::numeric_limits<Day>::max() / 7)
        return false;

    return add_day(value * 7);
}

// ============================================
// Subtract week
// ============================================

bool Date::sub_week(Week value) noexcept
{
    if (value > std::numeric_limits<Day>::max() / 7)
        return false;

    return sub_day(value * 7);
}

// ============================================
// Add day
// ============================================

bool Date::add_day(Day value) noexcept
{
    constexpr Day DAYS_PER_400_YEARS = 146097;

    Date result = *this;

    const Day cycles = value / DAYS_PER_400_YEARS;
    const Day remainder = value % DAYS_PER_400_YEARS;

    if (cycles != 0)
    {
        // A 400-year Gregorian cycle contains exactly 146097 days.
        const Year year_delta = cycles * 400;

        if (!result.add_year(year_delta))
            return false;
    }

    for (Day i = 0; i < remainder; ++i)
    {
        if (!next_day(result))
            return false;
    }

    *this = result;
    return true;
}

// ============================================
// Subtract day
// ============================================

bool Date::sub_day(Day value) noexcept
{
    constexpr Day DAYS_PER_400_YEARS = 146097;

    Date result = *this;

    const Day cycles = value / DAYS_PER_400_YEARS;
    const Day remainder = value % DAYS_PER_400_YEARS;

    if (cycles != 0)
    {
        const Year year_delta = cycles * 400;

        if (!result.sub_year(year_delta))
            return false;
    }

    for (Day i = 0; i < remainder; ++i)
    {
        if (!previous_day(result))
            return false;
    }

    *this = result;
    return true;
}

// ============================================
// Operators
// Operand unit: days
// ============================================

Date Date::operator+(Day value) const
{
    Date result = *this;

    if (!result.add_day(value))
        throw std::overflow_error("date addition failed");

    return result;
}

Date Date::operator-(Day value) const
{
    Date result = *this;

    if (!result.sub_day(value))
        throw std::overflow_error("date subtraction failed");

    return result;
}

Date& Date::operator+=(Day value)
{
    if (!add_day(value))
        throw std::overflow_error("date addition failed");

    return *this;
}

Date& Date::operator-=(Day value)
{
    if (!sub_day(value))
        throw std::overflow_error("date subtraction failed");

    return *this;
}

} // namespace tx