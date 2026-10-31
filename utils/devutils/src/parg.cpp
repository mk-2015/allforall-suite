
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct Options {
    std::string inputFile;
    std::string delimiter;
    std::string placeholder;
    std::string batchPlaceholder;

    size_t maxArgs = 0;
    size_t maxLines = 0;
    size_t threads = 1;

    bool nullDelimiter = false;
    bool noRunEmpty = false;
    bool prompt = false;
    bool stopOnError = false;
};

using Args = std::vector<std::string>;
using Batch = std::vector<Args>;

static bool number(const std::string& s, size_t& n) {
    try {
        size_t pos = 0;
        unsigned long long v = std::stoull(s, &pos);

        if (pos != s.size() || v == 0)
            return false;

        n = static_cast<size_t>(v);
        return static_cast<unsigned long long>(n) == v;
    } catch (...) {
        return false;
    }
}

static bool parseOptions(int argc, char** argv,
                         Options& opt, Args& command) {
    int i = 1;

    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "--") {
            ++i;
            break;
        }

        if (arg == "-in") {
            if (++i >= argc) {
                std::cerr << "parg: -in requires FILE\n";
                return false;
            }
            opt.inputFile = argv[i++];
        } else if (arg == "-0") {
            opt.nullDelimiter = true;
            ++i;
        } else if (arg == "-mx" || arg == "-L" || arg == "-T") {
            if (++i >= argc) {
                std::cerr << "parg: " << arg << " requires a number\n";
                return false;
            }

            size_t n;
            if (!number(argv[i], n)) {
                std::cerr << "parg: invalid number for " << arg << '\n';
                return false;
            }

            if (arg == "-mx")
                opt.maxArgs = n;
            else if (arg == "-L")
                opt.maxLines = n;
            else
                opt.threads = n;

            ++i;
        } else if (arg == "-nrun") {
            opt.noRunEmpty = true;
            ++i;
        } else if (arg == "-d") {
            if (++i >= argc || std::string(argv[i]).empty()) {
                std::cerr << "parg: -d requires a delimiter\n";
                return false;
            }
            opt.delimiter = argv[i++];
        } else if (arg == "-P") {
            opt.prompt = true;
            ++i;
        } else if (arg == "-p" || arg == "-J") {
            if (++i >= argc || std::string(argv[i]).empty()) {
                std::cerr << "parg: " << arg << " requires a placeholder\n";
                return false;
            }

            if (arg == "-p")
                opt.placeholder = argv[i];
            else
                opt.batchPlaceholder = argv[i];

            ++i;
        } else if (arg == "-e") {
            opt.stopOnError = true;
            ++i;
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "parg: unknown option: " << arg << '\n';
            return false;
        } else {
            break;
        }
    }

    for (; i < argc; ++i)
        command.emplace_back(argv[i]);

    if (command.empty()) {
        std::cerr << "parg: missing command\n";
        return false;
    }

    if (opt.maxArgs && opt.maxLines) {
        std::cerr << "parg: -mx and -L cannot be combined\n";
        return false;
    }

    if (opt.nullDelimiter && !opt.delimiter.empty()) {
        std::cerr << "parg: -0 and -d cannot be combined\n";
        return false;
    }

    if (!opt.placeholder.empty() &&
        !opt.batchPlaceholder.empty()) {
        std::cerr << "parg: -p and -J cannot be combined\n";
        return false;
    }

    if (!opt.placeholder.empty() &&
        (opt.maxArgs || opt.maxLines)) {
        std::cerr << "parg: -p cannot be combined with -mx or -L\n";
        return false;
    }

    if (opt.prompt && opt.threads > 1) {
        std::cerr << "parg: -P cannot be combined with parallel execution\n";
        return false;
    }

    if (!opt.placeholder.empty() &&
        std::none_of(command.begin(), command.end(),
            [&](const std::string& s) {
                return s.find(opt.placeholder) != std::string::npos;
            })) {
        std::cerr << "parg: -p placeholder not found in command\n";
        return false;
    }

    if (!opt.batchPlaceholder.empty() &&
        std::none_of(command.begin(), command.end(),
            [&](const std::string& s) {
                return s.find(opt.batchPlaceholder) != std::string::npos;
            })) {
        std::cerr << "parg: -J placeholder not found in command\n";
        return false;
    }

    return true;
}

