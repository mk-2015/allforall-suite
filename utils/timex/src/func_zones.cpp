
#include <tx/zones.hpp>

#include <unicode/ucal.h>
#include <unicode/uenum.h>
#include <unicode/ustring.h>
#include <unicode/utypes.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace tx::zone
{
namespace
{

std::mutex selection_mutex;
std::string selected_zone;

constexpr std::uint64_t MAX_ICU_SECONDS =
    8640000000000ULL;

constexpr int32_t MAX_ZONE_ID_CHARS = 1024;


// ============================================================
// ICU resource management
// ============================================================

class Calendar final
{
public:
    explicit Calendar(UCalendar* value) noexcept
        : value_(value)
    {
    }

    ~Calendar()
    {
        if (value_)
            ucal_close(value_);
    }

    Calendar(const Calendar&) = delete;
    Calendar& operator=(const Calendar&) = delete;

    Calendar(Calendar&& other) noexcept
        : value_(other.value_)
    {
        other.value_ = nullptr;
    }

    Calendar& operator=(Calendar&& other) noexcept
    {
        if (this != &other)
        {
            if (value_)
                ucal_close(value_);

            value_ = other.value_;
            other.value_ = nullptr;
        }

        return *this;
    }

    UCalendar* get() const noexcept
    {
        return value_;
    }

private:
    UCalendar* value_;
};


class Enumeration final
{
public:
    explicit Enumeration(UEnumeration* value) noexcept
        : value_(value)
    {
    }

    ~Enumeration()
    {
        if (value_)
            uenum_close(value_);
    }

    Enumeration(const Enumeration&) = delete;
    Enumeration& operator=(const Enumeration&) = delete;

    UEnumeration* get() const noexcept
    {
        return value_;
    }

private:
    UEnumeration* value_;
};


// ============================================================
// Unicode conversion
// ============================================================

std::vector<UChar> to_uchar(const std::string& input)
{
    if (input.empty())
        throw std::invalid_argument(
            "Timezone identifier cannot be empty"
        );

    if (input.size() >
        static_cast<std::size_t>(
            std::numeric_limits<int32_t>::max()
        ))
    {
        throw std::invalid_argument(
            "Timezone identifier is too long"
        );
    }

    UErrorCode status = U_ZERO_ERROR;

    int32_t needed = 0;

    u_strFromUTF8(
        nullptr,
        0,
        &needed,
        input.data(),
        static_cast<int32_t>(input.size()),
        &status
    );

    if (status != U_BUFFER_OVERFLOW_ERROR &&
        U_FAILURE(status))
    {
        throw std::invalid_argument(
            "Timezone identifier is not valid UTF-8"
        );
    }

    if (needed < 0 || needed > MAX_ZONE_ID_CHARS)
        throw std::invalid_argument(
            "Timezone identifier is too long"
        );

    std::vector<UChar> output(
        static_cast<std::size_t>(needed) + 1
    );

    status = U_ZERO_ERROR;

    int32_t written = 0;

    u_strFromUTF8(
        output.data(),
        static_cast<int32_t>(output.size()),
        &written,
        input.data(),
        static_cast<int32_t>(input.size()),
        &status
    );

    if (U_FAILURE(status))
        throw std::invalid_argument(
            "Timezone identifier is not valid UTF-8"
        );

    output.resize(static_cast<std::size_t>(written) + 1);
    output[static_cast<std::size_t>(written)] = 0;

    return output;
}


std::string to_utf8(const UChar* input, int32_t length)
{
    if (!input || length < 0)
        throw std::runtime_error(
            "ICU returned invalid Unicode data"
        );

    UErrorCode status = U_ZERO_ERROR;

    int32_t needed = 0;

    u_strToUTF8(
        nullptr,
        0,
        &needed,
        input,
        length,
        &status
    );

    if (status != U_BUFFER_OVERFLOW_ERROR &&
        U_FAILURE(status))
    {
        throw std::runtime_error(
            "Could not convert ICU text to UTF-8"
        );
    }

    std::string output(
        static_cast<std::size_t>(needed),
        '\0'
    );

    status = U_ZERO_ERROR;

    int32_t written = 0;

    u_strToUTF8(
        output.data(),
        needed,
        &written,
        input,
        length,
        &status
    );

    if (U_FAILURE(status))
        throw std::runtime_error(
            "Could not convert ICU text to UTF-8"
        );

    output.resize(static_cast<std::size_t>(written));

    return output;
}


// ============================================================
// Timezone identifier handling
// ============================================================

std::string canonical_system_id(
    const std::vector<UChar>& id
)
{
    std::vector<UChar> canonical(
        MAX_ZONE_ID_CHARS + 1
    );

    UBool is_system_id = false;

    UErrorCode status = U_ZERO_ERROR;

    const int32_t length =
        ucal_getCanonicalTimeZoneID(
            id.data(),
            -1,
            canonical.data(),
            static_cast<int32_t>(canonical.size()),
            &is_system_id,
            &status
        );

    if (U_FAILURE(status) ||
        !is_system_id ||
        length <= 0 ||
        length >= static_cast<int32_t>(canonical.size()))
    {
        return {};
    }

    return to_utf8(canonical.data(), length);
}


std::string normalize_zone(const std::string& requested)
{
    const auto id = to_uchar(requested);

    // First, try an IANA timezone identifier.
    if (const auto canonical = canonical_system_id(id);
        !canonical.empty())
    {
        return canonical;
    }

    // Windows uses identifiers such as:
    // Pacific Standard Time
    //
    // ICU's CLDR mapping converts Windows IDs to IANA IDs.

    std::vector<UChar> iana(
        MAX_ZONE_ID_CHARS + 1
    );

    UErrorCode status = U_ZERO_ERROR;

    const int32_t length =
        ucal_getTimeZoneIDForWindowsID(
            id.data(),
            -1,
            nullptr,
            iana.data(),
            static_cast<int32_t>(iana.size()),
            &status
        );

    if (U_FAILURE(status) ||
        length <= 0 ||
        length >= static_cast<int32_t>(iana.size()))
    {
        throw std::invalid_argument(
            "Unknown timezone identifier: " + requested
        );
    }

    iana[static_cast<std::size_t>(length)] = 0;

    const std::string canonical =
        canonical_system_id(iana);

    if (canonical.empty())
        throw std::invalid_argument(
            "Timezone cannot be mapped to an IANA identifier: " +
            requested
        );

    return canonical;
}


// ============================================================
// Calendar creation
// ============================================================

Calendar open_calendar(const std::string& canonical_id)
{
    const auto id = to_uchar(canonical_id);

    UErrorCode status = U_ZERO_ERROR;

    UCalendar* calendar = ucal_open(
        id.data(),
        -1,
        "en_US_POSIX",
        UCAL_GREGORIAN,
        &status
    );

    if (U_FAILURE(status) || !calendar)
        throw std::runtime_error(
            "ICU could not open timezone: " + canonical_id
        );

    return Calendar(calendar);
}


// ============================================================
// System timezone detection
// ============================================================

std::string get_default_zone()
{
    std::vector<UChar> id(
        MAX_ZONE_ID_CHARS + 1
    );

    UErrorCode status = U_ZERO_ERROR;

    const int32_t length =
        ucal_getDefaultTimeZone(
            id.data(),
            static_cast<int32_t>(id.size()),
            &status
        );

    if (U_FAILURE(status) ||
        length <= 0 ||
        length >= static_cast<int32_t>(id.size()))
    {
        throw std::runtime_error(
            "ICU could not detect the system timezone"
        );
    }

    id[static_cast<std::size_t>(length)] = 0;

    const std::string raw = to_utf8(id.data(), length);

    try
    {
        return normalize_zone(raw);
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error(
            "System timezone is not recognized by ICU: " + raw
        );
    }
}


// ============================================================
// Transition conversion
// ============================================================

std::uint64_t transition_seconds(UDate millis)
{
    if (!std::isfinite(millis) || millis < 0.0)
        return 0;

    const long double seconds =
        std::floor(
            static_cast<long double>(millis) / 1000.0L
        );

    if (seconds >=
        static_cast<long double>(
            std::numeric_limits<std::uint64_t>::max()
        ))
    {
        return std::numeric_limits<std::uint64_t>::max();
    }

    return static_cast<std::uint64_t>(seconds);
}


// ============================================================
// Timezone abbreviation
// ============================================================

std::string timezone_short_name(
    const UCalendar* calendar,
    bool is_dst
)
{
    std::vector<UChar> name(256);

    UErrorCode status = U_ZERO_ERROR;

    const auto type = is_dst
        ? UCAL_SHORT_DST
        : UCAL_SHORT_STANDARD;

    int32_t length = ucal_getTimeZoneDisplayName(
        calendar,
        type,
        "en_US_POSIX",
        name.data(),
        static_cast<int32_t>(name.size()),
        &status
    );

    if (U_FAILURE(status))
        return {};

    if (length <= 0)
        return {};

    if (length >= static_cast<int32_t>(name.size()))
    {
        name.resize(
            static_cast<std::size_t>(length) + 1
        );

        status = U_ZERO_ERROR;

        length = ucal_getTimeZoneDisplayName(
            calendar,
            type,
            "en_US_POSIX",
            name.data(),
            static_cast<int32_t>(name.size()),
            &status
        );

        if (U_FAILURE(status) ||
            length <= 0 ||
            length >= static_cast<int32_t>(name.size()))
        {
            return {};
        }
    }

    return to_utf8(name.data(), length);
}


// ============================================================
// Timezone information
// ============================================================

ZoneInfo query_zone(
    const std::string& canonical_id,
    tx::Second timestamp
)
{
    if (timestamp > MAX_ICU_SECONDS)
        throw std::out_of_range(
            "Timestamp exceeds ICU's supported timezone range"
        );

    Calendar calendar = open_calendar(canonical_id);

    const UDate instant =
        static_cast<UDate>(timestamp) * 1000.0;

    UErrorCode status = U_ZERO_ERROR;

    ucal_setMillis(
        calendar.get(),
        instant,
        &status
    );

    if (U_FAILURE(status))
        throw std::runtime_error(
            "ICU could not set timezone query timestamp"
        );

    // Standard UTC offset.

    status = U_ZERO_ERROR;

    const int32_t raw_ms = ucal_get(
        calendar.get(),
        UCAL_ZONE_OFFSET,
        &status
    );

    if (U_FAILURE(status))
        throw std::runtime_error(
            "Could not read timezone UTC offset"
        );

    // Daylight saving adjustment.

    status = U_ZERO_ERROR;

    const int32_t dst_ms = ucal_get(
        calendar.get(),
        UCAL_DST_OFFSET,
        &status
    );

    if (U_FAILURE(status))
        throw std::runtime_error(
            "Could not read timezone DST offset"
        );

    const std::int64_t total_ms =
        static_cast<std::int64_t>(raw_ms) + dst_ms;

    if (raw_ms % 1000 != 0 ||
        dst_ms % 1000 != 0)
    {
        throw std::runtime_error(
            "ICU returned a sub-second timezone offset"
        );
    }

    ZoneInfo info{};

    info.offset = static_cast<tx::Offset>(
        total_ms / 1000
    );

    info.save = static_cast<tx::Offset>(
        dst_ms / 1000
    );

    info.is_dst = dst_ms != 0;

    info.abbrev = timezone_short_name(
        calendar.get(),
        info.is_dst
    );

    // Previous transition, inclusive.
    //
    // begin <= timestamp

    UDate transition = 0;

    status = U_ZERO_ERROR;

    const UBool has_previous =
        ucal_getTimeZoneTransitionDate(
            calendar.get(),
            UCAL_TZ_TRANSITION_PREVIOUS_INCLUSIVE,
            &transition,
            &status
        );

    if (U_FAILURE(status))
        throw std::runtime_error(
            "Could not find previous timezone transition"
        );

    if (has_previous)
        info.begin = transition_seconds(transition);
    else
        info.begin = 0;

    // Next transition, exclusive.
    //
    // timestamp < end

    transition = 0;

    status = U_ZERO_ERROR;

    const UBool has_next =
        ucal_getTimeZoneTransitionDate(
            calendar.get(),
            UCAL_TZ_TRANSITION_NEXT,
            &transition,
            &status
        );

    if (U_FAILURE(status))
        throw std::runtime_error(
            "Could not find next timezone transition"
        );

    if (has_next)
    {
        info.end = transition_seconds(transition);
    }
    else
    {
        info.end =
            std::numeric_limits<tx::Second>::max();
    }

    if (info.begin > timestamp)
        throw std::runtime_error(
            "Invalid timezone transition interval"
        );

    if (info.end !=
            std::numeric_limits<tx::Second>::max() &&
        info.end <= timestamp)
    {
        throw std::runtime_error(
            "Invalid timezone transition interval"
        );
    }

    return info;
}

} // namespace


// ============================================================
// Public API
// ============================================================

std::string current_zone()
{
    std::lock_guard<std::mutex> lock(selection_mutex);

    if (selected_zone.empty())
        selected_zone = get_default_zone();

    return selected_zone;
}


std::vector<std::string> available_zones()
{
    UErrorCode status = U_ZERO_ERROR;

    Enumeration zones(
        ucal_openTimeZoneIDEnumeration(
            UCAL_ZONE_TYPE_CANONICAL,
            nullptr,
            nullptr,
            &status
        )
    );

    if (U_FAILURE(status) || !zones.get())
        throw std::runtime_error(
            "ICU could not enumerate timezone identifiers"
        );

    std::vector<std::string> result;

    for (;;)
    {
        int32_t length = 0;

        status = U_ZERO_ERROR;

        const UChar* id = uenum_unext(
            zones.get(),
            &length,
            &status
        );

        if (U_FAILURE(status))
            throw std::runtime_error(
                "ICU timezone enumeration failed"
            );

        if (!id)
            break;

        if (length <= 0)
            continue;

        result.push_back(
            to_utf8(id, length)
        );
    }

    std::sort(result.begin(), result.end());

    result.erase(
        std::unique(result.begin(), result.end()),
        result.end()
    );

    return result;
}


void select_zone(const std::string& zone)
{
    const std::string canonical =
        normalize_zone(zone);

    // Validate before modifying the selected timezone.
    (void)open_calendar(canonical);

    std::lock_guard<std::mutex> lock(selection_mutex);

    selected_zone = canonical;
}


ZoneInfo zone_info()
{
    const auto now =
        std::chrono::system_clock::now()
            .time_since_epoch();

    const auto seconds =
        std::chrono::duration_cast<
            std::chrono::seconds
        >(now).count();

    if (seconds < 0)
        throw std::runtime_error(
            "System clock is before the Unix epoch"
        );

    return zone_info(
        current_zone(),
        static_cast<tx::Second>(seconds)
    );
}


ZoneInfo zone_info(
    const std::string& zone,
    tx::Second timestamp
)
{
    return query_zone(
        normalize_zone(zone),
        timestamp
    );
}

} // namespace tx::zone
