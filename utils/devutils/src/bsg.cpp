#include <cctype>
#include <cerrno>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0600

#include <windows.h>
#include <commctrl.h>

#else

#include <gtk/gtk.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#endif

enum class MessageType
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    CRITICAL
};

enum class ButtonType
{
    OK,
    OKRETRY,
    OKCANCEL,
    OKRETRYCANCEL
};

struct Config
{
    MessageType type = MessageType::INFO;
    ButtonType buttons = ButtonType::OK;
    std::string message;
    std::string command;
    bool has_command = false;
};

std::string Upper(std::string value)
{
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::toupper(c));
        }
    );

    return value;
}

bool ParseType(const std::string& value, MessageType& type)
{
    const std::string v = Upper(value);

    if (v == "DEBUG")    type = MessageType::DEBUG;
    else if (v == "INFO") type = MessageType::INFO;
    else if (v == "WARN") type = MessageType::WARN;
    else if (v == "ERROR") type = MessageType::ERROR;
    else if (v == "FATAL") type = MessageType::FATAL;
    else if (v == "CRITICAL") type = MessageType::CRITICAL;
    else return false;

    return true;
}

bool ParseButtons(const std::string& value, ButtonType& type)
{
    const std::string v = Upper(value);

    if (v == "OK") type = ButtonType::OK;
    else if (v == "OKRETRY") type = ButtonType::OKRETRY;
    else if (v == "OKCANCEL") type = ButtonType::OKCANCEL;
    else if (v == "OKRETRYCANCEL")
        type = ButtonType::OKRETRYCANCEL;
    else
        return false;

    return true;
}

void Help()
{
    std::cout <<
        "bsg - Basic System GUI\n\n"
        "Usage: bsg [OPTIONS] [MESSAGE]\n\n"
        "Options:\n"
        "  -t TYPE    DEBUG, INFO, WARN, ERROR, FATAL, CRITICAL\n"
        "  -T TYPE    OK, OKRETRY, OKCANCEL, OKRETRYCANCEL\n"
        "  -C CMD     Run command in background\n"
        "  -h         Display help\n"
        "  -v         Display version\n";
}

bool ParseArguments(int argc, char* argv[], Config& config)
{
    std::vector<std::string> message_parts;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help")
        {
            Help();
            return false;
        }
        else if (arg == "-v" || arg == "--version")
        {
            std::cout << "bsg 1.0.0\n";
            return false;
        }
        else if (arg == "-t" || arg == "-T" || arg == "-C")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "bsg: missing argument for " << arg << '\n';
                return false;
            }

            std::string value = argv[++i];

            if (arg == "-t")
            {
                if (!ParseType(value, config.type))
                {
                    std::cerr << "bsg: invalid message type\n";
                    return false;
                }
            }
            else if (arg == "-T")
            {
                if (!ParseButtons(value, config.buttons))
                {
                    std::cerr << "bsg: invalid button type\n";
                    return false;
                }
            }
            else
            {
                config.command = value;
                config.has_command = true;
            }
        }
        else if (!arg.empty() && arg[0] == '-')
        {
            std::cerr << "bsg: unknown option: " << arg << '\n';
            return false;
        }
        else
        {
            message_parts.push_back(arg);
        }
    }

    for (const auto& part : message_parts)
    {
        if (!config.message.empty())
            config.message += ' ';

        config.message += part;
    }

    if (config.message.empty() && !config.has_command)
    {
        std::cerr << "bsg: message required\n";
        return false;
    }

    return true;
}

#ifdef _WIN32

std::wstring ToWide(const std::string& input)
{
    if (input.empty())
        return {};

    int size = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        input.c_str(), -1, nullptr, 0
    );

    if (!size)
        throw std::runtime_error("Invalid UTF-8 input");

    std::wstring result(size, L'\0');

    MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS,
        input.c_str(), -1, result.data(), size
    );

    result.pop_back();
    return result;
}

bool RunBackground(const std::string& command)
{
    wchar_t comspec[MAX_PATH];

    DWORD length = GetEnvironmentVariableW(
        L"COMSPEC", comspec, MAX_PATH
    );

    if (!length || length >= MAX_PATH)
        return false;

    std::wstring wide_command = ToWide(command);

    std::wstring command_line =
        L"\"" + std::wstring(comspec) +
        L"\" /D /S /C \"" + wide_command + L"\"";

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);

    PROCESS_INFORMATION process{};

    BOOL result = CreateProcessW(
        comspec,
        command_line.data(),
        nullptr,
        nullptr,
        FALSE,
        DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,
        nullptr,
        nullptr,
        &startup,
        &process
    );

    if (!result)
        return false;

    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    return true;
}

