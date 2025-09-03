// =========================
// Server.hpp (CHANGES REQUIRED)
// =========================

// Make sure to change the declaration of the clients vector from
//    std::vector<Client> clients;
// to
//    std::vector<Client*> clients;
// and update any forward declarations / includes accordingly.

// Also add a helper prototype:
// Client* findClientByFd(int fd);

// =========================
// Server.cpp (FULL UPDATED - Fix A: Server stores Client* )
// =========================

#include "../includes/Server.hpp"
#include <cstring>
#include <sstream>
#include <cerrno> // for errno

bool Server::Signal = false; //-> initialize the static boolean

Server::Server()
{
	std::cout << YEL << "[Server] constructor" << std::endl;
	SerSocketFd = -1;
}

Server::~Server()
{
	std::cout << YEL << "[Server] destructor" << std::endl;

	// Clean up channels
	for (size_t i = 0; i < channels.size(); i++)
	{
		delete channels[i];
	}
	channels.clear();

	// Clean up clients (they are pointers now)
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i])
		{
			close(clients[i]->GetFd());
			delete clients[i];
		}
	}
	clients.clear();
}

void Server::setPassword(const std::string &pass)
{
	password = pass;
}

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl
			  << "Signal Received!" << std::endl;
	Server::Signal = true; //-> set the static boolean to true to stop the server
}

void Server::CloseFds()
{
	for (size_t i = 0; i < clients.size(); i++)
	{ //-> close all the clients
		if (clients[i])
		{
			std::cout << RED << "Client <" << clients[i]->GetFd() << "> Disconnected" << WHI << std::endl;
			close(clients[i]->GetFd());
			delete clients[i];
		}
	}
	clients.clear();

	if (SerSocketFd != -1)
	{ //-> close the server socket
		std::cout << RED << "Server <" << SerSocketFd << "> Disconnected" << WHI << std::endl;
		close(SerSocketFd);
	}
}

void Server::ServerSocket()
{
	struct sockaddr_in add;
	struct pollfd NewPoll;
	add.sin_family = AF_INET;					   //-> set the address family to ipv4
	add.sin_port = htons(this->Port);			   //-> convert the port to network byte order (big endian)
	add.sin_addr.s_addr = INADDR_ANY;			   //-> set the address to any local machine address
												   //   ip_4    // tcp socket // choose protocol
	SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> create the server socket
	if (SerSocketFd == -1)						   //-> check if the socket is created
		throw(std::runtime_error("faild to create socket"));

	int en = 1;
	if (setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(SerSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));

	NewPoll.fd = SerSocketFd; //-> add the server socket to the pollfd
	NewPoll.events = POLLIN;  //-> set the event to POLLIN for reading data
	NewPoll.revents = 0;	  //-> set the revents to 0
	fds.push_back(NewPoll);	  //-> add the server socket to the pollfd
}

void Server::ServerInit(int port)
{
	this->Port = port;
	ServerSocket(); //-> create the server socket

	std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";

	while (Server::Signal == false) //-> run the server until the signal is received
	{
		if ((poll(&fds[0], fds.size(), -1) == -1) && Server::Signal == false) //-> wait for an event
			throw(std::runtime_error("poll() faild"));

		for (size_t i = 0; i < fds.size(); i++) //-> check all file descriptors
		{
			if (fds[i].revents & POLLIN) //-> check if there is data to read
			{
				if (fds[i].fd == SerSocketFd)
				{
					std::cout << " accept a client...\n";
					AcceptNewClient(); //-> accept new client
				}
				else
					ReceiveNewData(fds[i].fd); //-> receive new data from a registered client
			}
		}
	}
	CloseFds(); //-> close the file descriptors when the server stops
}

