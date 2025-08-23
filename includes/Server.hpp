#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"

class Server //-> class for server
{
private:
	int Port; //-> server port
	int SerSocketFd; //-> server socket file descriptor
	static bool Signal; //-> static boolean for signal
	std::vector<Client> clients; //-> vector of clients
	// *pollfd* : to check for events on multiple file descriptors without blocking your program
	std::vector<struct pollfd> fds; //-> vector of pollfd
public:
	Server(); //-> default constructor
	~Server();                  //-> destructor
	void ServerInit(); //-> server initialization
	void SerSocket(); //-> server socket creation
	void AcceptNewClient(); //-> accept new client
	void ReceiveNewData(int fd); //-> receive new data from a registered client

	static void SignalHandler(int signum); //-> signal handler
 
	void CloseFds(); //-> close file descriptors
	void ClearClients(int fd); //-> clear clients
};


#endif