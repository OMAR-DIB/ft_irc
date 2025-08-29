#include "includes/Client.hpp"
#include "includes/Server.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    
    int port = atoi(argv[1]);
    std::string password = argv[2];
    
    if (port <= 0 || port > 65535) {
        std::cerr << "Error: Port must be between 1 and 65535" << std::endl;
        return 1;
    }
    
    Server server(port, password);
    std::cout << "---- IRC SERVER ----" << std::endl;
    std::cout << "Port: " << port << std::endl;
    
    try {
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        server.ServerInit();
    }
    catch(const std::exception& e) {
        server.CloseFds();
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "The Server Closed!" << std::endl;
    return 0;
}