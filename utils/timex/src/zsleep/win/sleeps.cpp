#if defined(_WINDOWS)

#include <zsleep/win/sleeps.hpp>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <limits>

namespace tx::sleep {

namespace {

constexpr std::uint64_t NS_PER_MS = 1'000'000ULL;
constexpr std::uint64_t NS_PER_SEC = 1'000'000'000ULL;
constexpr std::uint64_t NS_PER_100NS = 100ULL;

#ifdef _WEEK_NO_HOLIDAYS
constexpr std::uint64_t DAYS_PER_WEEK = 5;
#else
constexpr std::uint64_t DAYS_PER_WEEK = 7;
#endif

// Ordinary Windows sleep.
void basic_wait(std::uint64_t ns) noexcept
{
    while (ns > 0) {
        const std::uint64_t ms = std::min<std::uint64_t>(
            ns / NS_PER_MS + (ns % NS_PER_MS != 0),
            MAXDWORD
        );

        ::Sleep(static_cast<DWORD>(ms));

        const std::uint64_t elapsed = ms * NS_PER_MS;

        if (elapsed >= ns)
            break;

        ns -= elapsed;
    }
}

// High-resolution Windows waitable timer.
void exact_wait(std::uint64_t ns) noexcept
{
    if (ns == 0)
        return;

    HANDLE timer = ::CreateWaitableTimerExW(
        nullptr,
        nullptr,
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
        TIMER_ALL_ACCESS
    );

    // Fall back when high-resolution timers are unavailable.
    if (!timer) {
        timer = ::CreateWaitableTimerW(nullptr, FALSE, nullptr);
    }

    if (!timer) {
        basic_wait(ns);
        return;
    }

    constexpr std::uint64_t MAX_CHUNK =
        24ULL * 60 * 60 * NS_PER_SEC;

    while (ns > 0) {
        const std::uint64_t chunk = std::min(ns, MAX_CHUNK);

        // Negative due time means relative time.
        LARGE_INTEGER due{};
        due.QuadPart = -static_cast<LONGLONG>(
            (chunk + NS_PER_100NS - 1) / NS_PER_100NS
        );

        if (!::SetWaitableTimer(
                timer,
                &due,
                0,
                nullptr,
                nullptr,
                FALSE
            )) {
            basic_wait(ns);
            break;
        }

        const DWORD result = ::WaitForSingleObject(
            timer,
            INFINITE
        );

        if (result != WAIT_OBJECT_0) {
            basic_wait(ns);
            break;
        }

        ns -= chunk;
    }

    ::CancelWaitableTimer(timer);
    ::CloseHandle(timer);
}

// Convert local calendar date/time to UTC FILETIME.
bool to_utc_filetime(
    const Date& date,
    const Time& time,
    FILETIME& result
) noexcept {
    if (!date.is_valid() || !time.is_valid())
        return false;

    if (date.year() > 30827)
        return false;

    SYSTEMTIME local{};

    local.wYear = static_cast<WORD>(date.year());
    local.wMonth = static_cast<WORD>(date.month());
    local.wDay = static_cast<WORD>(date.day());

    local.wHour = static_cast<WORD>(time.hour());
    local.wMinute = static_cast<WORD>(time.minute());
    local.wSecond = static_cast<WORD>(time.second());
    local.wMilliseconds = static_cast<WORD>(time.millisecond());

    SYSTEMTIME utc{};

    if (!::TzSpecificLocalTimeToSystemTime(
            nullptr,
            &local,
            &utc
        )) {
        return false;
    }

    return ::SystemTimeToFileTime(&utc, &result) != FALSE;
}

// Wait until an absolute local calendar target.
void wait_until(
    const Date& date,
    const Time& time,
    bool high_resolution
) noexcept {
    FILETIME target{};

    if (!to_utc_filetime(date, time, target))
        return;

    for (;;) {
        FILETIME current{};
        ::GetSystemTimeAsFileTime(&current);

        ULARGE_INTEGER target_value{};
        ULARGE_INTEGER current_value{};

        target_value.LowPart = target.dwLowDateTime;
        target_value.HighPart = target.dwHighDateTime;

        current_value.LowPart = current.dwLowDateTime;
        current_value.HighPart = current.dwHighDateTime;

        if (current_value.QuadPart >= target_value.QuadPart)
            return;

        const std::uint64_t remaining_100ns =
            target_value.QuadPart - current_value.QuadPart;

        const std::uint64_t remaining_ns =
            remaining_100ns * NS_PER_100NS;

        if (high_resolution)
            exact_wait(remaining_ns);
        else
            basic_wait(remaining_ns);

        // Recheck the wall clock after waking.
    }
}

// Construct a target date from year/month/week/day.
bool make_target_date(
    Year y,
    Month m,
    Week w,
    Day d,
    Date& result
) noexcept {
    if (m < 1 || m > 12 || d == 0)
        return false;

    if (y > 30827)
        return false;

    Date target(y, m, 1);

    if (!target.is_valid())
        return false;

    // Weeks and days are offsets from the first of the month.
    if (w > std::numeric_limits<std::uint64_t>::max()
                / DAYS_PER_WEEK) {
        return false;
    }

    const std::uint64_t week_days = w * DAYS_PER_WEEK;

    if (week_days > std::numeric_limits<std::uint64_t>::max() - (d - 1))
        return false;

    const std::uint64_t offset = week_days + d - 1;

    target.add_day(offset);

    if (!target.is_valid())
        return false;

    result = target;
    return true;
}

} // namespace

// --------------------------------------------------
// sleep_basic
// --------------------------------------------------

void _win_sleep_basic(Time t)
{
    if (!t.is_valid())
        return;

    basic_wait(t.to_nanoseconds());
}

// --------------------------------------------------
// sleep_exact
// --------------------------------------------------

void _win_sleep_exact(Time t)
{
    if (!t.is_valid())
        return;

    exact_wait(t.to_nanoseconds());
}

// --------------------------------------------------
// sleep_long
// --------------------------------------------------

void _win_sleep_long(Date d)
{
    if (!d.is_valid())
        return;

    wait_until(d, Time(0, 0, 0), false);
}

void _win_sleep_long(Timedate td)
{
    if (!td.date().is_valid() || !td.time().is_valid())
        return;

    wait_until(td.date(), td.time(), false);
}

void _win_sleep_long(
    Year y,
    Month m,
    Week w,
    Day d
) {
    Date target(0, 1, 1);

    if (!make_target_date(y, m, w, d, target))
        return;

    _win_sleep_long(target);
}

// --------------------------------------------------
// sleep_all
// --------------------------------------------------

void _win_sleep_all(Time t, Date d)
{
    if (!d.is_valid() || !t.is_valid())
        return;

    wait_until(d, t, true);
}

void _win_sleep_all(Timedate td)
{
    if (!td.date().is_valid() || !td.time().is_valid())
        return;

    wait_until(td.date(), td.time(), true);
}

void _win_sleep_all(
    Year y,
    Month m,
    Week w,
    Day d,
    Hour hour,
    Minute minute,
    Second s,
    Millisecond ms,
    Microsecond mi,
    Nanosecond ns
) {
    Date target(0, 1, 1);

    if (!make_target_date(y, m, w, d, target))
        return;

    Time target_time(hour, minute, s, ms, mi, ns);

    if (!target_time.is_valid())
        return;

    _win_sleep_all(target_time, target);
}

} // namespace tx::sleep

#else

#warning "win is not the current OS"

#endif