#include <tx/sleep.hpp>

// ==================================================
// Backend declarations
// ==================================================

#if defined(_WIN32) || defined(_WINDOWS)

    #include <zsleep/win/sleeps.hpp>

    #define TX_SLEEP_BASIC _win_sleep_basic
    #define TX_SLEEP_EXACT _win_sleep_exact
    #define TX_SLEEP_LONG  _win_sleep_long
    #define TX_SLEEP_ALL   _win_sleep_all

#elif defined(__linux__)

    #include <zsleep/linux/sleeps.hpp>

    #define TX_SLEEP_BASIC _linux_sleep_basic
    #define TX_SLEEP_EXACT _linux_sleep_exact
    #define TX_SLEEP_LONG  _linux_sleep_long
    #define TX_SLEEP_ALL   _linux_sleep_all

#elif defined(__FreeBSD__)

    #include <zsleep/freebsd/sleeps.hpp>

    #define TX_SLEEP_BASIC _freebsd_sleep_basic
    #define TX_SLEEP_EXACT _freebsd_sleep_exact
    #define TX_SLEEP_LONG  _freebsd_sleep_long
    #define TX_SLEEP_ALL   _freebsd_sleep_all

#elif defined(__NetBSD__)

    #include <zsleep/netbsd/sleeps.hpp>

    #define TX_SLEEP_BASIC _netbsd_sleep_basic
    #define TX_SLEEP_EXACT _netbsd_sleep_exact
    #define TX_SLEEP_LONG  _netbsd_sleep_long
    #define TX_SLEEP_ALL   _netbsd_sleep_all

#elif defined(__OpenBSD__)

    #include <zsleep/openbsd/sleeps.hpp>

    #define TX_SLEEP_BASIC _openbsd_sleep_basic
    #define TX_SLEEP_EXACT _openbsd_sleep_exact
    #define TX_SLEEP_LONG  _openbsd_sleep_long
    #define TX_SLEEP_ALL   _openbsd_sleep_all

#else

    #error "timex: unsupported operating system for tx::sleep"

#endif

// ==================================================
// Public API
// ==================================================

namespace tx::sleep {

// --------------------------------------------------
// sleep_basic
// --------------------------------------------------

void sleep_basic(Time t)
{
    if (!t.is_valid())
        return;

    TX_SLEEP_BASIC(t);
}

void sleep_basic(Timedate td)
{
    if (!td.date().is_valid() || !td.time().is_valid())
        return;

    TX_SLEEP_BASIC(td.time());
}

void sleep_basic(Hour hour, Minute minute, Second second)
{
    Time t(hour, minute, second);

    if (!t.is_valid())
        return;

    TX_SLEEP_BASIC(t);
}

// --------------------------------------------------
// sleep_exact
// --------------------------------------------------

void sleep_exact(Time t)
{
    if (!t.is_valid())
        return;

    TX_SLEEP_EXACT(t);
}

void sleep_exact(Timedate td)
{
    if (!td.date().is_valid() || !td.time().is_valid())
        return;

    TX_SLEEP_EXACT(td.time());
}

void sleep_exact(
    Second s,
    Millisecond ms,
    Microsecond mi,
    Nanosecond ns
) {
    Time t(0, 0, s, ms, mi, ns);

    if (!t.is_valid())
        return;

    TX_SLEEP_EXACT(t);
}

// --------------------------------------------------
// sleep_long
// --------------------------------------------------

void sleep_long(Date d)
{
    if (!d.is_valid())
        return;

    TX_SLEEP_LONG(d);
}

void sleep_long(Timedate td)
{
    if (!td.date().is_valid() || !td.time().is_valid())
        return;

    TX_SLEEP_LONG(td);
}

void sleep_long(Year y, Month m, Week w, Day d)
{
    Date target(y, m, 1);

    if (!target.is_valid() || d == 0)
        return;

    TX_SLEEP_LONG(y, m, w, d);
}

// --------------------------------------------------
// sleep_all
// --------------------------------------------------

void sleep_all(Time t, Date d)
{
    if (!t.is_valid() || !d.is_valid())
        return;

    TX_SLEEP_ALL(t, d);
}

void sleep_all(Timedate td)
{
    if (!td.date().is_valid() || !td.time().is_valid())
        return;

    TX_SLEEP_ALL(td);
}

void sleep_all(
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
    Date target(y, m, 1);
    Time target_time(hour, minute, s, ms, mi, ns);

    if (!target.is_valid() || d == 0 || !target_time.is_valid())
        return;

    TX_SLEEP_ALL(y, m, w, d, hour, minute, s, ms, mi, ns);
}

} // namespace tx::sleep

// ==================================================
// Cleanup
// ==================================================

#undef TX_SLEEP_BASIC
#undef TX_SLEEP_EXACT
#undef TX_SLEEP_LONG
#undef TX_SLEEP_ALL