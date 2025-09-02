#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp" 

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
	std::vector<Channel*> channels;  // NEW: Channel storage
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


	// find client by nickname
	Client* findClientByNickname(const std::string& nickname);
	
	 // Normal IRC commands
    void handlePRIVMSG(Client& client, const std::string& command);



	// NEW: Channel management methods
    Channel* findChannel(const std::string& channelName);
    Channel* createChannel(const std::string& channelName);
    void removeChannel(Channel* channel);
    void broadcastToChannel(Channel* channel, const std::string& message, Client* sender = NULL);



    void handleJOIN(Client& client, const std::string& command);  
	void handlePART(Client& client, const std::string& command);
    void handleQUIT(Client& client, const std::string& command);
    void handlePING(Client& client, const std::string& command);

	void handleChannelMessage(Client& client, const std::string& channelName, const std::string& message);
	void handleUserMessage(Client& client, const std::string& target, const std::string& message);
};

#endif