int ShowDialog(const Config& config)
{
    TASKDIALOG_BUTTON buttons[3]{};
    int count = 0;

    constexpr int ID_OK = 100;
    constexpr int ID_RETRY = 101;
    constexpr int ID_CANCEL = 102;

    buttons[count++] = { ID_OK, L"OK" };

    if (config.buttons == ButtonType::OKRETRY ||
        config.buttons == ButtonType::OKRETRYCANCEL)
    {
        buttons[count++] = { ID_RETRY, L"Retry" };
    }

    if (config.buttons == ButtonType::OKCANCEL ||
        config.buttons == ButtonType::OKRETRYCANCEL)
    {
        buttons[count++] = { ID_CANCEL, L"Cancel" };
    }

    std::wstring title = L"bsg";
    std::wstring message = ToWide(config.message);

    switch (config.type)
    {
        case MessageType::DEBUG:    title += L" - DEBUG"; break;
        case MessageType::INFO:     title += L" - INFO"; break;
        case MessageType::WARN:     title += L" - WARNING"; break;
        case MessageType::ERROR:    title += L" - ERROR"; break;
        case MessageType::FATAL:    title += L" - FATAL"; break;
        case MessageType::CRITICAL: title += L" - CRITICAL"; break;
    }

    PCWSTR icon = TD_INFORMATION_ICON;

    if (config.type == MessageType::WARN)
        icon = TD_WARNING_ICON;
    else if (config.type == MessageType::ERROR ||
             config.type == MessageType::FATAL ||
             config.type == MessageType::CRITICAL)
        icon = TD_ERROR_ICON;

    TASKDIALOGCONFIG dialog{};
    dialog.cbSize = sizeof(dialog);
    dialog.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION;
    dialog.pszWindowTitle = title.c_str();
    dialog.pszMainIcon = icon;
    dialog.pszContent = message.c_str();
    dialog.cButtons = count;
    dialog.pButtons = buttons;
    dialog.nDefaultButton = ID_OK;

    int selected = 0;

    HRESULT result = TaskDialogIndirect(
        &dialog, &selected, nullptr, nullptr
    );

    if (FAILED(result))
        return 3;

    if (selected == ID_OK) return 0;
    if (selected == ID_RETRY) return 1;
    if (selected == ID_CANCEL) return 2;

    return 2;
}

#else

bool RunBackground(const std::string& command)
{
    pid_t first = fork();

    if (first < 0)
        return false;

    if (first == 0)
    {
        if (setsid() < 0)
            _exit(1);

        pid_t second = fork();

        if (second < 0)
            _exit(1);

        if (second > 0)
            _exit(0);

        int fd = open("/dev/null", O_RDWR);

        if (fd >= 0)
        {
            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);

            if (fd > STDERR_FILENO)
                close(fd);
        }

        execl("/bin/sh", "sh", "-c", command.c_str(),
              static_cast<char*>(nullptr));

        _exit(127);
    }

    int status = 0;

    while (waitpid(first, &status, 0) < 0)
    {
        if (errno != EINTR)
            return false;
    }

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int ShowDialog(const Config& config)
{
    int argc = 1;
    char program[] = "bsg";
    char* argv[] = { program, nullptr };
    char** gtk_argv = argv;

    if (!gtk_init_check(&argc, &gtk_argv))
    {
        std::cerr << "bsg: unable to initialize graphical display\n";
        return 3;
    }

    GtkMessageType type = GTK_MESSAGE_INFO;

    switch (config.type)
    {
        case MessageType::DEBUG:
        case MessageType::INFO:
            type = GTK_MESSAGE_INFO;
            break;

        case MessageType::WARN:
            type = GTK_MESSAGE_WARNING;
            break;

        case MessageType::ERROR:
        case MessageType::FATAL:
        case MessageType::CRITICAL:
            type = GTK_MESSAGE_ERROR;
            break;
    }

    GtkWidget* dialog = gtk_message_dialog_new(
        nullptr,
        GTK_DIALOG_MODAL,
        type,
        GTK_BUTTONS_NONE,
        "%s",
        config.message.c_str()
    );

    gtk_window_set_title(GTK_WINDOW(dialog), "bsg");

    constexpr int ID_OK = 100;
    constexpr int ID_RETRY = 101;
    constexpr int ID_CANCEL = 102;

    gtk_dialog_add_button(GTK_DIALOG(dialog), "OK", ID_OK);

    if (config.buttons == ButtonType::OKRETRY ||
        config.buttons == ButtonType::OKRETRYCANCEL)
    {
        gtk_dialog_add_button(GTK_DIALOG(dialog), "Retry", ID_RETRY);
    }

    if (config.buttons == ButtonType::OKCANCEL ||
        config.buttons == ButtonType::OKRETRYCANCEL)
    {
        gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel", ID_CANCEL);
    }

    gtk_dialog_set_default_response(GTK_DIALOG(dialog), ID_OK);

    int response = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

    while (gtk_events_pending())
        gtk_main_iteration();

    if (response == ID_OK) return 0;
    if (response == ID_RETRY) return 1;

    return 2;
}

#endif

int main(int argc, char* argv[])
{
    Config config;

    if (!ParseArguments(argc, argv, config))
        return 3;

    if (config.has_command)
    {
        if (!RunBackground(config.command))
        {
            std::cerr << "bsg: failed to launch background command\n";
            return 3;
        }
    }

    if (config.message.empty())
        return 0;

    return ShowDialog(config);
}
