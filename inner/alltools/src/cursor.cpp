#include <iostream>
#include <string_view>
#include <string>
#include <chrono>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#define THEME_CLASSIC     100
#define THEME_DOTS        101
#define THEME_NUMWHEEL    102
#define THEME_ARK         103

std::string_view get_arg_value(int argc, char* argv[], int& i) {
    if (i + 1 < argc) {
        return argv[++i];
    }
    std::cerr << "Error: Missing value for argument " << argv[i] << "\n";
    exit(1);
}

int main(int argc, char* argv[]) 
{
    int delay_ms = 100;
    std::string command_to_run = "sleep 41.69420";
    int verbose = 0;
    int selected_theme = THEME_CLASSIC;

    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];
		
		if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: cursor [OPTIONS]\n\n";
            std::cout << "A cross-platform wrapper that animates a loading spinner while monitoring a process.\n\n";
            std::cout << "Options:\n";
            std::cout << "  -h, --help               Show this help menu and exit\n";
            std::cout << "  -c, --copyright          Show tool copyright information and exit\n";
            std::cout << "  -d, --d <ms>             Set animation loop delay frame step in milliseconds (default: 100)\n";
            std::cout << "  --cmd \"<command>\"      The target command (with arguments/spaces) to run asynchronously\n";
            std::cout << "  -v, --verbose            Enable verbose tracking output\n";
            std::cout << "  -th, --theme <name>      Set the visual spinner theme profile\n\n";
            std::cout << "Available Themes:\n";
            std::cout << "  classic                  Standard spinning line pattern: | / - \\\n";
            std::cout << "  updot                    Floating dot sequence:          . * ` *\n";
            std::cout << "  num-wheel                Cascading numeric wheel:        1 2 3 ... 0\n";
            std::cout << "  ark                      Rippling wave pattern:          ~ ` ~ * `\n\n";
            std::cout << "Example:\n";
            std::cout << "  cursor -d 150 -th ark --cmd \"ping 127.0.0.1 -n 4\"\n";
            return 0;
        }
        else if (arg == "-c" || arg == "--copyright") {
            std::cout << "mk-2015 (C) allforall tools, Spinner wrapper tool\n";
            return 0;
        } else if (arg == "--d" || arg == "-d") {
            std::string_view val = get_arg_value(argc, argv, i);
            try {
                delay_ms = std::stoi(std::string(val));
            } catch (...) {
                std::cerr << "Error: Invalid delay integer format.\n";
                return 1;
            }
        } else if (arg == "--cmd") {
            command_to_run = std::string(get_arg_value(argc, argv, i));
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = 1;
        } else if (arg == "-th" || arg == "--theme") {
            std::string_view theme_str = get_arg_value(argc, argv, i); // Unique variable name
            if (theme_str == "updot") {
                selected_theme = THEME_DOTS;
            } else if (theme_str == "num-wheel") 
			{
				selected_theme = THEME_NUMWHEEL;
			} else if (theme_str == "ark")
			{
				selected_theme = THEME_ARK;
			}
        }
    }

    if (command_to_run.empty()) {
        std::cerr << "Usage: spinner --d <ms> --cmd \"<command>\" [--theme updot] [--verbose]\n";
        return 1;
    }

    if (verbose) {
        std::cout << "Executing: " << command_to_run << "\n";
    }

#ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::vector<char> cmd_buffer(command_to_run.begin(), command_to_run.end());
    cmd_buffer.push_back('\0');

    if (!CreateProcessA(
            NULL,
            cmd_buffer.data(),
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi
        )) {
        std::cerr << "Error: Windows failed to spawn process. Code: " << GetLastError() << "\n";
        return 1;
    }
#else
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Error: Fork failed.\n";
        return 1;
    }
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command_to_run.c_str(), (char*)NULL);
        exit(1);
    }
#endif
    
    const char dots_frames[] = { '.', '*', '`', '*' };
    const char classic_frames[] = { '|', '/', '-', '\\' };
    const char numwheel_frames[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	const char ark_frames[] = { '~', '`', '~', '*', '`'};
	
    const char* spinner_frames = classic_frames;
    int frame_count = sizeof(classic_frames) / sizeof(classic_frames[0]);
    
    if (selected_theme == THEME_DOTS) 
	{
        spinner_frames = dots_frames;
        frame_count = sizeof(dots_frames) / sizeof(dots_frames[0]);
    } else if (selected_theme == THEME_NUMWHEEL) 
	{
		spinner_frames = numwheel_frames;
		frame_count = sizeof(numwheel_frames) / sizeof(numwheel_frames[0]); 
	} else if (selected_theme == THEME_ARK)
	{
		spinner_frames = ark_frames;
		frame_count = sizeof(ark_frames) / sizeof(ark_frames[0]); 
	}
    
    int current_frame = 0;

    while (true) {
#ifdef _WIN32
        DWORD exit_code;
        if (GetExitCodeProcess(pi.hProcess, &exit_code)) {
            if (exit_code != STILL_ACTIVE) {
                break;
            }
        }
#else
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result != 0) {
            break;
        }
#endif
        if (verbose) {
            std::cout << "\r[" << spinner_frames[current_frame] << "] Running Tasks...";
        } else {
            std::cout << "\r" << spinner_frames[current_frame];
        }
        std::cout.flush();

        current_frame = (current_frame + 1) % frame_count;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

#ifdef _WIN32
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif

    std::cout << "\r     \n";
    return 0;
}