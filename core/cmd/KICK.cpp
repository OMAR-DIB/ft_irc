
#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"


void Cmd::handleKICK(Server &server, Client &client, const std::string &command)
{
	std::vector<std::string> tokens = server.splitCommand(command);

	if (tokens.size() < 3)
	{
		server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
		return;
	}

	std::string channelName = tokens[1];
	std::string targetNick = tokens[2];

	std::cout << YEL << "=== KICK DEBUG ===" << WHI << std::endl;
	std::cout << "Kicker: " << client.getNickname() << " (fd:" << client.GetFd() << ")" << std::endl;
	std::cout << "Channel: " << channelName << std::endl;
	std::cout << "Target: " << targetNick << std::endl;

	// Find the channel
	Channel *channel = server.findChannel(channelName);
	if (!channel)
	{
		server.sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Check if kicker is in the channel
	if (!channel->hasClient(&client))
	{
		server.sendToClient(client.GetFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	// Check if kicker is an operator
	if (!channel->isOperator(&client))
	{
		server.sendToClient(client.GetFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// Find the target client
	Client *targetClient = server.findClientByNickname(targetNick);
	if (!targetClient)
	{
		server.sendToClient(client.GetFd(), ":server 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
		return;
	}

	// Check if target is in the channel
	if (!channel->hasClient(targetClient))
	{
		server.sendToClient(client.GetFd(), ":server 441 " + client.getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
		return;
	}

	// Prevent self-kick
	if (targetClient == &client)
	{
		server.sendToClient(client.GetFd(), ":server 484 " + client.getNickname() + " " + channelName + " :Cannot kick yourself\r\n");
		return;
	}

	// Extract kick reason if provided
	std::string kickReason;
	if (tokens.size() > 3)
	{
		kickReason = tokens[3];
		if (kickReason[0] == ':')
		{
			kickReason = kickReason.substr(1); // Remove the ':'
		}
		// Rebuild reason from remaining tokens
		for (size_t i = 4; i < tokens.size(); i++)
		{
			kickReason += " " + tokens[i];
		}
	}
	else
	{
		kickReason = client.getNickname(); // Default reason is kicker's nickname
	}

	std::cout << "Kick reason: [" << kickReason << "]" << std::endl;

	// Create KICK message
	std::string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + channelName + " " + targetNick + " :" + kickReason + "\r\n";

	// Broadcast KICK message to ALL channel members (including kicker and target)
	const std::vector<Client *> &channelClients = channel->getClients();
	for (size_t i = 0; i < channelClients.size(); i++)
	{
		server.sendToClient(channelClients[i]->GetFd(), kickMsg);
	}

	std::cout << GRE << "Broadcasting KICK: " << kickMsg << WHI;

	// Remove target from channel
	channel->removeClient(targetClient);

	// Check if channel is empty and remove if needed
	if (channel->isEmpty())
	{
		server.removeChannel(channel);
		std::cout << RED << "Channel " << channelName << " deleted (empty after kick)" << WHI << std::endl;
	}

	std::cout << GRE << "KICK completed: " << client.getNickname() << " kicked " << targetNick
			  << " from " << channelName << " (reason: " << kickReason << ")" << WHI << std::endl;
}
