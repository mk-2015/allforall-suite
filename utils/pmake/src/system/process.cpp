#include <system/process.hpp>
#include <log.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

extern "C" char **environ;

namespace pmake::system {

int run_process(const std::string& command, const std::vector<std::string>& args, const std::string& working_dir)
{
    // Build argv array
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(command.c_str()));
    for (const auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid;
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);

    if (!working_dir.empty()) {
        posix_spawn_file_actions_addchdir_np(&actions, working_dir.c_str());
    }

    int status = posix_spawnp(&pid, command.c_str(), &actions, nullptr, argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);

    if (status != 0) {
        pmake::log::slogf(pmake::log::ERROR, "Failed to spawn process '{}': {}", command, std::strerror(status));
        return -1;
    }

    int child_status = 0;
    while (waitpid(pid, &child_status, 0) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }

    if (WIFEXITED(child_status)) {
        return WEXITSTATUS(child_status);
    } else if (WIFSIGNALED(child_status)) {
        pmake::log::slogf(pmake::log::ERROR, "Process '{}' terminated by signal {}", command, WTERMSIG(child_status));
        return -1;
    }

    return -1;
}

} // namespace pmake::system
