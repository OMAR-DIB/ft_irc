#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"



void Cmd::handleINVITE(Server &server,Client &client, const std::string &command)
{
	std::vector<std::string> tokens = server.splitCommand(command);


	if (tokens.size() < 3)
	{
		server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " INVITE :Not enough parameters\r\n");
		return;
	}

	std::string targetNick = tokens[1];
	std::string channelName = tokens[2];

	std::cout << YEL << "=== INVITE DEBUG ===" << WHI << std::endl;
	std::cout << "Inviter: " << client.getNickname() << " (fd:" << client.GetFd() << ")" << std::endl;
	std::cout << "Target: " << targetNick << std::endl;
	std::cout << "Channel: " << channelName << std::endl;

	// Validate channel name format
	if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
	{
		server.sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Find the target client
	Client *targetClient = server.findClientByNickname(targetNick);
	if (!targetClient)
	{
		server.sendToClient(client.GetFd(), ":server 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
		return;
	}

	// Cannot invite yourself
	if (targetClient == &client)
	{
		server.sendToClient(client.GetFd(), ":server 484 " + client.getNickname() + " " + channelName + " :Cannot invite yourself\r\n");
		return;
	}

	// Find the channel
	Channel *channel = server.findChannel(channelName);
	if (!channel)
	{
		server.sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	// Check if inviter is in the channel
	if (!channel->hasClient(&client))
	{
		server.sendToClient(client.GetFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return;
	}

	// Check if inviter is an operator (required for invitations)
	if (!channel->isOperator(&client))
	{
		server.sendToClient(client.GetFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
		return;
	}

	// Check if target is already in the channel
	if (channel->hasClient(targetClient))
	{
		server.sendToClient(client.GetFd(), ":server 443 " + client.getNickname() + " " + targetNick + " " + channelName + " :is already on channel\r\n");
		return;
	}

	std::cout << GRE << "All checks passed, sending invitation..." << WHI << std::endl;

	// Send invitation message to target user
	std::string inviteMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost INVITE " + targetNick + " " + channelName + "\r\n";
	server.sendToClient(targetClient->GetFd(), inviteMsg);

	// Send confirmation to inviter
	server.sendToClient(client.GetFd(), ":server 341 " + client.getNickname() + " " + targetNick + " " + channelName + "\r\n");

	// Optional: Notify other channel operators about the invitation
	std::string notifyMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost NOTICE " + channelName + " :" + client.getNickname() + " has invited " + targetNick + " to join\r\n";

	const std::vector<Client *> &channelClients = channel->getClients();
	for (size_t i = 0; i < channelClients.size(); i++)
	{
		// Notify other operators (not the inviter)
		if (channelClients[i] != &client && channel->isOperator(channelClients[i]))
		{
			server.sendToClient(channelClients[i]->GetFd(), notifyMsg);
		}
	}

	std::cout << GRE << "INVITE completed: " << client.getNickname() << " invited " << targetNick
			  << " to " << channelName << WHI << std::endl;

	// Log current channel state
	std::cout << "Channel " << channelName << " members:";
	for (size_t i = 0; i < channelClients.size(); i++)
	{
		std::cout << " " << channelClients[i]->getNickname();
		if (channel->isOperator(channelClients[i]))
			std::cout << "@";
	}
	std::cout << std::endl;
}