#include <iostream>
#include <string_view>
#include <string>
#include <memory>
#include <filesystem>
#include <httplib.h>

int main(int argc, char* argv[])
{
    std::cout << "mk-2015 (C) allforall tools, Serve tool\n";
    
    int is_http = 1;
    int port = 8080;
	int cors_enabled = 0;
    std::string folder = ".";
    
    for(int i = 1; i < argc; i++)
    {
        std::string_view arg = argv[i]; 
        
		if(arg == "-h" || arg == "--help")
		{
			std::cout << "All For All: Serve Tool\n"
                      << "Usage: serve [options]\n\n"
                      << "Options:\n"
                      << "  -h, --help        Show this help menu and exit\n"
                      << "  --cors            Enable Cross-Origin Resource Sharing (CORS headers)\n"
                      << "  --https           Enable secure HTTPS mode (requires local cert.pem and key.pem)\n"
                      << "  --port <number>   Specify custom port number (default: 8080)\n"
                      << "  --folder <path>   Specify the directory path to serve (default: \".\")\n";
            return 0;		
		}
		
		else if (arg == "--cors")
		{
			cors_enabled = 1;
		}
        else if(arg == "--https") 
        {
            is_http = 0;
        }
        else if(arg == "--port") 
        {
            if(i + 1 < argc) 
            {
                try {
                    port = std::stoi(argv[++i]);
                } catch (...) {
                    std::cerr << "Error: Invalid port number format.\n";
                    return 1;
                }
            }
        }
        else if(arg == "--folder") 
        {
            if(i + 1 < argc)
            {
                folder = argv[++i];
            }
        }
    }
    
    if (!std::filesystem::exists(folder) || !std::filesystem::is_directory(folder)) {
        std::cerr << "Error: Target path '" << folder << "' is not a valid directory.\n";
        return 1;
    }

    std::unique_ptr<httplib::Server> svr;

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    if (!is_http) {
        if (!std::filesystem::exists("cert.pem") || !std::filesystem::exists("key.pem")) {
            std::cerr << "Error: SSL mode requires 'cert.pem' and 'key.pem' in the execution directory.\n";
            return 1;
        }
        svr = std::make_unique<httplib::SSLServer>("cert.pem", "key.pem");
    } else {
        svr = std::make_unique<httplib::Server>();
    }
#else
    if (!is_http) {
        std::cerr << "Error: Binary not compiled with OpenSSL support. '--https' is unavailable.\n";
        return 1;
    }
    svr = std::make_unique<httplib::Server>();
#endif
    svr->set_mount_point("/", folder);
    
	if (cors_enabled) {
        svr->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            
            if (req.method == "OPTIONS") {
                res.status = 204;
                return httplib::Server::HandlerResponse::Handled;
            }
            
            return httplib::Server::HandlerResponse::Unhandled;
        });
    }
	
    std::cout << "Listening on 0.0.0.0:" << port << "/ (Serving: " << std::filesystem::absolute(folder) << ")\n";
    
    if (!svr->listen("0.0.0.0", port)) {
        std::cerr << "Error: Could not bind server to port " << port << "\n";
        return 1;
    }

    return 0;
}