void Server::AcceptNewClient()
{
	Client *cli = new Client(); // allocate on heap
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len); //-> accept the new client
	if (incofd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		delete cli;
		return;
	}

	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
	{
		std::cout << "fcntl() failed" << std::endl;
		close(incofd);
		delete cli;
		return;
	}

	NewPoll.fd = incofd;	 //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0;	 //-> set the revents to 0

	cli->SetFd(incofd);							 //-> set the client file descriptor
	cli->setIpAdd(inet_ntoa((cliadd.sin_addr))); //-> convert the ip address to string and set it
	clients.push_back(cli);						 //-> add the client pointer to the vector of clients
	fds.push_back(NewPoll);						 //-> add the client socket to the pollfd

	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::ReceiveNewData(int fd)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);

	if (bytes <= 0)
	{
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		ClearClients(fd);
		close(fd);
		return;
	}

	buff[bytes] = '\0';
	std::cout << YEL << "Client <" << fd << "> Raw Data: " << WHI << buff;

	// Find the client and process the data (clients are pointers now)
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i] && clients[i]->GetFd() == fd)
		{
			// Show client state for debugging
			std::cout << "Client state - Auth: " << (clients[i]->isAuthenticated() ? "YES" : "NO")
					  << ", Registered: " << (clients[i]->isRegistered() ? "YES" : "NO")
					  << ", Nick: [" << clients[i]->getNickname() << "]" << std::endl;

			// Add data to client's buffer
			clients[i]->appendToBuffer(std::string(buff));

			// Process all complete commands
			while (clients[i]->hasCompleteCommand())
			{
				std::string command = clients[i]->extractCommand();
				if (!command.empty())
				{
					processCommand(*clients[i], command);
				}
			}
			break;
		}
	}
}

void Server::ClearClients(int fd)
{
	std::cout << RED << "=== CLEANUP START: Client fd=" << fd << " ===" << WHI << std::endl;

	// Step 1: Find the disconnecting client (store index and pointer)
	int clientIndex = -1;
	Client *clientPtr = NULL;
	std::string clientNick, clientUser;

	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i] && clients[i]->GetFd() == fd)
		{
			clientIndex = i;
			clientPtr = clients[i];
			clientNick = clientPtr->getNickname();
			clientUser = clientPtr->getUsername();
			break;
		}
	}

	if (clientIndex == -1 || clientPtr == NULL)
	{
		std::cout << RED << "ERROR: Client with fd " << fd << " not found!" << WHI << std::endl;
		return;
	}

	std::cout << YEL << "Disconnecting client: " << clientNick << " (fd:" << fd << ")" << WHI << std::endl;

	// Step 2: Show current channel states
	std::cout << YEL << "=== CHANNELS BEFORE CLEANUP ===" << WHI << std::endl;
	for (size_t i = 0; i < channels.size(); i++)
	{
		std::cout << "Channel " << channels[i]->getName() << ":";
		const std::vector<Client *> &chClients = channels[i]->getClients();
		for (size_t j = 0; j < chClients.size(); j++)
		{
			std::cout << " " << chClients[j]->getNickname() << "(fd:" << chClients[j]->GetFd() << ")";
			if (channels[i]->isOperator(chClients[j]))
				std::cout << "@";
		}
		std::cout << std::endl;
	}

	// Step 3: Create quit message
	std::string quitMsg = ":" + clientNick + "!" + clientUser + "@localhost QUIT :Client disconnected\r\n";

	// Step 4: Remove client from channels using fd comparison and broadcast
	std::vector<Channel *> channelsToDelete;

	for (size_t i = 0; i < channels.size(); i++)
	{
		Channel *channel = channels[i];
		std::cout << YEL << "Checking channel " << channel->getName() << WHI << std::endl;

		const std::vector<Client *> &channelClients = channel->getClients();

		for (size_t j = 0; j < channelClients.size(); j++)
		{
			if (channelClients[j] && channelClients[j]->GetFd() == fd)
			{
				std::cout << GRE << "  -> " << clientNick << " IS in " << channel->getName() << WHI << std::endl;

				// Broadcast QUIT to OTHER members
				broadcastToChannel(channel, quitMsg, channelClients[j]);

				// Remove using the CURRENT valid pointer
				channel->removeClient(channelClients[j]);

				break; // client removed from this channel; move on
			}
		}

		// Check if channel is now empty
		if (channel->isEmpty())
		{
			std::cout << RED << "  -> Channel " << channel->getName() << " is now empty" << WHI << std::endl;
			channelsToDelete.push_back(channel);
		}
		else
		{
			std::cout << GRE << "  -> Channel " << channel->getName() << " still has "
					  << channel->getClientCount() << " clients" << WHI << std::endl;
		}
	}

	// Step 5: Delete empty channels
	for (size_t i = 0; i < channelsToDelete.size(); i++)
	{
		std::cout << RED << "Deleting empty channel: " << channelsToDelete[i]->getName() << WHI << std::endl;

		for (size_t j = 0; j < channels.size(); j++)
		{
			if (channels[j] == channelsToDelete[i])
			{
				delete channels[j];
				channels.erase(channels.begin() + j);
				break;
			}
		}
	}

	// Step 6: Show channel states AFTER cleanup
	std::cout << YEL << "=== CHANNELS AFTER CLEANUP ===" << WHI << std::endl;
	for (size_t i = 0; i < channels.size(); i++)
	{
		std::cout << "Channel " << channels[i]->getName() << ":";
		const std::vector<Client *> &chClients = channels[i]->getClients();
		for (size_t j = 0; j < chClients.size(); j++)
		{
			std::cout << " " << chClients[j]->getNickname() << "(fd:" << chClients[j]->GetFd() << ")";
			if (channels[i]->isOperator(chClients[j]))
				std::cout << "@";
		}
		std::cout << std::endl;
	}

	// Step 7: Remove from server's fds vector
	for (size_t i = 0; i < fds.size(); i++)
	{
		if (fds[i].fd == fd)
		{
			fds.erase(fds.begin() + i);
			break;
		}
	}

	// Step 8: Remove and delete client pointer from server->clients
	Client *toDelete = clients[clientIndex];
	clients.erase(clients.begin() + clientIndex);
	delete toDelete;

	std::cout << GRE << "=== CLEANUP COMPLETE for " << clientNick << " ===" << WHI << std::endl;
}

