#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <chrono>
#include <thread>
#include <limits>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

using Clock = std::chrono::steady_clock;

static constexpr int EXIT_USAGE = 2;
static constexpr int EXIT_SYSTEM = 3;
static constexpr int EXIT_TIMEOUT = 124;
static constexpr double DEFAULT_MAX_RUNTIME = -1.0;
static constexpr double MAX_DELAY_SECONDS = 86400.0;
static constexpr double MAX_RUNTIME_SECONDS = 31536000.0;

void print_help() {
    std::cout
        << "retry [OPTIONS] <command> [ARGS...]\n\n"
        << "Options:\n"
        << "    -h, --help  Display help\n"
        << "    -n NUM      Maximum total attempts (default: 3)\n"
        << "    -d SECONDS  Delay between attempts (default: 1)\n"
        << "    -m SECONDS  Maximum total runtime (default: disabled)\n"
        << "    -q          Suppress retry messages\n"
        << "    -v          Verbose attempt logging\n"
        << "    --          End option parsing\n\n"
        << "Exit status:\n"
        << "    Command's exit status after the final attempt\n"
        << "    2   Invalid arguments\n"
        << "    3   Internal process/wait error\n"
        << "    124 Runtime limit exceeded\n";
}

bool parse_integer(const char* input, int& value) {
    if (input == nullptr || *input == '\0') {
        return false;
    }

    try {
        std::string s(input);
        std::size_t pos = 0;
        long parsed = std::stol(s, &pos, 10);

        if (pos != s.size() ||
            parsed < 1 ||
            parsed > std::numeric_limits<int>::max()) {
            return false;
        }

        value = static_cast<int>(parsed);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool parse_double(const char* input, double& value) {
    if (input == nullptr || *input == '\0') {
        return false;
    }

    try {
        std::string s(input);
        std::size_t pos = 0;

        double parsed = std::stod(s, &pos);

        if (pos != s.size() || !std::isfinite(parsed)) {
            return false;
        }

        value = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool wait_for_child(pid_t pid, int& status) {
    while (true) {
        pid_t result = waitpid(pid, &status, 0);

        if (result == pid) {
            return true;
        }

        if (result == -1 && errno == EINTR) {
            continue;
        }

        return false;
    }
}

bool sleep_until_or_for(
    Clock::time_point duration_end,
    const Clock::time_point* deadline
) {
    while (Clock::now() < duration_end) {
        auto now = Clock::now();

        if (deadline != nullptr && now >= *deadline) {
            return false;
        }

        auto wake_at = duration_end;

        if (deadline != nullptr && *deadline < wake_at) {
            wake_at = *deadline;
        }

        std::this_thread::sleep_until(wake_at);
    }

    return true;
}

int terminate_process_group(pid_t pid, int& status) {
    // The child is placed in its own process group before exec.
    // Signal the group so child processes are included as well.
    if (kill(-pid, SIGTERM) == -1 && errno != ESRCH) {
        // Fall back to signalling the direct child.
        kill(pid, SIGTERM);
    }

    const auto grace_end =
        Clock::now() + std::chrono::milliseconds(500);

    while (Clock::now() < grace_end) {
        pid_t result = waitpid(pid, &status, WNOHANG);

        if (result == pid) {
            // The direct child exited. Ensure remaining group members
            // are not left running after the timeout.
            if (kill(-pid, SIGKILL) == -1 && errno != ESRCH) {
                // Nothing else to do if the group no longer exists.
            }
            return EXIT_TIMEOUT;
        }

        if (result == -1) {
            if (errno == EINTR) {
                continue;
            }
            return EXIT_SYSTEM;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(20)
        );
    }

    // Force termination if the grace period expires.
    if (kill(-pid, SIGKILL) == -1 && errno != ESRCH) {
        kill(pid, SIGKILL);
    }

    if (!wait_for_child(pid, status)) {
        return EXIT_SYSTEM;
    }

    return EXIT_TIMEOUT;
}

int main(int argc, char* argv[]) {
    int max_attempts = 3;
    double delay_seconds = 1.0;
    double max_runtime = DEFAULT_MAX_RUNTIME;

    bool quiet = false;
    bool verbose = false;

    int cmd_start_idx = -1;
    bool dashdash = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (!dashdash && arg == "--") {
            dashdash = true;
            continue;
        }

        if (!dashdash && arg.rfind("-", 0) == 0 && arg != "-") {
            if (arg == "-h" || arg == "--help") {
                print_help();
                return 0;
            } else if (arg == "-n") {
                if (i + 1 >= argc ||
                    !parse_integer(argv[++i], max_attempts)) {
                    std::cerr
                        << "Error: -n requires a positive integer.\n";
                    return EXIT_USAGE;
                }
            } else if (arg == "-d") {
                if (i + 1 >= argc ||
                    !parse_double(argv[++i], delay_seconds) ||
                    delay_seconds < 0 ||
                    delay_seconds > MAX_DELAY_SECONDS) {
                    std::cerr
                        << "Error: -d requires a number between 0 and "
                        << MAX_DELAY_SECONDS << " seconds.\n";
                    return EXIT_USAGE;
                }
            } else if (arg == "-m") {
                if (i + 1 >= argc ||
                    !parse_double(argv[++i], max_runtime) ||
                    max_runtime <= 0 ||
                    max_runtime > MAX_RUNTIME_SECONDS) {
                    std::cerr
                        << "Error: -m requires a positive number no "
                        << "greater than "
                        << MAX_RUNTIME_SECONDS << " seconds.\n";
                    return EXIT_USAGE;
                }
            } else if (arg == "-q") {
                quiet = true;
            } else if (arg == "-v") {
                verbose = true;
            } else {
                std::cerr << "Error: Unknown option " << arg << "\n";
                return EXIT_USAGE;
            }
        } else {
            cmd_start_idx = i;
            break;
        }
    }

    if (cmd_start_idx == -1 || cmd_start_idx >= argc) {
        std::cerr << "Error: No command specified.\n";
        return EXIT_USAGE;
    }

    if (quiet && verbose) {
        // Quiet takes precedence over verbose.
        verbose = false;
    }

    std::vector<char*> cmd_args;
    for (int i = cmd_start_idx; i < argc; ++i) {
        cmd_args.push_back(argv[i]);
    }
    cmd_args.push_back(nullptr);

    const auto start_time = Clock::now();

    Clock::time_point deadline;
    const bool timeout_enabled = max_runtime > 0;

    if (timeout_enabled) {
        deadline = start_time +
            std::chrono::duration_cast<Clock::duration>(
                std::chrono::duration<double>(max_runtime)
            );
    }

    int last_exit_code = 1;

    for (int attempt = 1; attempt <= max_attempts; ++attempt) {
        if (timeout_enabled && Clock::now() >= deadline) {
            if (!quiet) {
                std::cerr
                    << "Error: Maximum total runtime exceeded.\n";
            }
            return EXIT_TIMEOUT;
        }

        if (verbose && !quiet) {
            std::cout << "Starting attempt " << attempt
                      << "/" << max_attempts << ".\n";
        }

        pid_t pid = fork();

        if (pid < 0) {
            std::cerr << "Error: Failed to fork process.\n";
            return EXIT_SYSTEM;
        }

        if (pid == 0) {
            // Isolate the command and its descendants into a process group.
            if (setpgid(0, 0) == -1) {
                _exit(EXIT_SYSTEM);
            }

            execvp(cmd_args[0], cmd_args.data());

            // Match conventional shell exit statuses.
            int exec_error = errno;
            if (exec_error == ENOENT) {
                _exit(127);
            }
            _exit(126);
        }

        // Close the race where the parent runs before the child calls setpgid.
        if (setpgid(pid, pid) == -1 &&
            errno != EACCES && errno != ESRCH) {
            kill(pid, SIGTERM);
        }

        int status = 0;
        bool child_finished = false;

        if (timeout_enabled) {
            while (Clock::now() < deadline) {
                pid_t result = waitpid(pid, &status, WNOHANG);

                if (result == pid) {
                    child_finished = true;
                    break;
                }

                if (result == -1) {
                    if (errno == EINTR) {
                        continue;
                    }

                    std::cerr << "Error: waitpid failed.\n";
                    kill(-pid, SIGKILL);
                    wait_for_child(pid, status);
                    return EXIT_SYSTEM;
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(20)
                );
            }

            if (!child_finished) {
                if (!quiet) {
                    std::cerr
                        << "Error: Maximum total runtime exceeded.\n";
                }

                int timeout_result =
                    terminate_process_group(pid, status);

                if (timeout_result == EXIT_SYSTEM) {
                    std::cerr
                        << "Error: Failed to reap timed-out command.\n";
                }

                return timeout_result;
            }
        } else {
            if (!wait_for_child(pid, status)) {
                std::cerr << "Error: waitpid failed.\n";
                return EXIT_SYSTEM;
            }
            child_finished = true;
        }

        if (WIFEXITED(status)) {
            last_exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            last_exit_code = 128 + WTERMSIG(status);
        } else {
            last_exit_code = EXIT_SYSTEM;
        }

        if (last_exit_code == 0) {
            if (verbose && !quiet) {
                std::cout << "Success on attempt "
                          << attempt << ".\n";
            }
            return 0;
        }

        if (attempt == max_attempts) {
            if (!quiet) {
                std::cerr << "Final attempt " << attempt
                          << "/" << max_attempts
                          << " failed (exit code "
                          << last_exit_code << ").\n";
            }
            break;
        }

        if (!quiet) {
            std::cerr << "Attempt " << attempt << "/"
                      << max_attempts << " failed (exit code "
                      << last_exit_code << ").\n"
                      << "Retrying in " << delay_seconds
                      << " second(s)...\n";
        }

        if (delay_seconds > 0) {
            const auto delay_end = Clock::now() +
                std::chrono::duration_cast<Clock::duration>(
                    std::chrono::duration<double>(delay_seconds)
                );

            if (timeout_enabled) {
                if (!sleep_until_or_for(delay_end, &deadline)) {
                    if (!quiet) {
                        std::cerr
                            << "Error: Maximum total runtime exceeded.\n";
                    }
                    return EXIT_TIMEOUT;
                }
            } else {
                std::this_thread::sleep_until(delay_end);
            }
        }
    }

    return last_exit_code;
}
