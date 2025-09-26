
#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"

void Cmd::handlePART(Server &server, Client &client, const std::string &command)
{
	std::vector<std::string> tokens = server.splitCommand(command);

	if (tokens.size() < 2)
	{
		server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PART :Not enough parameters\r\n");
		return;
	}

	std::string channelName = tokens[1];

	// Find channel
	Channel *channel = server.findChannel(channelName);
	if (!channel)
	{
		server.sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Check if client is in channel
	if (!channel->hasClient(&client))
	{
		server.sendToClient(client.GetFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	// Extract part message - REQUIRE colon for trailing parameters
	std::string partMsg;

	// Check if there are additional parameters beyond the required ones
	if (tokens.size() > 2)
	{
		// Find end of "PART" token
		std::string::size_type p1 = command.find(' ');
		if (p1 == std::string::npos)
		{
			server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PART :Not enough parameters\r\n");
			return;
		}

		// Skip spaces/tabs after "PART"
		while (p1 + 1 < command.size() && (command[p1 + 1] == ' ' || command[p1 + 1] == '\t'))
			++p1;

		// Find end of channel token
		std::string::size_type p2 = command.find(' ', p1 + 1);
		if (p2 == std::string::npos)
		{
			server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PART :Not enough parameters\r\n");
			return;
		}

		// Skip spaces/tabs after channel
		std::string::size_type k = p2;
		while (k < command.size() && (command[k] == ' ' || command[k] == '\t'))
			++k;

		if (k < command.size() && command[k] == ':')
		{
			// Good! Found colon for trailing parameter
			partMsg = command.substr(k + 1);
			std::cout << GRE << "Found required colon - part message: [" << partMsg << "]" << WHI << std::endl;
		}
		else
		{
			// ERROR: Additional parameters provided but no colon
			server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PART :Missing ':' for part message\r\n");
			return;
		}
	}
	else
	{
		// No part message provided - this is allowed
		partMsg = "";
		std::cout << YEL << "No part message provided" << WHI << std::endl;
	}

	std::cout << YEL << "Client " << client.getNickname() << " leaving channel " << channelName;
	if (!partMsg.empty())
		std::cout << " with message: [" << partMsg << "]";
	else
		std::cout << " (no part message)";
	std::cout << WHI << std::endl;

	// Create PART message
	std::string fullPartMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PART " + channelName;
	if (!partMsg.empty())
	{
		fullPartMsg += " :" + partMsg;
	}
	fullPartMsg += "\r\n";

	// Send PART to client and broadcast to channel
	server.sendToClient(client.GetFd(), fullPartMsg);
	server.broadcastToChannel(channel, fullPartMsg, &client);

	// Remove client from channel
	channel->removeClient(&client);

	// Remove empty channel
	if (channel->isEmpty())
	{
		server.removeChannel(channel);
	}

	std::cout << GRE << "Client " << client.getNickname() << " left " << channelName << WHI << std::endl;
}