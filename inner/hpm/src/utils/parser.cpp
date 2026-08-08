#include <pkg.hpp>
#include <yaml.h>
#include <iostream>
#include <string>
#include <vector>

bool parse_box_yaml(const std::string& filepath, BoxConfig& out_config) {
    FILE* fh = fopen(filepath.c_str(), "rb");
    if (!fh) {
        hpm_fail((int)NULL, "Failed to open file: %s\n", filepath.c_str());
        return false;
    }

    yaml_parser_t parser;
    yaml_token_t token;

    if (!yaml_parser_initialize(&parser)) {
        hpm_fail((int)NULL, "Failed to initialize YAML parser.\n");
        fclose(fh);
        return false;
    }

    yaml_parser_set_input_file(&parser, fh);

    std::string last_key;
    bool in_commands = false;

    while (true) {
        if (!yaml_parser_scan(&parser, &token)) {
            hpm_fail((int)NULL, "YAML Syntax error in file: %s\n", filepath.c_str());
            break;
        }

        if (token.type == YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
            break;
        }

        if (token.type == YAML_KEY_TOKEN) {
            in_commands = false;
        } else if (token.type == YAML_VALUE_TOKEN) {
            // Next scalar belongs to last_key
        } else if (token.type == YAML_SCALAR_TOKEN) {
            std::string value = reinterpret_cast<char*>(token.data.scalar.value);

            if (last_key == "box") {
                out_config.name = value;
                last_key.clear();
            } else if (last_key == "version") {
                out_config.version = value;
                last_key.clear();
            } else if (last_key == "maintainer") {
                out_config.maintainer = value;
                last_key.clear();
            } else if (last_key == "commands") {
                out_config.commands.push_back(value);
            } else if (last_key == "dependencies") {
                std::string dep_str = reinterpret_cast<char*>(token.data.scalar.value);
                
                size_t space_pos = dep_str.find(' ');
                if (space_pos != std::string::npos) {
                    out_config.dependencies.push_back({
                        dep_str.substr(0, space_pos),
                        dep_str.substr(space_pos + 1)
                    });
                } else {
                    out_config.dependencies.push_back({dep_str, ""});
                }
            } else {
                last_key = value;
            }
        }

        yaml_token_delete(&token);
    }

    yaml_parser_delete(&parser);
    fclose(fh);
    return true;
}

std::vector<std::string> parse_hpm_config_repos(const std::string& filepath) {
    std::vector<std::string> repos;

    FILE* fh = fopen(filepath.c_str(), "rb");
    if (!fh) {
        hpm_fail((int)NULL, "Config file missing at %s\n", filepath.c_str());
        return repos;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    if (!yaml_parser_initialize(&parser)) {
        fclose(fh);
        return repos;
    }

    yaml_parser_set_input_file(&parser, fh);

    std::string current_key;
    bool in_repos_sequence = false;

    while (true) {
        if (!yaml_parser_parse(&parser, &event)) break;

        if (event.type == YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        switch (event.type) {
            case YAML_SEQUENCE_START_EVENT:
                if (current_key == "repos") {
                    in_repos_sequence = true;
                }
                break;

            case YAML_SEQUENCE_END_EVENT:
                if (in_repos_sequence) {
                    in_repos_sequence = false;
                }
                break;

            case YAML_SCALAR_EVENT: {
                std::string val = reinterpret_cast<char*>(event.data.scalar.value);

                if (in_repos_sequence) {
                    if (!val.empty() && val.back() == '/') {
                        val.pop_back();
                    }
                    repos.push_back(val);
                } else {
                    current_key = val;
                }
                break;
            }
            default:
                break;
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(fh);
    return repos;
}