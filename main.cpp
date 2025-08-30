#include "includes/Client.hpp"
#include "includes/Server.hpp"
#include <cstdlib>

int main(int argc,char **argv) {
    if(argc != 3) {
        std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }	
    Server ser;
	std::cout << "---- SERVER ----" << std::endl;
	try {
        int port = std::atoi(argv[1]);
        std::string password = argv[2];
        
        ser.setPassword(password);  // Set the password
        // You'll also need to modify ServerInit to accept port parameter
        
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        ser.ServerInit(port);
    }
    catch(const std::exception& e) {
        ser.CloseFds();
        std::cerr << e.what() << std::endl;
    }
    
    std::cout << "The Server Closed!" << std::endl;
    return 0;
}