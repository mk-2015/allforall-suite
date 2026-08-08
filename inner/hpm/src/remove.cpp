#include <pkg_db.hpp>
#include <pkg.hpp>
#include <cstdio>
#include <string>
#include <vector>

int hpm_uninstall_package(const std::vector<std::string>& pkglist, bool force) {
    if (pkglist.empty()) {
        hpm_info("No packages specified for removal.\n");
        return 0;
    }

    int fail_count = 0;

    for (const auto& pkg_name : pkglist) {
        printf("===> Processing removal for: %s\n", pkg_name.c_str());

        if (!db_is_package_installed(pkg_name)) {
            hpm_fail((int)NULL, "Package '%s' is not installed in the database.\n", pkg_name.c_str());
            fail_count++;
            continue;
        }

        std::vector<std::string> dependents = db_get_dependents(pkg_name);

        if (!dependents.empty() && !force) {
            hpm_fail((int)NULL, "Cannot remove '%s'. It is required by:\n", pkg_name.c_str());
            for (const auto& dep : dependents) {
                printf("       - %s\n", dep.c_str());
            }
            printf("     Use '--force' or '-f' to override and remove anyway.\n");
            fail_count++;
            continue;
        }

        if (!db_remove_package(pkg_name)) {
            hpm_fail((int)NULL, "Failed to completely uninstall package '%s'.\n", pkg_name.c_str());
            fail_count++;
        }
    }

    return (fail_count == 0) ? 0 : 1;
}