static std::string readInput(const Options& opt) {
    std::istream* input = &std::cin;
    std::ifstream file;

    if (!opt.inputFile.empty()) {
        file.open(opt.inputFile, std::ios::binary);

        if (!file) {
            std::cerr << "parg: cannot open input: "
                      << opt.inputFile << '\n';
            return {};
        }

        input = &file;
    }

    return std::string(
        std::istreambuf_iterator<char>(*input),
        std::istreambuf_iterator<char>()
    );
}

static Args split(const std::string& input, const Options& opt) {
    Args result;

    if (opt.nullDelimiter) {
        size_t start = 0;

        while (start < input.size()) {
            size_t end = input.find('\0', start);

            if (end == std::string::npos)
                end = input.size();

            if (end > start)
                result.emplace_back(input.substr(start, end - start));

            start = end + 1;
        }

        return result;
    }

    if (!opt.delimiter.empty()) {
        size_t start = 0;

        while (start < input.size()) {
            size_t end = input.find(opt.delimiter, start);

            if (end == std::string::npos)
                end = input.size();

            if (end > start)
                result.emplace_back(input.substr(start, end - start));

            start = end + opt.delimiter.size();
        }

        return result;
    }

    size_t i = 0;

    while (i < input.size()) {
        while (i < input.size() &&
               std::isspace(static_cast<unsigned char>(input[i])))
            ++i;

        size_t start = i;

        while (i < input.size() &&
               !std::isspace(static_cast<unsigned char>(input[i])))
            ++i;

        if (i > start)
            result.emplace_back(input.substr(start, i - start));
    }

    return result;
}

static Batch makeBatches(const std::string& input,
                         const Options& opt) {
    Batch batches;

    if (!opt.maxLines) {
        Args tokens = split(input, opt);

        if (opt.maxArgs == 0) {
            if (!tokens.empty())
                batches.push_back(std::move(tokens));
            return batches;
        }

        for (size_t i = 0; i < tokens.size(); i += opt.maxArgs) {
            size_t end = std::min(tokens.size(), i + opt.maxArgs);
            batches.emplace_back(
                tokens.begin() + i,
                tokens.begin() + end
            );
        }

        return batches;
    }

    size_t start = 0;
    size_t lineCount = 0;
    Args current;

    while (start < input.size()) {
        size_t end = input.find('\n', start);

        if (end == std::string::npos)
            end = input.size();

        std::string line = input.substr(start, end - start);
        start = end + 1;

        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        Args tokens = split(line, opt);

        if (!tokens.empty()) {
            current.insert(current.end(),
                           tokens.begin(), tokens.end());
            ++lineCount;
        }

        if (lineCount == opt.maxLines) {
            batches.push_back(std::move(current));
            current.clear();
            lineCount = 0;
        }
    }

    if (!current.empty())
        batches.push_back(std::move(current));

    return batches;
}

static Args replaceIndividual(const Args& command,
                              const std::string& holder,
                              const std::string& value) {
    Args result = command;

    for (std::string& arg : result) {
        size_t pos = 0;

        while ((pos = arg.find(holder, pos)) != std::string::npos) {
            arg.replace(pos, holder.size(), value);
            pos += value.size();
        }
    }

    return result;
}

static Args replaceBatch(const Args& command,
                         const std::string& holder,
                         const Args& values) {
    Args result;
    bool replaced = false;

    for (const std::string& arg : command) {
        size_t pos = replaced
            ? std::string::npos
            : arg.find(holder);

        if (pos == std::string::npos) {
            result.push_back(arg);
            continue;
        }

        replaced = true;

        std::string prefix = arg.substr(0, pos);
        std::string suffix = arg.substr(pos + holder.size());

        if (values.empty()) {
            result.push_back(prefix + suffix);
            continue;
        }

        result.push_back(prefix + values.front() + suffix);

        for (size_t i = 1; i < values.size(); ++i)
            result.push_back(values[i]);
    }

    return result;
}

