#pragma once

#include <tx/tx.hpp>

namespace tx::sleep {

void _freebsd_sleep_basic(Time t);
void _freebsd_sleep_exact(Time t);

void _freebsd_sleep_long(Date d);
void _freebsd_sleep_long(Timedate td);
void _freebsd_sleep_long(Year y, Month m, Week w, Day d);

void _freebsd_sleep_all(Time t, Date d);
void _freebsd_sleep_all(Timedate td);
void _freebsd_sleep_all(
    Year y, Month m, Week w, Day d,
    Hour hour, Minute minute, Second s,
    Millisecond ms, Microsecond mi, Nanosecond ns
);

} // namespace tx::sleep