void Server::processCommand(Client &client, const std::string &command)
{
	std::cout << "Processing command: [" << command << "]" << std::endl;

	std::vector<std::string> tokens = splitCommand(command);
	if (tokens.empty())
		return;

	std::string cmd = tokens[0];
	// Convert to uppercase for case-insensitive comparison
	for (size_t i = 0; i < cmd.length(); i++)
	{
		cmd[i] = std::toupper(cmd[i]);
	}

	// AUTHENTICATION CHECK: Only allow these commands for unauthenticated clients
	if (!client.isAuthenticated())
	{
		if (cmd == "PASS")
		{
			handlePASS(client, command);
			return;
		}
		else
		{
			// Reject everything else for unauthenticated clients
			sendToClient(client.GetFd(), ":server 464 * :Password required\r\n");
			std::cout << RED << "Client <" << client.GetFd() << "> tried command '" << cmd << "' without authentication" << WHI << std::endl;
			return;
		}
	}

	// REGISTRATION CHECK: Only allow NICK and USER for authenticated but unregistered clients
	if (!client.isRegistered())
	{
		if (cmd == "NICK")
		{
			handleNICK(client, command);
			return;
		}
		else if (cmd == "USER")
		{
			handleUSER(client, command);
			return;
		}
		else if (cmd == "PASS")
		{
			// Already authenticated, don't allow PASS again
			sendToClient(client.GetFd(), ":server 462 " + client.getNickname() + " :You may not reregister\r\n");
			return;
		}
		else
		{
			// Reject other commands for unregistered clients
			sendToClient(client.GetFd(), ":server 451 * :You have not registered\r\n");
			std::cout << RED << "Client <" << client.GetFd() << "> tried command '" << cmd << "' without registration" << WHI << std::endl;
			return;
		}
	}

	// CLIENT IS FULLY AUTHENTICATED AND REGISTERED
	// Now handle normal IRC commands
	if (cmd == "PRIVMSG")
	{
		handlePRIVMSG(client, command);
	}
	else if (cmd == "JOIN")
	{
		handleJOIN(client, command);
	}
	else if (cmd == "PART")
	{
		handlePART(client, command);
	}
	else if (cmd == "QUIT")
	{
		handleQUIT(client, command);
	}
	else if (cmd == "PING")
	{
		handlePING(client, command);
	}
	else if (cmd == "TOPIC")
	{
		handleTOPIC(client, command);
	}
	else if (cmd == "PASS" || cmd == "NICK" || cmd == "USER")
	{
		// Already registered, don't allow these again
		sendToClient(client.GetFd(), ":server 462 " + client.getNickname() + " :You may not reregister\r\n");
	}
	else
	{
		// Unknown command
		sendToClient(client.GetFd(), ":server 421 " + client.getNickname() + " " + cmd + " :Unknown command\r\n");
	}
}

std::vector<std::string> Server::splitCommand(const std::string &command)
{
	std::vector<std::string> tokens;
	std::stringstream ss(command);
	std::string token;

	while (ss >> token)
	{
		tokens.push_back(token);
	}

	return tokens;
}

