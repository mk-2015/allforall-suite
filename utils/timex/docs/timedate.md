# `tx::Timedate`

[`[Index]`](./index.md)

`tx::Timedate` combines a `tx::Date` and a `tx::Time` into a single timestamp object with nanosecond precision.

---

## Class Synopsis

```cpp
namespace tx {
class Timedate {
public:
    Timedate() = default;
    Timedate(const Date& date, const Time& time);

    // Getters
    Date date() const noexcept;
    Time time() const noexcept;

    // Component Setters
    bool set_date(const Date& value) noexcept;
    bool set_time(const Time& value) noexcept;
    bool set_year(Year value) noexcept;
    bool set_month(Month value) noexcept;
    bool set_day(Day value) noexcept;
    bool set_hour(Hour value) noexcept;
    bool set_minute(Minute value) noexcept;
    bool set_second(Second value) noexcept;
    bool set(const Date& date, const Time& time) noexcept;

    // Date Arithmetic
    bool add_year(Year) noexcept;
    bool add_month(Month) noexcept;
    bool add_week(Week) noexcept;
    bool add_day(Day) noexcept;

    bool sub_year(Year) noexcept;
    bool sub_month(Month) noexcept;
    bool sub_week(Week) noexcept;
    bool sub_day(Day) noexcept;

    // Time Arithmetic
    bool add_hour(Hour) noexcept;
    bool add_minute(Minute) noexcept;
    bool add_sec(Second) noexcept;
    bool add_ms(Millisecond) noexcept;
    bool add_us(Microsecond) noexcept;
    bool add_ns(Nanosecond) noexcept;

    bool sub_hour(Hour) noexcept;
    bool sub_minute(Minute) noexcept;
    bool sub_sec(Second) noexcept;
    bool sub_ms(Millisecond) noexcept;
    bool sub_us(Microsecond) noexcept;
    bool sub_ns(Nanosecond) noexcept;

    // Operators
    Timedate operator+(Nanosecond value) const;
    Timedate operator-(Nanosecond value) const;
    Timedate& operator+=(Nanosecond value);
    Timedate& operator-=(Nanosecond value);
    auto operator<=>(const Timedate&) const = default;

    // Validation
    bool is_valid() const noexcept;
};
}
```

---

## Member Functions

### Constructors

| Signature | Description |
| :--- | :--- |
| `Timedate()` | Constructs a default `Timedate` set to `1970-01-01 00:00:00.000000000`. |
| `Timedate(const Date& date, const Time& time)` | Constructs a combined `Timedate` from given `Date` and `Time` instances. |

### Accessors (Getters)

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `date() const noexcept` | `Date` | Returns the `Date` component. |
| `time() const noexcept` | `Time` | Returns the `Time` component. |

### Setters

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `set_date(const Date& value) noexcept` | `bool` | Sets the date component. Returns `true` on success. |
| `set_time(const Time& value) noexcept` | `bool` | Sets the time component. Returns `true` on success. |
| `set_year(Year value) noexcept` | `bool` | Sets the year on the date component. |
| `set_month(Month value) noexcept` | `bool` | Sets the month on the date component. |
| `set_day(Day value) noexcept` | `bool` | Sets the day on the date component. |
| `set_hour(Hour value) noexcept` | `bool` | Sets the hour on the time component. |
| `set_minute(Minute value) noexcept` | `bool` | Sets the minute on the time component. |
| `set_second(Second value) noexcept` | `bool` | Sets the second on the time component. |
| `set(const Date& d, const Time& t)` | `bool` | Sets both date and time components. |

### Arithmetic (Date & Time Operations)

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `add_year` / `sub_year` | `bool` | Adds/subtracts years on the date component. |
| `add_month` / `sub_month` | `bool` | Adds/subtracts months on the date component. |
| `add_week` / `sub_week` | `bool` | Adds/subtracts weeks on the date component. |
| `add_day` / `sub_day` | `bool` | Adds/subtracts days on the date component. |
| `add_hour` / `sub_hour` | `bool` | Adds/subtracts hours (may rollover date across midnight). |
| `add_minute` / `sub_minute` | `bool` | Adds/subtracts minutes (may rollover date). |
| `add_sec` / `sub_sec` | `bool` | Adds/subtracts seconds (may rollover date). |
| `add_ms` / `sub_ms` | `bool` | Adds/subtracts milliseconds (may rollover date). |
| `add_us` / `sub_us` | `bool` | Adds/subtracts microseconds (may rollover date). |
| `add_ns` / `sub_ns` | `bool` | Adds/subtracts nanoseconds (may rollover date). |

### Validation

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `is_valid() const noexcept` | `bool` | Returns `true` if both internal `Date` and `Time` are valid. |

---

## Example Usage

```cpp
#include <tx/tx.hpp>
#include <iostream>

int main() {
    using namespace tx;

    Date d(2026, 12, 31);
    Time t(23, 59, 50);
    Timedate dt(d, t);

    if (dt.is_valid()) {
        std::cout << "Initial: " << dt.date().year() << "-" 
                  << dt.date().month() << "-" << dt.date().day() << "\n";
    }

    // Add 20 seconds (rolls over to the next day / year)
    dt.add_sec(20);

    std::cout << "After +20s: " << dt.date().year() << "-" 
              << dt.date().month() << "-" << dt.date().day() << " "
              << dt.time().hour() << ":" << dt.time().minute() 
              << ":" << dt.time().second() << "\n"; // 2027-01-01 00:00:10
}
```
