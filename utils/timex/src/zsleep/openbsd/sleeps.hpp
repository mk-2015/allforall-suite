#pragma once

#include <tx/tx.hpp>

namespace tx::sleep {

void _openbsd_sleep_basic(Time t);
void _openbsd_sleep_exact(Time t);

void _openbsd_sleep_long(Date d);
void _openbsd_sleep_long(Timedate td);
void _openbsd_sleep_long(Year y, Month m, Week w, Day d);

void _openbsd_sleep_all(Time t, Date d);
void _openbsd_sleep_all(Timedate td);
void _openbsd_sleep_all(
    Year y, Month m, Week w, Day d,
    Hour hour, Minute minute, Second s,
    Millisecond ms, Microsecond mi, Nanosecond ns
);

} // namespace tx::sleep