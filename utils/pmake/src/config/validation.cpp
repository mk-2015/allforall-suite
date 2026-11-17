#include <config/validation.hpp>
#include <stdexcept>
#include <toml++/toml.hpp>
#include <regex>

namespace pmake::config {

void validate_config(const ProjectConfig& config, const std::string& filename)
{
    // Validate package table
    if (config.package.name.empty()) {
        throw std::runtime_error("[ERROR] " + filename + ": package: missing or empty required field 'name'");
    }
    if (config.package.version.empty()) {
        throw std::runtime_error("[ERROR] " + filename + ": package: missing or empty required field 'version'");
    }
    if (config.package.release <= 0) {
        throw std::runtime_error("[ERROR] " + filename + ": package: invalid release number (must be > 0)");
    }

    // Validate build table
    if (config.build.jobs <= 0) {
        throw std::runtime_error("[ERROR] " + filename + ": build: invalid jobs count (must be > 0)");
    }

    // Validate sources
    if (config.sources.empty()) {
        throw std::runtime_error("[ERROR] " + filename + ": build: at least one source table is required");
    }

    // Re-parse raw TOML to strictly check for missing 'main' field on each source
    toml::table tbl;
    try {
        tbl = toml::parse_file(filename);
    } catch (...) {
        // If already parsed successfully in parser, this should succeed
    }

    int main_count = 0;

    if (auto bld_tbl = tbl["build"].as_table()) {
        bld_tbl->for_each([&](auto&& key, auto&& val) {
            std::string k{key};
            std::regex source_regex(R"(^source(\d+)$)");
            if (std::regex_match(k, source_regex) && val.is_table()) {
                auto src_tbl = val.as_table();
                std::string src_id = k;

                if (!src_tbl->contains("main")) {
                    throw std::runtime_error("[ERROR] " + filename + ": build." + src_id + ": missing required field 'main'");
                }
                if (!src_tbl->contains("name")) {
                    throw std::runtime_error("[ERROR] " + filename + ": build." + src_id + ": missing required field 'name'");
                }
                if (!src_tbl->contains("version")) {
                    throw std::runtime_error("[ERROR] " + filename + ": build." + src_id + ": missing required field 'version'");
                }
                if (!src_tbl->contains("source")) {
                    throw std::runtime_error("[ERROR] " + filename + ": build." + src_id + ": missing required field 'source'");
                }

                auto m_node = src_tbl->get("main");
                if (!m_node->is_boolean()) {
                    throw std::runtime_error("[ERROR] " + filename + ": build." + src_id + ": field 'main' must be a boolean");
                }
                if (m_node->as_boolean()->get()) {
                    main_count++;
                }
            }
        });
    }

    if (main_count == 0) {
        throw std::runtime_error("[ERROR] " + filename + ": configuration error: zero sources have 'main = true' (exactly one main source is required)");
    }
    if (main_count > 1) {
        throw std::runtime_error("[ERROR] " + filename + ": configuration error: multiple sources have 'main = true' (exactly one main source is required)");
    }

    // Validate individual source properties
    for (const auto& src : config.sources) {
        if (src.name.empty()) {
            throw std::runtime_error("[ERROR] " + filename + ": build." + src.id + ": source name cannot be empty");
        }
        if (src.version.empty()) {
            throw std::runtime_error("[ERROR] " + filename + ": build." + src.id + ": source version cannot be empty");
        }
        if (src.source.empty()) {
            throw std::runtime_error("[ERROR] " + filename + ": build." + src.id + ": source URL/reference cannot be empty");
        }

        // Validate build system
        std::string bs = src.build_system;
        if (bs != "make" && bs != "cmake" && bs != "meson" && bs != "autotools" && bs != "custom") {
            throw std::runtime_error("[ERROR] " + filename + ": build." + src.id + ": unsupported build_system '" + bs + "'");
        }

        if (bs == "custom" && (!src.custom_build || src.custom_build->empty())) {
            throw std::runtime_error("[ERROR] " + filename + ": build." + src.id + ": build_system 'custom' requires 'custom_build' command");
        }
    }
}

} // namespace pmake::config
