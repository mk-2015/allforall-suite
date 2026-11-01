#if defined(__NetBSD__)

#include <zsleep/netbsd/sleeps.hpp>

#include <cerrno>
#include <cstdint>
#include <ctime>
#include <limits>

namespace tx::sleep {

namespace {

constexpr std::uint64_t NS_PER_SEC = 1'000'000'000ULL;

#ifdef _WEEK_NO_HOLIDAYS
constexpr std::uint64_t DAYS_PER_WEEK = 5;
#else
constexpr std::uint64_t DAYS_PER_WEEK = 7;
#endif

// --------------------------------------------------
// Basic native sleep
// --------------------------------------------------

void basic_wait(std::uint64_t ns) noexcept
{
    while (ns > 0) {
        timespec req{};

        req.tv_sec = static_cast<time_t>(ns / NS_PER_SEC);
        req.tv_nsec = static_cast<long>(ns % NS_PER_SEC);

        timespec rem{};

        if (::nanosleep(&req, &rem) == 0)
            return;

        if (errno != EINTR)
            return;

        req = rem;

        while (::nanosleep(&req, &rem) == -1) {
            if (errno != EINTR)
                return;

            req = rem;
        }

        return;
    }
}

// --------------------------------------------------
// Exact monotonic sleep
// --------------------------------------------------

void exact_wait(std::uint64_t ns) noexcept
{
    if (ns == 0)
        return;

    timespec deadline{};

    if (::clock_gettime(CLOCK_MONOTONIC, &deadline) != 0)
        return;

    const std::uint64_t add_sec = ns / NS_PER_SEC;
    const std::uint64_t add_ns = ns % NS_PER_SEC;

    if (add_sec > static_cast<std::uint64_t>(
            std::numeric_limits<time_t>::max() - deadline.tv_sec))
        return;

    deadline.tv_sec += static_cast<time_t>(add_sec);
    deadline.tv_nsec += static_cast<long>(add_ns);

    if (deadline.tv_nsec >= static_cast<long>(NS_PER_SEC)) {
        ++deadline.tv_sec;
        deadline.tv_nsec -= static_cast<long>(NS_PER_SEC);
    }

    int result;

    do {
        result = ::clock_nanosleep(
            CLOCK_MONOTONIC,
            TIMER_ABSTIME,
            &deadline,
            nullptr
        );
    } while (result == EINTR);
}

// --------------------------------------------------
// Local calendar conversion
// --------------------------------------------------

bool to_timespec(
    const Date& date,
    const Time& time,
    timespec& result
) noexcept {
    if (!date.is_valid() || !time.is_valid())
        return false;

    if (date.year() > static_cast<Year>(
            std::numeric_limits<int>::max() - 1900))
        return false;

    std::tm target{};

    target.tm_year = static_cast<int>(date.year()) - 1900;
    target.tm_mon = static_cast<int>(date.month()) - 1;
    target.tm_mday = static_cast<int>(date.day());

    target.tm_hour = static_cast<int>(time.hour());
    target.tm_min = static_cast<int>(time.minute());
    target.tm_sec = static_cast<int>(time.second());

    target.tm_isdst = -1;

    const std::time_t epoch = ::mktime(&target);

    if (epoch == static_cast<std::time_t>(-1))
        return false;

    result.tv_sec = epoch;

    result.tv_nsec =
        static_cast<long>(time.millisecond() * 1'000'000ULL) +
        static_cast<long>(time.microsecond() * 1'000ULL) +
        static_cast<long>(time.nanosecond());

    return true;
}

// --------------------------------------------------
// Absolute calendar sleep
// --------------------------------------------------

void wait_until(
    const Date& date,
    const Time& time
) noexcept {
    timespec target{};

    if (!to_timespec(date, time, target))
        return;

    for (;;) {
        const int result = ::clock_nanosleep(
            CLOCK_REALTIME,
            TIMER_ABSTIME,
            &target,
            nullptr
        );

        if (result == 0 || result != EINTR)
            return;
    }
}

// --------------------------------------------------
// Date construction
// --------------------------------------------------

bool make_target_date(
    Year y,
    Month m,
    Week w,
    Day d,
    Date& result
) noexcept {
    if (m < 1 || m > 12 || d == 0)
        return false;

    Date target(y, m, 1);

    if (!target.is_valid())
        return false;

    if (w > std::numeric_limits<std::uint64_t>::max()
                / DAYS_PER_WEEK)
        return false;

    const std::uint64_t week_days = w * DAYS_PER_WEEK;

    if (week_days > std::numeric_limits<std::uint64_t>::max() - (d - 1))
        return false;

    target.add_day(week_days + d - 1);

    if (!target.is_valid())
        return false;

    result = target;
    return true;
}

} // namespace

// --------------------------------------------------
// sleep_basic
// --------------------------------------------------

void _netbsd_sleep_basic(Time t)
{
    if (!t.is_valid())
        return;

    basic_wait(t.to_nanoseconds());
}

// --------------------------------------------------
// sleep_exact
// --------------------------------------------------

void _netbsd_sleep_exact(Time t)
{
    if (!t.is_valid())
        return;

    exact_wait(t.to_nanoseconds());
}

// --------------------------------------------------
// sleep_long
// --------------------------------------------------

void _netbsd_sleep_long(Date d)
{
    if (!d.is_valid())
        return;

    wait_until(d, Time(0, 0, 0));
}

void _netbsd_sleep_long(Timedate td)
{
    if (!td.date().is_valid() || !td.time().is_valid())
        return;

    wait_until(td.date(), td.time());
}

void _netbsd_sleep_long(
    Year y,
    Month m,
    Week w,
    Day d
) {
    Date target(0, 1, 1);

    if (!make_target_date(y, m, w, d, target))
        return;

    _netbsd_sleep_long(target);
}

// --------------------------------------------------
// sleep_all
// --------------------------------------------------

void _netbsd_sleep_all(Time t, Date d)
{
    if (!t.is_valid() || !d.is_valid())
        return;

    wait_until(d, t);
}

void _netbsd_sleep_all(Timedate td)
{
    if (!td.date().is_valid() || !td.time().is_valid())
        return;

    wait_until(td.date(), td.time());
}

void _netbsd_sleep_all(
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

    _netbsd_sleep_all(target_time, target);
}

} // namespace tx::sleep

#else

#warning "netbsd is not the current OS"

#endif