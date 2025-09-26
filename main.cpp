#include "includes/Client.hpp"
#include "includes/Server.hpp"
#include "includes/Helper.hpp"
#include <cstdlib>
#include <csignal>
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    if (port < 0 || port > 65535)
    {
        std::cerr << "Error Port : must be between 1 and 65535." << std::endl;
        return 1;
    }

    std::string password = argv[2];
    if (!Helper::isPasswordValid(password))
    {
        std::cerr << "Error password : alphabetical characters or numbers only." << std::endl;
        return 1;
    }

    Server ser;
    std::cout << "---- SERVER ----" << std::endl;

    try
    {
        ser.setPassword(password);

        // Set up signal handlers BEFORE starting server
        signal(SIGINT, Server::SignalHandler);  // Ctrl+C
        signal(SIGQUIT, Server::SignalHandler); 
        signal(SIGTERM, Server::SignalHandler);  // Termination request

        std::cout << "Server starting on port " << port << "..." << std::endl;
        std::cout << "Press Ctrl+C or Ctrl+\\ to shutdown gracefully" << std::endl;

        ser.ServerInit(port);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        ser.CloseFds(); // Ensure cleanup happens
        return 1;
    }

    std::cout << "The Server Closed!" << std::endl;
    return 0;
}