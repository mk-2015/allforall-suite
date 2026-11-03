# `tx::zone`

[`[Index]`](./index.md)

The `tx::zone` namespace provides robust time zone management, canonicalization, system time zone detection, and historical transition querying leveraging ICU (International Components for Unicode).

---

## Subsystem Synopsis

```cpp
namespace tx::zone {

struct ZoneInfo {
    tx::Second begin = 0;
    tx::Second end = 0;       // Exclusive transition boundary
    tx::Offset offset = 0;    // Total UTC offset in seconds
    tx::Offset save = 0;      // Daylight Saving Time (DST) adjustment in seconds
    std::string abbrev;       // Timezone abbreviation (e.g., "EST", "EDT")
    bool is_dst = false;      // Whether DST is active
};

// Returns the current active system or selected timezone identifier
std::string current_zone();

// Returns a sorted vector of all canonical ICU/IANA timezone identifiers
std::vector<std::string> available_zones();

// Selects and sets the active global timezone (supports IANA and Windows IDs)
void select_zone(const std::string& zone);

// Queries timezone information for the current time in the active zone
ZoneInfo zone_info();

// Queries timezone information for a specific Unix timestamp in a given zone
ZoneInfo zone_info(
    const std::string& zone,
    tx::Second timestamp
);

}
```

---

## Structures & Functions Reference

### `ZoneInfo` Structure

Contains offset, daylight saving, and transition interval details for a given timezone at a specific timestamp.

| Member | Type | Description |
| :--- | :--- | :--- |
| `begin` | `tx::Second` | Unix timestamp marking the start of the current transition interval (inclusive). |
| `end` | `tx::Second` | Unix timestamp marking the end of the transition interval (exclusive). |
| `offset` | `tx::Offset` | Total UTC offset in seconds (including DST adjustment if active). |
| `save` | `tx::Offset` | Daylight Saving Time adjustment in seconds (`0` if standard time). |
| `abbrev` | `std::string` | Short display name / abbreviation of the timezone (e.g., `"PST"`, `"PDT"`). |
| `is_dst` | `bool` | `true` if Daylight Saving Time is in effect; `false` otherwise. |

---

### Timezone Functions

| Signature | Return Type | Description |
| :--- | :--- | :--- |
| `current_zone()` | `std::string` | Returns the canonical IANA identifier of the currently selected or detected system timezone. |
| `available_zones()` | `std::vector<std::string>` | Enumerates and returns all available canonical system timezones sorted alphabetically. |
| `select_zone(const std::string& zone)` | `void` | Validates and sets the global active timezone. Accepts IANA (e.g., `"America/New_York"`) and Windows IDs (e.g., `"Eastern Standard Time"`). Throws `std::invalid_argument` if unknown. |
| `zone_info()` | `ZoneInfo` | Queries and returns `ZoneInfo` for the current system clock time in the active timezone. |
| `zone_info(const std::string& zone, tx::Second ts)` | `ZoneInfo` | Queries and returns `ZoneInfo` for a specific Unix timestamp (`ts`) in the specified `zone`. |

---

## Example Usage

```cpp
#include <tx/tx.hpp>
#include <tx/zones.hpp>
#include <iostream>

int main() {
    using namespace tx;

    try {
        // Get current system or selected zone
        std::string zone = zone::current_zone();
        std::cout << "Current Zone: " << zone << "\n";

        // Query zone info for now
        zone::ZoneInfo info = zone::zone_info();
        std::cout << "UTC Offset: " << (info.offset / 3600) << " hours\n";
        std::cout << "Abbreviation: " << info.abbrev << "\n";
        std::cout << "Is DST Active: " << (info.is_dst ? "Yes" : "No") << "\n";

        // Select a different timezone
        zone::select_zone("Europe/London");
        std::cout << "Switched to: " << zone::current_zone() << "\n";

        // Query historical or future timestamp in a specific zone
        Second timestamp = 1'700'000'000ULL; // e.g. Nov 2023
        zone::ZoneInfo hist = zone::zone_info("Asia/Tokyo", timestamp);
        std::cout << "Tokyo Offset at timestamp: " << (hist.offset / 3600) << "h\n";

    } catch (const std::exception& e) {
        std::cerr << "Timezone error: " << e.what() << "\n";
    }

    return 0;
}
```
