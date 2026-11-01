# `tx::Date`

[`[Index]`](./index.md)

`tx::Date` represents a calendar date consisting of year, month, and day.

---

## Class Synopsis

```cpp
namespace tx {
class Date {
public:
    Date() = default;
    Date(Year year, Month month, Day day);

    // Getters
    Year year() const noexcept;
    Month month() const noexcept;
    Day day() const noexcept;

    // Setters
    bool set_year(Year value) noexcept;
    bool set_month(Month value) noexcept;
    bool set_day(Day value) noexcept;
    bool set(Year year, Month month, Day day) noexcept;

    // Addition
    bool add_year(Year) noexcept;
    bool add_month(Month) noexcept;
    bool add_week(Week) noexcept;
    bool add_day(Day) noexcept;

    // Subtraction
    bool sub_year(Year) noexcept;
    bool sub_month(Month) noexcept;
    bool sub_week(Week) noexcept;
    bool sub_day(Day) noexcept;

    // Operators
    Date operator+(Day value) const;
    Date operator-(Day value) const;
    Date& operator+=(Day value);
    Date& operator-=(Day value);
    auto operator<=>(const Date&) const = default;

    // Validation & Helpers
    bool is_valid() const noexcept;
    bool is_leap_year() const noexcept;
    static bool is_leap_year(Year year) noexcept;
    static Day days_in_month(Year year, Month month) noexcept;
};
}
```

---

## Member Functions

### Constructors

| Signature | Description |
| :--- | :--- |
| `Date()` | Constructs a default `Date` set to `1970-01-01`. |
| `Date(Year y, Month m, Day d)` | Constructs a `Date` with specified year, month, and day. Validates against calendar rules. |

### Accessors (Getters)

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `year() const noexcept` | `Year` | Returns the year component. |
| `month() const noexcept` | `Month` | Returns the month component (1–12). |
| `day() const noexcept` | `Day` | Returns the day component (1–31). |

### Setters

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `set_year(Year value) noexcept` | `bool` | Sets the year. Returns `true` on success. |
| `set_month(Month value) noexcept` | `bool` | Sets the month if valid (1–12). Returns `true` on success. |
| `set_day(Day value) noexcept` | `bool` | Sets the day if valid for the current month/year. Returns `true` on success. |
| `set(y, m, d) noexcept` | `bool` | Sets all date components. Returns `true` if valid. |

### Arithmetic (Addition & Subtraction)

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `add_year(Year) noexcept` | `bool` | Adds years (handles leap day adjustments). |
| `add_month(Month) noexcept` | `bool` | Adds months (handles month-end clamping). |
| `add_week(Week) noexcept` | `bool` | Adds weeks (7 days per week). |
| `add_day(Day) noexcept` | `bool` | Adds days across month and year boundaries. |
| `sub_year(Year) noexcept` | `bool` | Subtracts years. |
| `sub_month(Month) noexcept` | `bool` | Subtracts months. |
| `sub_week(Week) noexcept` | `bool` | Subtracts weeks. |
| `sub_day(Day) noexcept` | `bool` | Subtracts days. |

### Validation & Calendar Helpers

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `is_valid() const noexcept` | `bool` | Checks if the date is a valid calendar date. |
| `is_leap_year() const noexcept` | `bool` | Returns `true` if the date's year is a leap year. |
| `is_leap_year(Year year)` | `bool` (static) | Returns `true` if the specified year is a leap year. |
| `days_in_month(Year y, Month m)` | `Day` (static) | Returns the number of days in the given month and year. |

---

## Example Usage

```cpp
#include <tx/tx.hpp>
#include <iostream>

int main() {
    using namespace tx;

    Date d(2024, 2, 28);

    if (d.is_valid()) {
        std::cout << "Initial Date: " << d.year() << "-" 
                  << d.month() << "-" << d.day() << "\n";
    }

    // Check leap year
    if (Date::is_leap_year(2024)) {
        std::cout << "2024 is a leap year!\n";
    }

    // Add 2 days (crosses Feb 29 leap day)
    d.add_day(2);
    std::cout << "After adding 2 days: " << d.year() << "-" 
              << d.month() << "-" << d.day() << "\n"; // 2024-03-01
}
```
