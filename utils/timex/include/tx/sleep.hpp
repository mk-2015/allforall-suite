#pragma once

#include <tx/tx.hpp>

namespace tx::sleep {

void sleep_basic(Time t);
void sleep_basic(Timedate td);
void sleep_basic(Hour hour, Minute minute, Second second = 1);

void sleep_exact(Time t);
void sleep_exact(Timedate td);
void sleep_exact(
    Second s,
    Millisecond ms = 0,
    Microsecond mi = 0,
    Nanosecond ns = 0
);

void sleep_long(Date d);
void sleep_long(Timedate td);
void sleep_long(Year y, Month m, Week w, Day d);

void sleep_all(Time t, Date d);
void sleep_all(Timedate td);
void sleep_all(
    Year y, Month m, Week w, Day d,
    Hour hour, Minute minute, Second s = 1,
    Millisecond ms = 0,
    Microsecond mi = 0,
    Nanosecond ns = 0
);

} // namespace tx::sleep