# `tx::Time`

[`[Index]`](./index.md)

`tx::Time` represents a time of day with sub-second precision down to nanoseconds.

---

## Class Synopsis

```cpp
namespace tx {
class Time {
public:
    Time() = default;
    Time(Hour hour, Minute minute, Second second = 0,
         Millisecond millisecond = 0, Microsecond microsecond = 0,
         Nanosecond nanosecond = 0);

    // Getters
    Hour hour() const noexcept;
    Minute minute() const noexcept;
    Second second() const noexcept;
    Millisecond millisecond() const noexcept;
    Microsecond microsecond() const noexcept;
    Nanosecond nanosecond() const noexcept;

    // Setters
    bool set_hour(Hour value) noexcept;
    bool set_minute(Minute value) noexcept;
    bool set_second(Second value) noexcept;
    bool set_millisecond(Millisecond value) noexcept;
    bool set_microsecond(Microsecond value) noexcept;
    bool set_nanosecond(Nanosecond value) noexcept;
    bool set(Hour hour, Minute minute, Second second = 0,
             Millisecond millisecond = 0, Microsecond microsecond = 0,
             Nanosecond nanosecond = 0) noexcept;

    // Addition
    bool add_hour(Hour) noexcept;
    bool add_minute(Minute) noexcept;
    bool add_sec(Second) noexcept;
    bool add_ms(Millisecond) noexcept;
    bool add_us(Microsecond) noexcept;
    bool add_ns(Nanosecond) noexcept;

    // Subtraction
    bool sub_hour(Hour) noexcept;
    bool sub_minute(Minute) noexcept;
    bool sub_sec(Second) noexcept;
    bool sub_ms(Millisecond) noexcept;
    bool sub_us(Microsecond) noexcept;
    bool sub_ns(Nanosecond) noexcept;

    // Operators
    Time operator+(Nanosecond value) const;
    Time operator-(Nanosecond value) const;
    Time& operator+=(Nanosecond value);
    Time& operator-=(Nanosecond value);
    auto operator<=>(const Time&) const = default;

    // Validation & Conversion
    bool is_valid() const noexcept;
    Nanosecond to_nanoseconds() const noexcept;
    static Time from_nanoseconds(Nanosecond value);
};
}
```

---

## Member Functions

### Constructors

| Signature | Description |
| :--- | :--- |
| `Time()` | Constructs a default `Time` object set to `00:00:00.000000000`. |
| `Time(Hour h, Minute m, Second s = 0, Millisecond ms = 0, Microsecond us = 0, Nanosecond ns = 0)` | Constructs a `Time` with specified components. Values are validated. |

### Accessors (Getters)

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `hour() const noexcept` | `Hour` | Returns the hour component (0–23). |
| `minute() const noexcept` | `Minute` | Returns the minute component (0–59). |
| `second() const noexcept` | `Second` | Returns the second component (0–59). |
| `millisecond() const noexcept` | `Millisecond` | Returns the millisecond component (0–999). |
| `microsecond() const noexcept` | `Microsecond` | Returns the microsecond component (0–999). |
| `nanosecond() const noexcept` | `Nanosecond` | Returns the nanosecond component (0–999). |

### Setters

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `set_hour(Hour value) noexcept` | `bool` | Sets the hour if valid (0–23). Returns `true` on success. |
| `set_minute(Minute value) noexcept` | `bool` | Sets the minute if valid (0–59). Returns `true` on success. |
| `set_second(Second value) noexcept` | `bool` | Sets the second if valid (0–59). Returns `true` on success. |
| `set_millisecond(Millisecond value) noexcept` | `bool` | Sets the millisecond if valid (0–999). Returns `true` on success. |
| `set_microsecond(Microsecond value) noexcept` | `bool` | Sets the microsecond if valid (0–999). Returns `true` on success. |
| `set_nanosecond(Nanosecond value) noexcept` | `bool` | Sets the nanosecond if valid (0–999). Returns `true` on success. |
| `set(h, m, s, ms, us, ns) noexcept` | `bool` | Sets all time components. Returns `true` if all are valid. |

