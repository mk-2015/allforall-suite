#include <config/parser.hpp>
#include <stdexcept>
#include <iostream>
#include <regex>
#include <algorithm>

namespace pmake::config {

ProjectConfig parse_config(const std::string& path)
{
    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error& err) {
        throw std::runtime_error("Failed to parse TOML config file '" + path + "': " + std::string(err.description()));
    }

    ProjectConfig config;

    // Parse [package]
    if (auto pkg_tbl = tbl["package"].as_table()) {
        if (auto name = pkg_tbl->get("name")) config.package.name = name->value_or("");
        if (auto ver = pkg_tbl->get("version")) config.package.version = ver->value_or("");
        if (auto rel = pkg_tbl->get("release")) config.package.release = rel->value_or(1);
        if (auto desc = pkg_tbl->get("description")) config.package.description = desc->value_or("");
        if (auto arch = pkg_tbl->get("architecture")) config.package.architecture = arch->value_or("x86_64");

        if (auto lic = pkg_tbl->get("license")) {
            if (lic->is_array()) {
                lic->as_array()->for_each([&](auto&& elem) {
                    if (elem.is_string()) config.package.license.push_back(elem.template value_or(""));
                });
            } else if (lic->is_string()) {
                config.package.license.push_back(lic->template value_or(""));
            }
        }
    }

    // Parse [build]
    if (auto bld_tbl = tbl["build"].as_table()) {
        if (auto jobs = bld_tbl->get("jobs")) config.build.jobs = jobs->value_or(4);
        if (auto dir = bld_tbl->get("directory")) config.build.directory = dir->value_or("build");
        if (auto out = bld_tbl->get("output")) config.build.output = out->value_or("pkg");

        // Parse dynamic source tables [build.source1], [build.source2], etc.
        // We collect all keys matching sourceN and sort them numerically by N
        std::vector<std::pair<int, toml::table*>> source_tables;

        bld_tbl->for_each([&](auto&& key, auto&& val) {
            std::string k{key};
            std::regex source_regex(R"(^source(\d+)$)");
            std::smatch match;
            if (std::regex_match(k, match, source_regex)) {
                if (val.is_table()) {
                    int index = std::stoi(match[1].str());
                    source_tables.push_back({index, val.as_table()});
                }
            }
        });

        // Sort by numeric suffix deterministically
        std::sort(source_tables.begin(), source_tables.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        for (auto& [index, src_tbl] : source_tables) {
            SourceConfig src;
            src.id = "source" + std::to_string(index);

            if (auto name = src_tbl->get("name")) src.name = name->value_or("");
            if (auto ver = src_tbl->get("version")) src.version = ver->value_or("");
            if (auto s = src_tbl->get("source")) src.source = s->value_or("");
            if (auto m = src_tbl->get("main")) {
                if (m->is_boolean()) {
                    src.main = m->as_boolean()->get();
                }
            }
            if (auto bs = src_tbl->get("build_system")) src.build_system = bs->value_or("make");
            if (auto cb = src_tbl->get("custom_build")) src.custom_build = cb->value_or("");
            if (auto ci = src_tbl->get("custom_install")) src.custom_install = ci->value_or("");

            config.sources.push_back(src);
        }
    }

    return config;
}

} // namespace pmake::config
