
#pragma once

#include <tx/tx.hpp>

#include <string>
#include <vector>

namespace tx::zone
{

struct ZoneInfo
{
    tx::Second begin = 0;
    tx::Second end = 0;       // Exclusive
    tx::Offset offset = 0;    // Total UTC offset, seconds
    tx::Offset save = 0;      // DST adjustment, seconds
    std::string abbrev;
    bool is_dst = false;
};

std::string current_zone();

std::vector<std::string> available_zones();

void select_zone(const std::string& zone);

ZoneInfo zone_info();

ZoneInfo zone_info(
    const std::string& zone,
    tx::Second timestamp
);

}