### Arithmetic (Addition & Subtraction)

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `add_hour(Hour) noexcept` | `bool` | Adds hours (supports wraparound across midnight). |
| `add_minute(Minute) noexcept` | `bool` | Adds minutes (supports wraparound). |
| `add_sec(Second) noexcept` | `bool` | Adds seconds (supports wraparound). |
| `add_ms(Millisecond) noexcept` | `bool` | Adds milliseconds (supports wraparound). |
| `add_us(Microsecond) noexcept` | `bool` | Adds microseconds (supports wraparound). |
| `add_ns(Nanosecond) noexcept` | `bool` | Adds nanoseconds (supports wraparound). |
| `sub_hour(Hour) noexcept` | `bool` | Subtracts hours. |
| `sub_minute(Minute) noexcept` | `bool` | Subtracts minutes. |
| `sub_sec(Second) noexcept` | `bool` | Subtracts seconds. |
| `sub_ms(Millisecond) noexcept` | `bool` | Subtracts milliseconds. |
| `sub_us(Microsecond) noexcept` | `bool` | Subtracts microseconds. |
| `sub_ns(Nanosecond) noexcept` | `bool` | Subtracts nanoseconds. |

### Conversion & Validation

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `is_valid() const noexcept` | `bool` | Checks if time components are within legal ranges. |
| `to_nanoseconds() const noexcept` | `Nanosecond` | Converts the time of day into total elapsed nanoseconds since midnight. |
| `from_nanoseconds(Nanosecond value)` | `Time` (static) | Factory method creating a `Time` from total nanoseconds since midnight. |

---

## Validation & Error Handling

`tx::Time` enforces strict validation rules on all components:
- **Hours:** `0` to `23`
- **Minutes:** `0` to `59`
- **Seconds:** `0` to `59`
- **Milliseconds, Microseconds, Nanoseconds:** `0` to `999`

When attempting to assign invalid values using setters (e.g., `set_hour(24)` or `set_minute(60)`), the operation fails safely, returns `false`, and leaves the object's existing valid state unmodified.

---

## Example Usage

```cpp
#include <tx/tx.hpp>
#include <iostream>

int main() {
    using namespace tx;

    Time t(12, 30, 45, 500);

    if (t.is_valid()) {
        std::cout << "Time: " << t.hour() << ":" << t.minute() 
                  << ":" << t.second() << "." << t.millisecond() << "\n";
    }

    // Add 2 hours and 15 minutes
    t.add_hour(2);
    t.add_minute(15);

    // Convert to total nanoseconds since midnight
    Nanosecond total_ns = t.to_nanoseconds();
    std::cout << "Total ns: " << total_ns << "\n";

    // Reconstruct from nanoseconds
    Time restored = Time::from_nanoseconds(total_ns);
    if (restored == t) {
        std::cout << "Round-trip successful!\n";
    }
}
```

---

## `tx::time` Namespace Utilities

The `tx::time` namespace provides helper functions for retrieving the current system time in various representations and time types (`UTC`, `GMT`, `UNIX`, `Local`), as well as advanced formatting utilities supporting custom tokens and printf-style variadic formatting.

### Namespace Synopsis

```cpp
namespace tx::time {

enum class TimeType {
    UTC,
    GMT,
    UNIX,
    Local
};

// Current time retrieval
tx::Second time_now(TimeType type = TimeType::UTC);
tx::Nanosecond time_now_ns(TimeType type = TimeType::UTC);
tx::Time time_now_time(TimeType type = TimeType::UTC);
tx::Timedate time_now_timedate(TimeType type = TimeType::UTC);

// Formatting functions
std::string time_format(
    tx::Timedate* time,
    const std::string& format,
    TimeType type, ...
);

std::string time_format(
    tx::Time* time,
    const std::string& format,
    TimeType type, ...
);

std::string time_format(
    tx::Date* time,
    const std::string& format,
    TimeType type, ...
);

std::string time_format(
    tx::Second* time,
    const std::string& format,
    TimeType type, ...
);

std::string time_format_ns(
    tx::Nanosecond* time,
    const std::string& format,
    TimeType type = TimeType::UTC, ...
);

}
```

