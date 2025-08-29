#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include "IRCMessage.hpp"
#include "Command.hpp"
#include <map>
#include <cstdlib>

class Server {
private:
	int Port;
	std::string Password;
	int SerSocketFd;
	static bool Signal;
	std::vector<Client> clients;
	std::vector<struct pollfd> fds;
	std::map<std::string, Channel*> channels;
	std::string serverName;
	
public:
	Server(int port, const std::string& password);
	~Server();
	
	// Core server functions
	void ServerInit();
	void SerSocket();
	void AcceptNewClient();
	void ReceiveNewData(int fd);
	
	// Signal handling
	static void SignalHandler(int signum);
	void CloseFds();
	void ClearClients(int fd);
	
	// IRC protocol handling
	void processMessage(Client* client, const std::string& message);
	void executeCommand(Client* client, const IRCMessage& ircMsg);
	
	// Client management
	Client* getClient(int fd);
	Client* getClientByNick(const std::string& nickname);
	bool isNickInUse(const std::string& nickname);
	
	// Channel management
	Channel* getChannel(const std::string& name);
	Channel* createChannel(const std::string& name);
	void removeChannel(const std::string& name);
	
	// Utility functions
	void sendMessage(Client* client, const std::string& message);
	void sendReply(Client* client, int code, const std::string& message);
	std::string getServerName() const;
	bool checkPassword(const std::string& password) const;
	
	// Registration helpers
	void tryCompleteRegistration(Client* client);
	void sendWelcomeMessages(Client* client);
};

#endif