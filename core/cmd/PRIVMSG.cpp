#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"



void Cmd::handlePRIVMSG(Server &server, Client &client, const std::string &command)
{
	
	std::cout << YEL << "Processing PRIVMSG from " << client.getNickname() << WHI << std::endl;

	// Parse the command: PRIVMSG <target> :<message>
	size_t firstSpace = command.find(' ');
	if (firstSpace == std::string::npos)
	{
		server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
		return;
	}

	size_t secondSpace = command.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
	{
		server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
		return;
	}

	// Extract target (channel or user)
	std::string target = command.substr(firstSpace + 1, secondSpace - firstSpace - 1);

	// Extract message (should start with ':')
	std::string message = command.substr(secondSpace + 1);
	if (message.empty() || message[0] != ':')
	{
		server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " PRIVMSG :Not enough parameters\r\n");
		return;
	}

	// Remove the ':' from the beginning of the message
	message = message.substr(1);

	if (message.empty())
	{
		server.sendToClient(client.GetFd(), ":server 412 " + client.getNickname() + " :No text to send\r\n");
		return;
	}

	std::cout << "PRIVMSG: [" << client.getNickname() << "] -> [" << target << "]: " << message << std::endl;

	// Check if target is a channel (starts with # or &)
	if (target[0] == '#' || target[0] == '&')
	{
		server.handleChannelMessage(client, target, message);
	}
	else
	{
		server.handleUserMessage(client, target, message);
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
