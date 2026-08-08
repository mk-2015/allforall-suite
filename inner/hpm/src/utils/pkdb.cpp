#include <pkg_db.hpp>
#include <pkg.hpp>
#include <sqlite3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>

namespace fs = std::filesystem;

static sqlite3* db = nullptr;

static fs::path get_db_path() {
    const char* home = std::getenv("HOME");
    if (!home) home = std::getenv("USERPROFILE");
    if (!home) return "hpm.sqlite3";

    fs::path hpm_dir = fs::path(home) / ".hpm";
    fs::create_directories(hpm_dir);
    return hpm_dir / "db.sqlite3";
}

bool db_init() {
    if (db) return true;

    std::string db_path = get_db_path().string();
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        hpm_fail((int)NULL, "Failed to open SQLite database at %s: %s\n", db_path.c_str(), sqlite3_errmsg(db));
        return false;
    }

    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    const char* sql_init = 
        "CREATE TABLE IF NOT EXISTS installed_packages ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT UNIQUE NOT NULL,"
        "  version TEXT NOT NULL,"
        "  installed_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");"
        "CREATE TABLE IF NOT EXISTS package_dependencies ("
        "    parent_id INTEGER NOT NULL,"
        "    child_name TEXT NOT NULL,"
        "    FOREIGN KEY (parent_id) REFERENCES installed_packages(id) ON DELETE CASCADE"
        ");"
        "CREATE TABLE IF NOT EXISTS package_files ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  package_id INTEGER NOT NULL,"
        "  file_path TEXT NOT NULL,"
        "  FOREIGN KEY (package_id) REFERENCES installed_packages(id) ON DELETE CASCADE"
        ");";

    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql_init, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        hpm_fail((int)NULL, "Failed to initialize database tables: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

std::vector<std::string> db_get_dependents(const std::string& pkg_name) {
    std::vector<std::string> dependents;
    if (!db && !db_init()) return dependents;

    const char* sql = 
        "SELECT p.name FROM installed_packages p "
        "JOIN package_dependencies d ON p.id = d.parent_id "
        "WHERE d.child_name = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, pkg_name.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* name = sqlite3_column_text(stmt, 0);
            if (name) dependents.push_back(reinterpret_cast<const char*>(name));
        }
        sqlite3_finalize(stmt);
    }
    return dependents;
}

bool db_is_package_installed(const std::string& pkg_name) {
    if (!db && !db_init()) return false;

    const char* sql = "SELECT id FROM installed_packages WHERE name = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, pkg_name.c_str(), -1, SQLITE_STATIC);
    bool installed = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return installed;
}

bool db_register_package(const std::string& pkg_name, const std::string& version, const std::vector<std::string>& installed_files) {
    if (!db && !db_init()) return false;

    // Clean up previous registration if reinstalling
    db_remove_package(pkg_name);

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* sql_pkg = "INSERT INTO installed_packages (name, version) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql_pkg, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    sqlite3_bind_text(stmt, 1, pkg_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, version.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(stmt);

    int64_t pkg_id = sqlite3_last_insert_rowid(db);

    const char* sql_file = "INSERT INTO package_files (package_id, file_path) VALUES (?, ?);";
    for (const auto& file : installed_files) {
        if (sqlite3_prepare_v2(db, sql_file, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, pkg_id);
            sqlite3_bind_text(stmt, 2, file.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

bool db_remove_package(const std::string& pkg_name) {
    if (!db && !db_init()) return false;

    // 1. Fetch file paths stored for this package
    const char* sql_files = 
        "SELECT f.file_path FROM package_files f "
        "JOIN installed_packages p ON f.package_id = p.id "
        "WHERE p.name = ?;";

    sqlite3_stmt* stmt = nullptr;
    std::vector<std::string> files_to_delete;

    if (sqlite3_prepare_v2(db, sql_files, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, pkg_name.c_str(), -1, SQLITE_STATIC);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* text = sqlite3_column_text(stmt, 0);
            if (text) files_to_delete.push_back(reinterpret_cast<const char*>(text));
        }
        sqlite3_finalize(stmt);
    }

    // 2. Delete files from system disk
    size_t removed_count = 0;
    for (const auto& file_path : files_to_delete) {
        std::error_code ec;
        if (fs::exists(file_path)) {
            if (fs::remove(file_path, ec)) {
                printf("  [DELETED] %s\n", file_path.c_str());
                removed_count++;
            } else {
                printf("  [FAILED]  %s (%s)\n", file_path.c_str(), ec.message().c_str());
            }
        }
    }

    // 3. Remove entry from DB (CASCADE deletes file records in package_files)
    const char* sql_del = "DELETE FROM installed_packages WHERE name = ?;";
    if (sqlite3_prepare_v2(db, sql_del, -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, pkg_name.c_str(), -1, SQLITE_STATIC);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    if (success) {
        printf("===> Successfully uninstalled %s (%zu files removed).\n", pkg_name.c_str(), removed_count);
    }

    return success;
}

std::vector<std::string> db_list_installed_packages() {
    std::vector<std::string> result;
    if (!db && !db_init()) return result;

    const char* sql = "SELECT name, version FROM installed_packages ORDER BY name ASC;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* name = sqlite3_column_text(stmt, 0);
            const unsigned char* ver = sqlite3_column_text(stmt, 1);
            if (name && ver) {
                result.push_back(std::string(reinterpret_cast<const char*>(name)) + " v" + reinterpret_cast<const char*>(ver));
            }
        }
        sqlite3_finalize(stmt);
    }

    return result;
}