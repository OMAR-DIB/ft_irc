#include "includes/Client.hpp"
#include "includes/Server.hpp"

int main(int argc,char **argv) {
    (void)argc;
    (void)argv;
    Server ser;
	std::cout << "---- SERVER ----" << std::endl;
	try{
		signal(SIGINT, Server::SignalHandler); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, Server::SignalHandler); //-> catch the signal (ctrl + \)
		ser.ServerInit(); //-> initialize the server
	}
	catch(const std::exception& e){
		ser.CloseFds(); //-> close the file descriptors
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
    return 0;
}