#include "includes/Client.hpp"
#include "includes/Server.hpp"
#include "includes/Helper.hpp"
#include <cstdlib>

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

        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        ser.ServerInit(port);
    }
    catch (const std::exception &e)
    {
        ser.CloseFds();
        std::cerr << e.what() << std::endl;
    }

    std::cout << "The Server Closed!" << std::endl;
    return 0;
}