void Server::sendToClient(int fd, const std::string &message)
{
	ssize_t sent = send(fd, message.c_str(), message.length(), 0);
	if (sent < 0)
	{
		std::cout << RED << "Failed to send to client <" << fd << ">: " << strerror(errno) << WHI << std::endl;
		// Don't try to use this fd anymore
		return;
	}

	std::cout << GRE << "Sent to client <" << fd << ">: " << message << WHI;
}

void Server::handlePASS(Client &client, const std::string &command)
{
	std::vector<std::string> tokens = splitCommand(command);

	if (tokens.size() < 2)
	{
		sendToClient(client.GetFd(), ":server 461 * PASS :Not enough parameters\r\n");
		return;
	}

	if (client.isAuthenticated())
	{
		sendToClient(client.GetFd(), ":server 462 * :You may not reregister\r\n");
		return;
	}

	if (tokens[1] == password)
	{
		client.setAuthenticated(true);
		std::cout << GRE << "Client <" << client.GetFd() << "> authenticated" << WHI << std::endl;
	}
	else
	{
		sendToClient(client.GetFd(), ":server 464 * :Password incorrect\r\n");
		// Disconnect client with wrong password
		std::cout << RED << "Client <" << client.GetFd() << "> wrong password" << WHI << std::endl;
		close(client.GetFd());
		ClearClients(client.GetFd());
	}
}

void Server::handleNICK(Client &client, const std::string &command)
{
	std::vector<std::string> tokens = splitCommand(command);

	if (tokens.size() < 2)
	{
		sendToClient(client.GetFd(), ":server 431 * :No nickname given\r\n");
		return;
	}

	if (!client.isAuthenticated())
	{
		sendToClient(client.GetFd(), ":server 464 * :Password required\r\n");
		return;
	}

	std::string nick = tokens[1];

	// Check if nickname is already taken
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i] && clients[i]->getNickname() == nick && clients[i]->GetFd() != client.GetFd())
		{
			sendToClient(client.GetFd(), ":server 433 * " + nick + " :Nickname is already in use\r\n");
			return;
		}
	}

	client.setNickname(nick);
	std::cout << GRE << "Client <" << client.GetFd() << "> set nickname: " << nick << WHI << std::endl;

	// Check if client is now fully registered
	if (!client.getUsername().empty())
	{
		client.setRegistered(true);
		sendToClient(client.GetFd(), ":server 001 " + nick + " :Welcome to the IRC Network!\r\n");
	}
}

void Server::handleUSER(Client &client, const std::string &command)
{
	std::vector<std::string> tokens = splitCommand(command);

	if (tokens.size() < 5)
	{
		sendToClient(client.GetFd(), ":server 461 * USER :Not enough parameters\r\n");
		return;
	}

	if (!client.isAuthenticated())
	{
		sendToClient(client.GetFd(), ":server 464 * :Password required\r\n");
		return;
	}

	if (client.isRegistered())
	{
		sendToClient(client.GetFd(), ":server 462 * :You may not reregister\r\n");
		return;
	}

	client.setUsername(tokens[1]);
	// tokens[4] and beyond are the real name (starts with :)
	std::string realname = tokens[4];
	if (realname[0] == ':')
	{
		realname = realname.substr(1); // Remove the ':'
	}
	client.setRealname(realname);

	std::cout << GRE << "Client <" << client.GetFd() << "> set username: " << tokens[1] << WHI << std::endl;

	// Check if client is now fully registered
	if (!client.getNickname().empty())
	{
		client.setRegistered(true);
		sendToClient(client.GetFd(), ":server 001 " + client.getNickname() + " :Welcome to the IRC Network!\r\n");
	}
}

// find client by nickname
Client *Server::findClientByNickname(const std::string &nickname)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i] && clients[i]->getNickname() == nickname && clients[i]->isRegistered())
		{
			return clients[i];
		}
	}
	return NULL;
}

// find client by fd
Client *Server::findClientByFd(int fd)
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i] && clients[i]->GetFd() == fd)
			return clients[i];
	}
	return NULL;
}

