
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
