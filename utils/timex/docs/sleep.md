# `tx::sleep`

[`[Index]`](./index.md)

The `tx::sleep` namespace provides advanced, cross-platform precision sleep utilities leveraging native OS primitives (`nanosleep`, `clock_nanosleep`, etc.) for relative, monotonic, absolute calendar, and combined timestamp sleeping.

---

## Subsystem Synopsis

```cpp
namespace tx::sleep {

// Relative sleep
void sleep_basic(Time t);
void sleep_basic(Timedate td);
void sleep_basic(Hour hour, Minute minute, Second second = 1);

// Monotonic exact sleep
void sleep_exact(Time t);
void sleep_exact(Timedate td);
void sleep_exact(Second s, Millisecond ms = 0, Microsecond mi = 0, Nanosecond ns = 0);

// Absolute long-range calendar sleep
void sleep_long(Date d);
void sleep_long(Timedate td);
void sleep_long(Year y, Month m, Week w, Day d);

// Combined absolute calendar and time-of-day sleep
void sleep_all(Time t, Date d);
void sleep_all(Timedate td);
void sleep_all(
    Year y, Month m, Week w, Day d,
    Hour hour, Minute minute, Second s = 1,
    Millisecond ms = 0, Microsecond mi = 0, Nanosecond ns = 0
);

}
```

---

## Sleep Functions Reference

### 1. `sleep_basic` (Relative Sleep)

Performs relative sleeping using standard nanosecond/millisecond relative intervals.

| Signature | Description |
| :--- | :--- |
| `sleep_basic(Time t)` | Sleeps for the relative duration specified by `Time`. |
| `sleep_basic(Timedate td)` | Sleeps for the relative duration represented by `Timedate`. |
| `sleep_basic(Hour h, Minute m, Second s = 1)` | Sleeps for the specified relative hours, minutes, and seconds (defaults to 1 second). |

---

### 2. `sleep_exact` (Monotonic Exact Sleep)

Performs precise relative or absolute sleeping utilizing monotonic clocks (`clock_nanosleep`) to avoid system clock adjustments.

| Signature | Description |
| :--- | :--- |
| `sleep_exact(Time t)` | Sleeps precisely for the duration specified by `Time`. |
| `sleep_exact(Timedate td)` | Sleeps precisely for the duration specified by `Timedate`. |
| `sleep_exact(Second s, Millisecond ms = 0, Microsecond mi = 0, Nanosecond ns = 0)` | Sleeps for exact sub-second intervals (`s`, `ms`, `mi`, `ns`). |

---

### 3. `sleep_long` (Absolute Calendar Sleep)

Sleeps until a specific absolute calendar date or duration.

| Signature | Description |
| :--- | :--- |
| `sleep_long(Date d)` | Sleeps until the arrival of the target calendar `Date`. |
| `sleep_long(Timedate td)` | Sleeps until the target `Timedate`. |
| `sleep_long(Year y, Month m, Week w, Day d)` | Sleeps until a specific year, month, week, and day. |

---

### 4. `sleep_all` (Combined Calendar & Time Sleep)

Sleeps until an exact absolute timestamp combining calendar date and time of day.

| Signature | Description |
| :--- | :--- |
| `sleep_all(Time t, Date d)` | Sleeps until the combined absolute date and time. |
| `sleep_all(Timedate td)` | Sleeps until the specified `Timedate`. |
| `sleep_all(y, m, w, d, h, min, s, ms, mi, ns)` | Sleeps until an absolute timestamp specified component-wise. |

---

## Example Usage

```cpp
#include <tx/tx.hpp>
#include <tx/sleep.hpp>
#include <iostream>

int main() {
    using namespace tx;

    std::cout << "Sleeping for 2 seconds (basic)...\n";
    sleep::sleep_basic(0, 0, 2);

    std::cout << "Sleeping with monotonic precision (500ms)...\n";
    sleep::sleep_exact(0, 500, 0, 0);

    // Sleep until a specific future date and time
    Date target_date(2026, 12, 31);
    Time target_time(23, 59, 59);
    
    std::cout << "Sleeping until New Year's Eve...\n";
    // sleep::sleep_all(target_time, target_date);

    return 0;
}
```
