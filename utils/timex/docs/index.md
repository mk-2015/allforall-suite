# Timex C++ Library (`tx`)

**Timex** (`tx`) is a compiled library C++ date and time manipulation library designed for high performance, ease of use, and robust validation. It provides strongly typed classes for managing times, dates, and combined timestamps with nanosecond precision.

---

## Navigation

- **Classes & Subsystems**
  - [`tx::Time`](./time.md) — Time of day with nanosecond precision
  - [`tx::Date`](./date.md) — Calendar date (Year, Month, Day) with leap year support
  - [`tx::Timedate`](./timedate.md) — Combined date and time representation
  - [`tx::sleep`](./sleep.md) — Cross-platform precision sleep utilities
- **Reference**
  - [Type Aliases](#type-aliases)
  - [Quick Start](#quick-start)

---

## Type Aliases

All fundamental temporal units are defined within the `tx` namespace as fixed-width integer type aliases:

| Type Alias | Underlying Type | Description |
| :--- | :--- | :--- |
| `tx::Year` | `std::uint64_t` | Calendar year |
| `tx::Month` | `std::uint64_t` | Month of the year (1–12) |
| `tx::Week` | `std::uint64_t` | Number of weeks |
| `tx::Day` | `std::uint64_t` | Day of the month / Days count |
| `tx::Hour` | `std::uint64_t` | Hour of the day (0–23) |
| `tx::Minute` | `std::uint64_t` | Minute of the hour (0–59) |
| `tx::Second` | `std::uint64_t` | Second of the minute (0–59) |
| `tx::Millisecond` | `std::uint64_t` | Millisecond component (0–999) |
| `tx::Microsecond` | `std::uint64_t` | Microsecond component (0–999) |
| `tx::Nanosecond` | `std::uint64_t` | Nanosecond component (0–999 / absolute ns) |

---

## Quick Start

Include the library header `<tx/tx.hpp>` and use the classes within the `tx` namespace:

```cpp
#include <tx/tx.hpp>
#include <iostream>

int main() {
    using namespace tx;

    // Construct a date and time
    Date date(2026, 6, 15);
    Time time(14, 30, 0);
    Timedate dt(date, time);

    if (dt.is_valid()) {
        std::cout << "Timestamp is valid!\n";
    }

    // Arithmetic operations
    dt.add_hour(2);
    dt.add_day(5);

    // Nanosecond conversion
    Nanosecond ns = time.to_nanoseconds();
    std::cout << "Nanoseconds since midnight: " << ns << "\n";

    return 0;
}
```

---

## Requirements

- **C++ Standard:** C++20 or later (utilizes `std::strong_ordering` and `<compare>`).
- **Dependencies:** None (pure C++ standard library).