static std::string shellQuote(const std::string& s) {
    std::string result = "'";

    for (char c : s) {
        if (c == '\'')
            result += "'\\''";
        else
            result += c;
    }

    return result + "'";
}

static bool confirm(const Args& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (i)
            std::cout << ' ';
        std::cout << shellQuote(args[i]);
    }

    std::cout << " ? ";
    std::cout.flush();

    std::string answer;
    if (!std::getline(std::cin, answer))
        return false;

    return answer == "y" || answer == "Y" ||
           answer == "yes" || answer == "YES";
}

static int execute(const Args& args) {
    if (args.empty())
        return 1;

    std::vector<char*> argv;
    argv.reserve(args.size() + 1);

    for (const std::string& arg : args)
        argv.push_back(const_cast<char*>(arg.c_str()));

    argv.push_back(nullptr);

    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "parg: fork failed: "
                  << std::strerror(errno) << '\n';
        return 1;
    }

    if (pid == 0) {
        execvp(argv[0], argv.data());
        std::cerr << "parg: cannot execute " << args[0]
                  << ": " << std::strerror(errno) << '\n';
        _exit(127);
    }

    int status = 0;

    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR)
            continue;

        std::cerr << "parg: waitpid failed\n";
        return 1;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);

    return 1;
}

static int runBatches(const Batch& batches,
                      const Args& command,
                      const Options& opt) {
    std::atomic<size_t> next{0};
    std::atomic<bool> failed{false};
    std::mutex promptMutex;

    auto worker = [&]() {
        while (true) {
            if (opt.stopOnError && failed.load())
                break;

            size_t index = next.fetch_add(1);

            if (index >= batches.size())
                break;

            const Args& batch = batches[index];
            Args args;

            if (!opt.placeholder.empty()) {
                args = replaceIndividual(
                    command, opt.placeholder, batch.front()
                );
            } else if (!opt.batchPlaceholder.empty()) {
                args = replaceBatch(
                    command, opt.batchPlaceholder, batch
                );
            } else {
                args = command;
                args.insert(args.end(), batch.begin(), batch.end());
            }

            if (opt.prompt) {
                std::lock_guard<std::mutex> lock(promptMutex);

                if (!confirm(args))
                    continue;
            }

            int status = execute(args);

            if (status != 0) {
                failed.store(true);

                std::cerr << "parg: command exited with status "
                          << status << '\n';
            }
        }
    };

    size_t count = std::min(opt.threads,
                            std::max<size_t>(1, batches.size()));

    std::vector<std::thread> workers;

    for (size_t i = 0; i < count; ++i)
        workers.emplace_back(worker);

    for (auto& thread : workers)
        thread.join();

    return failed.load() ? 1 : 0;
}

int main(int argc, char** argv) {
    Options opt;
    Args command;

    if (!parseOptions(argc, argv, opt, command))
        return 2;

    std::string input = readInput(opt);

    if (opt.inputFile.size() && !std::ifstream(opt.inputFile).good()) {
        std::cerr << "parg: input error\n";
        return 1;
    }

    if (input.empty() && opt.noRunEmpty)
        return 0;

    Batch batches;

    if (!opt.placeholder.empty()) {
        Args tokens = split(input, opt);

        for (const std::string& token : tokens)
            batches.push_back({token});
    } else {
        batches = makeBatches(input, opt);
    }

    if (batches.empty()) {
        if (opt.noRunEmpty)
            return 0;

        if (opt.placeholder.empty() &&
            opt.batchPlaceholder.empty()) {
            batches.push_back({});
        } else {
            return 0;
        }
    }

    return runBatches(batches, command, opt);
}