---

### `TimeType` Enumeration

Defines the time base / standard used when querying current time or formatting timestamps:

| Enumerator | Description |
| :--- | :--- |
| `TimeType::UTC` | Coordinated Universal Time. |
| `TimeType::GMT` | Greenwich Mean Time. |
| `TimeType::UNIX` | Unix epoch time. |
| `TimeType::Local` | Local system time zone. |

---

### Current Time Functions Reference

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `time_now(TimeType type)` | `tx::Second` | Returns current elapsed seconds according to the specified `TimeType`. |
| `time_now_ns(TimeType type)` | `tx::Nanosecond` | Returns current elapsed nanoseconds according to the specified `TimeType`. |
| `time_now_time(TimeType type)` | `tx::Time` | Returns current time of day (`tx::Time`) according to the specified `TimeType`. |
| `time_now_timedate(TimeType type)` | `tx::Timedate` | Returns current combined date and time (`tx::Timedate`) according to the specified `TimeType`. |

---

### Formatting Tokens

`time_format` and `time_format_ns` support the following specialized formatting tokens:

| Token | Description | Format / Range |
| :--- | :--- | :--- |
| `%Y` | Year | 4-digit year (e.g., `2026`) |
| `%y` | Year (2 digits) | 2-digit year (e.g., `26`) |
| `%m` | Month | `01` – `12` |
| `%d` | Day | `01` – `31` |
| `%H` | Hour (24-hour) | `00` – `23` |
| `%I` | Hour (12-hour) | `01` – `12` |
| `%M` | Minute | `00` – `59` |
| `%S` | Second | `00` – `59` |
| `%MS` | Millisecond | `000` – `999` |
| `%f` | Microsecond | `000000` – `999999` |
| `%N` | Nanosecond | `000000000` – `999999999` |
| `%p` | AM/PM | `AM` or `PM` |

Remaining printf-style conversions (e.g. `%s`, `%d`, etc.) use variadic arguments passed to `time_format`.

---

### Formatting Functions Reference

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `time_format(tx::Timedate* time, const std::string& format, TimeType type, ...)` | `std::string` | Formats a `Timedate` pointer using custom tokens and optional variadic arguments. |
| `time_format(tx::Time* time, const std::string& format, TimeType type, ...)` | `std::string` | Formats a `Time` pointer using custom tokens and optional variadic arguments. |
| `time_format(tx::Date* time, const std::string& format, TimeType type, ...)` | `std::string` | Formats a `Date` pointer using custom tokens and optional variadic arguments. |
| `time_format(tx::Second* time, const std::string& format, TimeType type, ...)` | `std::string` | Formats a `Second` timestamp pointer using custom tokens and optional variadic arguments. |
| `time_format_ns(tx::Nanosecond* time, const std::string& format, TimeType type = TimeType::UTC, ...)` | `std::string` | Formats a `Nanosecond` timestamp pointer using custom tokens and optional variadic arguments. |

---

### Example Usage (`tx::time`)

```cpp
#include <tx/tx.hpp>
#include <tx/time.hpp>
#include <iostream>

int main() {
    using namespace tx;

    // Get current time as a Timedate object in UTC
    Timedate now_utc = time::time_now_timedate(time::TimeType::UTC);

    // Format current time using custom tokens
    std::string formatted = time::time_format(
        &now_utc,
        "Current UTC: %Y-%m-%d %H:%M:%S.%MS (%p)",
        time::TimeType::UTC
    );

    std::cout << formatted << "\n";
}
```
