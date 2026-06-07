#include <iostream>
#include <string_view>
#include <string>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <unistd.h>
    #include <signal.h>
    #include <sys/types.h>
    #include <cstdlib>
#endif

bool terminate_session() {
#ifdef _WIN32
    if (ExitWindowsEx(EWX_LOGOFF | EWX_FORCEIFHUNG, 0)) {
        return true;
    }
    return false;

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    if (std::getenv("XDG_SESSION_ID") != nullptr) {
        std::string session_id = std::getenv("XDG_SESSION_ID");
        std::string cmd = "loginctl terminate-session " + session_id + " 2>/dev/null";
        if (std::system(cmd.c_str()) == 0) {
            return true;
        }
    }

    pid_t parent_pid = getppid();
    if (parent_pid > 1) {
        if (kill(parent_pid, SIGHUP) == 0) {
            return true;
        }
    }
    
    return false;
#endif
}

void _sleep(double milliseconds) {
    auto start = std::chrono::high_resolution_clock::now();
    
    if (milliseconds > 2.0) {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(milliseconds - 2.0));
    }
    
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - start).count();
        
        if (elapsed >= milliseconds) {
            break; 
        }
        
        if (milliseconds - elapsed > 0.1) {
            std::this_thread::yield();
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "mk-2015 (C) allforall tools, Lockout tool\n";
    
	int is_sleep = 0;
	double sleep_time = 0.0;
		
	for (int i = 1; i < argc; i++)
	{
		std::string_view arg = argv[i];

		if (arg == "-s" || arg == "--sleep")
		{
			if (i + 1 < argc) 
			{
				is_sleep = 1;
				sleep_time = std::stod(argv[++i]);
			}
		}
		
		if (arg == "-h"  || arg == "--help")
		{
			std::cout << "-s or --sleep := sleep for exact millisecond time\n";
		}
	}
	
	if(is_sleep == 1 && sleep_time < 0.0) _sleep(sleep_time);
	
    if (terminate_session()) {
        return 0;
    }

    std::cerr << "Error: Failed to initiate logout sequence.\n";
    return 1;
}