// Normal IRC commands
void Server::handlePRIVMSG(Client &client, const std::string &command)
{
	std::cout << YEL << "Processing PRIVMSG from " << client.getNickname() << WHI << std::endl;

	// Parse the command: PRIVMSG <target> :<message>
	size_t firstSpace = command.find(' ');
	if (firstSpace == std::string::npos)
	{
		sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
		return;
	}

	size_t secondSpace = command.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
	{
		sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
		return;
	}

	// Extract target (channel or user)
	std::string target = command.substr(firstSpace + 1, secondSpace - firstSpace - 1);

	// Extract message (should start with ':')
	std::string message = command.substr(secondSpace + 1);
	if (message.empty() || message[0] != ':')
	{
		sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
		return;
	}

	// Remove the ':' from the beginning of the message
	message = message.substr(1);

	if (message.empty())
	{
		sendToClient(client.GetFd(), ":server 412 " + client.getNickname() + " :No text to send\r\n");
		return;
	}

	std::cout << "PRIVMSG: [" << client.getNickname() << "] -> [" << target << "]: " << message << std::endl;

	// Check if target is a channel (starts with # or &)
	if (target[0] == '#' || target[0] == '&')
	{
		handleChannelMessage(client, target, message);
	}
	else
	{
		handleUserMessage(client, target, message);
	}
}

void Server::handleChannelMessage(Client &client, const std::string &channelName, const std::string &message)
{
	// Find the channel
	Channel *channel = findChannel(channelName);
	if (!channel)
	{
		sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Check if client is in the channel
	if (!channel->hasClient(&client))
	{
		sendToClient(client.GetFd(), ":server 404 " + client.getNickname() + " " + channelName + " :Cannot send to channel\r\n");
		return;
	}

	// Create the message to broadcast
	std::string fullMessage = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + channelName + " :" + message + "\r\n";

	// Broadcast to all channel members except sender
	broadcastToChannel(channel, fullMessage, &client);

	std::cout << GRE << "Channel message delivered: " << client.getNickname() << " -> " << channelName << ": " << message << WHI << std::endl;
}

void Server::handleUserMessage(Client &client, const std::string &target, const std::string &message)
{
	// Find the target client
	Client *targetClient = findClientByNickname(target);
	if (!targetClient)
	{
		sendToClient(client.GetFd(), ":server 401 " + client.getNickname() + " " + target + " :No such nick/channel\r\n");
		return;
	}

	// Send the message to the target client
	std::string fullMessage = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";
	sendToClient(targetClient->GetFd(), fullMessage);

	std::cout << GRE << "Private message delivered: " << client.getNickname() << " -> " << target << ": " << message << WHI << std::endl;
}

void Server::handlePING(Client &client, const std::string &command)
{
	std::vector<std::string> tokens = splitCommand(command);

	std::cout << YEL << "PING from " << client.getNickname() << WHI << std::endl;

	// PING <server> or just PING
	std::string pongMsg = ":server PONG server";
	if (tokens.size() > 1)
	{
		pongMsg += " :" + tokens[1];
	}
	pongMsg += "\r\n";

	sendToClient(client.GetFd(), pongMsg);
	std::cout << GRE << "PONG sent to " << client.getNickname() << WHI << std::endl;
}

// Channel management methods:
Channel *Server::findChannel(const std::string &channelName)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i]->getName() == channelName)
		{
			return channels[i];
		}
	}
	return NULL;
}

Channel *Server::createChannel(const std::string &channelName)
{
	Channel *channel = new Channel(channelName);
	channels.push_back(channel);
	std::cout << GRE << "Created channel: " << channelName << WHI << std::endl;
	return channel;
}

void Server::removeChannel(Channel *channel)
{
	if (channel && channel->isEmpty())
	{
		std::vector<Channel *>::iterator it = std::find(channels.begin(), channels.end(), channel);
		if (it != channels.end())
		{
			std::cout << RED << "Removing empty channel: " << channel->getName() << WHI << std::endl;
			channels.erase(it);
			delete channel;
		}
	}
}

void Server::broadcastToChannel(Channel *channel, const std::string &message, Client *sender)
{
	if (!channel)
		return;

	const std::vector<Client *> &channelClients = channel->getClients();
	for (size_t i = 0; i < channelClients.size(); i++)
	{
		// Don't send message back to sender
		if (channelClients[i] != sender)
		{
			int targetFd = channelClients[i]->GetFd();

			// Verify the fd is still valid by checking it exists in our active clients
			bool fdValid = false;
			for (size_t j = 0; j < clients.size(); j++)
			{
				if (clients[j] && clients[j]->GetFd() == targetFd)
				{
					fdValid = true;
					break;
				}
			}

			if (fdValid)
			{
				sendToClient(targetFd, message);
			}
			else
			{
				std::cout << RED << "Skipping broadcast to invalid fd: " << targetFd << WHI << std::endl;
			}
		}
	}
}

