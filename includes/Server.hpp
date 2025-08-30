#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"

class Server //-> class for server
{
private:
	int Port;					 //-> server port
	int SerSocketFd;			 //-> server socket file descriptor
	static bool Signal;			 //-> static boolean for signal
	std::vector<Client> clients; //-> vector of clients
	// *pollfd* : to check for events on multiple file descriptors without blocking your program
	std::vector<struct pollfd> fds; //-> vector of pollfd
	std::string password;			// Server password
public:
	Server();							   //-> default constructor
	~Server();							   //-> destructor
	void ServerInit(int port);					   //-> server initialization
	void ServerSocket();					   //-> server socket creation
	void AcceptNewClient();				   //-> accept new client
	void ReceiveNewData(int fd);		   //-> receive new data from a registered client
	static void SignalHandler(int signum); //-> signal handler
	void CloseFds();					   //-> close file descriptors
	void ClearClients(int fd);			   //-> clear clients

	void setPassword(const std::string &pass);
	void processCommand(Client &client, const std::string &command);
	std::vector<std::string> splitCommand(const std::string &command);
	void handlePASS(Client &client, const std::string &command);
	void handleNICK(Client &client, const std::string &command);
	void handleUSER(Client &client, const std::string &command);
	void sendToClient(int fd, const std::string &message);


	 // Normal IRC commands
    void handlePRIVMSG(Client& client, const std::string& command);
    void handleJOIN(Client& client, const std::string& command);  
    void handleQUIT(Client& client, const std::string& command);
    void handlePING(Client& client, const std::string& command);
};

#endif