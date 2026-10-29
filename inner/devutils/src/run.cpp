
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct Options {
    std::vector<std::string> env;
    std::string stdout_file;
    std::string stderr_file;
    std::string dir;
    bool quiet = false;
    bool info = false;
    bool async = false;
};

static void usage() {
    std::cout <<
        "Usage: run [OPTIONS] app [ARGS...]\n\n"
        "Options:\n"
        "  -env:NAME=VALUE  Set child environment variable\n"
        "  -stdout:FILE     Redirect stdout\n"
        "  -stderr:FILE     Redirect stderr\n"
        "  -stdort:FILE     Redirect both stdout and stderr\n"
        "  -dir:DIR         Set child working directory\n"
        "  -q               Suppress run's own messages\n"
        "  -i               Print process information\n"
        "  -Q               Return after launching child\n"
        "  -h, --help       Show this help\n";
}

static bool starts_with(const std::string& s,
                        const std::string& prefix) {
    return s.compare(0, prefix.size(), prefix) == 0;
}

static int fail(const Options& opt, const std::string& message) {
    if (!opt.quiet)
        std::cerr << "run: " << message << '\n';
    #ifdef _FAIL2
    return 2;
    #else
    return 127;
    #endif
}

int main(int argc, char* argv[]) {
    Options opt;
    std::vector<std::string> command;
    bool options_enabled = true;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (options_enabled && arg == "--") {
            options_enabled = false;
            continue;
        }

        if (options_enabled &&
            (arg == "-h" || arg == "--help")) {
            usage();
            return 0;
        }

        if (options_enabled && starts_with(arg, "-env:")) {
            std::string assignment = arg.substr(5);
            auto eq = assignment.find('=');

            if (eq == std::string::npos || eq == 0)
                return fail(opt, "expected -env:NAME=VALUE");

            std::string name = assignment.substr(0, eq);
            if (name.find('=') != std::string::npos ||
                name.find('\0') != std::string::npos)
                return fail(opt, "invalid environment variable name");

            opt.env.push_back(assignment);
        } else if (options_enabled && starts_with(arg, "-stdout:")) {
            opt.stdout_file = arg.substr(8);
            if (opt.stdout_file.empty())
                return fail(opt, "empty stdout filename");
        } else if (options_enabled && starts_with(arg, "-stderr:")) {
            opt.stderr_file = arg.substr(8);
            if (opt.stderr_file.empty())
                return fail(opt, "empty stderr filename");
        } else if (options_enabled && starts_with(arg, "-stdort:")) {
            std::string file = arg.substr(8);
            if (file.empty())
                return fail(opt, "empty combined output filename");
            opt.stdout_file = file;
            opt.stderr_file = file;
        } else if (options_enabled && starts_with(arg, "-dir:")) {
            opt.dir = arg.substr(5);
            if (opt.dir.empty())
                return fail(opt, "empty directory");
        } else if (options_enabled && arg == "-q") {
            opt.quiet = true;
        } else if (options_enabled && arg == "-i") {
            opt.info = true;
        } else if (options_enabled && arg == "-Q") {
            opt.async = true;
        } else if (options_enabled && !arg.empty() &&
                   arg[0] == '-') {
            return fail(opt, "unknown option: " + arg);
        } else {
            command.push_back(arg);
            for (++i; i < argc; ++i)
                command.push_back(argv[i]);
            break;
        }
    }

    if (command.empty())
        return fail(opt, "missing application");

    // Construct argv before fork so the child only uses prepared data.
    std::vector<char*> child_argv;
    child_argv.reserve(command.size() + 1);
    for (auto& s : command)
        child_argv.push_back(s.data());
    child_argv.push_back(nullptr);

    // Pipe reports chdir/redirect/exec failures to the parent.
    int error_pipe[2];
    if (pipe(error_pipe) == -1)
        return fail(opt, std::strerror(errno));

    if (fcntl(error_pipe[1], F_SETFD, FD_CLOEXEC) == -1) {
        close(error_pipe[0]);
        close(error_pipe[1]);
        return fail(opt, std::strerror(errno));
    }

    pid_t pid = fork();
    if (pid == -1) {
        int e = errno;
        close(error_pipe[0]);
        close(error_pipe[1]);
        return fail(opt, std::strerror(e));
    }

    if (pid == 0) {
        close(error_pipe[0]);

        auto child_error = [&](int e) {
            (void)write(error_pipe[1], &e, sizeof(e));
            _exit(127);
        };

        if (!opt.dir.empty() && chdir(opt.dir.c_str()) == -1)
            child_error(errno);

        auto redirect = [&](const std::string& path, int target) {
            if (path.empty())
                return true;

            int fd = open(path.c_str(),
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0666);
            if (fd == -1)
                return false;

            if (dup2(fd, target) == -1) {
                int e = errno;
                close(fd);
                errno = e;
                return false;
            }

            if (fd != target)
                close(fd);
            return true;
        };

        if (opt.stdout_file == opt.stderr_file &&
            !opt.stdout_file.empty()) {
            if (!redirect(opt.stdout_file, STDOUT_FILENO))
                child_error(errno);
            if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
                child_error(errno);
        } else {
            if (!redirect(opt.stdout_file, STDOUT_FILENO))
                child_error(errno);
            if (!redirect(opt.stderr_file, STDERR_FILENO))
                child_error(errno);
        }

        for (const auto& assignment : opt.env) {
            auto eq = assignment.find('=');
            std::string name = assignment.substr(0, eq);
            std::string value = assignment.substr(eq + 1);
            if (setenv(name.c_str(), value.c_str(), 1) == -1)
                child_error(errno);
        }

        execvp(child_argv[0], child_argv.data());
        child_error(errno);
    }

    close(error_pipe[1]);

    if (opt.info && !opt.quiet) {
        std::cout << "PID: " << pid << '\n';
        std::cout << "Directory: "
                  << (opt.dir.empty() ? "(inherited)" : opt.dir)
                  << '\n';
        for (const auto& e : opt.env)
            std::cout << "Environment override: " << e << '\n';
        std::cout << "stdout: "
                  << (opt.stdout_file.empty() ? "(inherited)" : opt.stdout_file)
                  << '\n';
        std::cout << "stderr: "
                  << (opt.stderr_file.empty() ? "(inherited)" : opt.stderr_file)
                  << '\n';
        std::cout.flush();
    }

    int child_errno = 0;
    ssize_t n;
    do {
        n = read(error_pipe[0], &child_errno, sizeof(child_errno));
    } while (n == -1 && errno == EINTR);
    close(error_pipe[0]);

    if (n > 0) {
        int status;
        while (waitpid(pid, &status, 0) == -1 && errno == EINTR) {}
        return fail(opt, std::strerror(child_errno));
    }

    if (n == -1) {
        int e = errno;
        kill(pid, SIGTERM);
        while (waitpid(pid, nullptr, 0) == -1 && errno == EINTR) {}
        return fail(opt, std::strerror(e));
    }

    if (opt.async)
        return 0;

    int status;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno == EINTR)
            continue;
        return fail(opt, std::strerror(errno));
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);

    return 1;
}