void Server::handleJOIN(Client &client, const std::string &command)
{
	std::vector<std::string> tokens = splitCommand(command);

	if (tokens.size() < 2)
	{
		sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " JOIN :Not enough parameters\r\n");
		return;
	}

	std::string channelName = tokens[1];

	if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
	{
		sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	std::cout << YEL << "=== JOIN DEBUG ===" << WHI << std::endl;
	std::cout << "Client " << client.getNickname() << " (fd:" << client.GetFd() << ") joining " << channelName << WHI << std::endl;

	Channel *channel = findChannel(channelName);
	if (!channel)
	{
		channel = createChannel(channelName);
		std::cout << GRE << "Created new channel " << channelName << WHI << std::endl;
	}
	else
	{
		std::cout << "Found existing channel " << channelName << WHI << std::endl;
		std::cout << "Current members:";
		const std::vector<Client *> &channelClients = channel->getClients();
		for (size_t i = 0; i < channelClients.size(); i++)
		{
			std::cout << " " << channelClients[i]->getNickname() << "(fd:" << channelClients[i]->GetFd() << ")";
		}
		std::cout << std::endl;
	}

	// Check if client is already in channel (compare by fd)
	if (channel->hasClient(&client))
	{
		std::cout << RED << "DUPLICATE JOIN DETECTED for " << client.getNickname() << WHI << std::endl;
		std::cout << "Channel thinks this client is already present!" << std::endl;
		return;
	}

	// Add client to channel
	channel->addClient(&client);

	// Send JOIN messages
	std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost JOIN " + channelName + "\r\n";
	sendToClient(client.GetFd(), joinMsg);
	broadcastToChannel(channel, joinMsg, &client);

	// TODO: Handle channel topic when new use enter
	// Send topic if exists, otherwise tell client there's no topic
	if (!channel->getTopic().empty())
	{
		sendToClient(client.GetFd(), ":server 332 " + client.getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
		// optional: if you keep who/time info in channel
		// sendToClient(client.GetFd(), ":server 333 " + client.getNickname() + " " + channelName + " <setter> <timestamp>\r\n");
	}
	else
	{
		sendToClient(client.GetFd(), ":server 331 " + client.getNickname() + " " + channelName + " :No topic is set\r\n");
	}

	// Send names list
	sendToClient(client.GetFd(), ":server 353 " + client.getNickname() + " = " + channelName + " :" + channel->getClientsList() + "\r\n");
	sendToClient(client.GetFd(), ":server 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list\r\n");

	std::cout << GRE << "Client " << client.getNickname() << " successfully joined " << channelName
			  << " (" << channel->getClientCount() << " clients)" << WHI << std::endl;
}

void Server::handlePART(Client &client, const std::string &command)
{
	std::vector<std::string> tokens = splitCommand(command);

	if (tokens.size() < 2)
	{
		sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PART :Not enough parameters\r\n");
		return;
	}

	std::string channelName = tokens[1];

	// Find channel
	Channel *channel = findChannel(channelName);
	if (!channel)
	{
		sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Check if client is in channel
	if (!channel->hasClient(&client))
	{
		sendToClient(client.GetFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	// Extract part message if provided
	std::string partMsg;
	if (tokens.size() > 2)
	{
		partMsg = tokens[2];
		if (partMsg[0] == ':')
		{
			partMsg = partMsg.substr(1);
		}
		// Rebuild message from remaining tokens
		for (size_t i = 3; i < tokens.size(); i++)
		{
			partMsg += " " + tokens[i];
		}
	}

	std::cout << YEL << "Client " << client.getNickname() << " leaving channel " << channelName << WHI << std::endl;

	// Create PART message
	std::string fullPartMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PART " + channelName;
	if (!partMsg.empty())
	{
		fullPartMsg += " :" + partMsg;
	}
	fullPartMsg += "\r\n";

	// Send PART to client and broadcast to channel
	sendToClient(client.GetFd(), fullPartMsg);
	broadcastToChannel(channel, fullPartMsg, &client);

	// Remove client from channel
	channel->removeClient(&client);

	// Remove empty channel
	if (channel->isEmpty())
	{
		removeChannel(channel);
	}

	std::cout << GRE << "Client " << client.getNickname() << " left " << channelName << WHI << std::endl;
}

void Server::handleTOPIC(Client &client, const std::string &command)
{
	std::vector<std::string> tokens = splitCommand(command);

	if (tokens.size() < 2)
	{
		sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
		return;
	}

	std::string channelName = tokens[1];

	// Find channel
	Channel *channel = findChannel(channelName);
	if (!channel)
	{
		sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	std::cout << YEL << "=== TOPIC DEBUG ===" << WHI << std::endl;
	std::cout << "Client: " << client.getNickname() << " (fd:" << client.GetFd() << ")" << std::endl;
	std::cout << "Channel " << channelName << " members:" << std::endl;
	const std::vector<Client *> &channelClients = channel->getClients();
	for (size_t i = 0; i < channelClients.size(); i++)
	{
		std::cout << "  - " << channelClients[i]->getNickname()
				  << " (fd:" << channelClients[i]->GetFd() << ")"
				  << (channel->isOperator(channelClients[i]) ? " @OP" : "")
				  << std::endl;
	}

	// Check if client is in channel
	bool clientFound = false;
	for (size_t i = 0; i < channelClients.size(); i++)
	{
		if (channelClients[i]->GetFd() == client.GetFd())
		{
			clientFound = true;
			break;
		}
	}

	if (!clientFound)
	{
		sendToClient(client.GetFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		std::cout << RED << "Client " << client.getNickname() << " NOT FOUND in channel!" << WHI << std::endl;
		return;
	}

	std::cout << GRE << "Client " << client.getNickname() << " IS in channel" << WHI << std::endl;

	// Rest of TOPIC logic...
	if (tokens.size() == 2)
	{
		if (channel->getTopic().empty())
		{
			sendToClient(client.GetFd(), ":server 331 " + client.getNickname() + " " + channelName + " :No topic is set\r\n");
		}
		else
		{
			sendToClient(client.GetFd(), ":server 332 " + client.getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
		}
		return;
	}

	// Check operator privileges
	if (!channel->isOperator(&client))
	{
		sendToClient(client.GetFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// Set topic
	std::string newTopic = tokens[2];
	if (newTopic[0] == ':')
	{
		newTopic = newTopic.substr(1);
	}
	for (size_t i = 3; i < tokens.size(); i++)
	{
		newTopic += " " + tokens[i];
	}

	channel->setTopic(newTopic);

	std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + channelName + " :" + newTopic + "\r\n";
	for (size_t i = 0; i < channelClients.size(); i++)
	{
		sendToClient(channelClients[i]->GetFd(), topicMsg);
	}

	std::cout << GRE << "Topic changed successfully" << WHI << std::endl;
}

void Server::handleQUIT(Client &client, const std::string &command)
{
	std::vector<std::string> tokens = splitCommand(command);

	// Extract quit message if provided
	std::string quitMsg = "Quit";
	if (tokens.size() > 1)
	{
		quitMsg = tokens[1];
		if (quitMsg[0] == ':')
		{
			quitMsg = quitMsg.substr(1);
		}
		for (size_t i = 2; i < tokens.size(); i++)
		{
			quitMsg += " " + tokens[i];
		}
	}

	std::cout << RED << "Client " << client.getNickname() << " is quitting: " << quitMsg << WHI << std::endl;

	// Create quit message to broadcast
	std::string fullQuitMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost QUIT :" + quitMsg + "\r\n";

	// Remove from all channels and broadcast quit message
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i]->hasClient(&client))
		{
			// Broadcast quit to channel members
			broadcastToChannel(channels[i], fullQuitMsg, &client);

			// Remove from channel
			channels[i]->removeClient(&client);
		}
	}

	// Send error closing message to client
	sendToClient(client.GetFd(), ":server ERROR :Closing Link: " + client.getNickname() + " (" + quitMsg + ")\r\n");

	// Close connection
	close(client.GetFd());
	ClearClients(client.GetFd());
}

// =========================
// Client.cpp (unchanged interface, implementation identical except destructor prints may change)
// =========================

// =========================
// Channel.cpp (unchanged, already using Client*; kept for completeness)
// =========================
