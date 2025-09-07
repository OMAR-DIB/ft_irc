#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"


void Cmd::handleJOIN(Server &server, Client &client, const std::string &command)
{
	std::vector<std::string> tokens = server.splitCommand(command);

	if (tokens.size() < 2)
	{
		server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " JOIN :Not enough parameters\r\n");
		return;
	}

	std::string channelName = tokens[1];

	if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
	{
		server.sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return;
	}

	std::cout << YEL << "=== JOIN DEBUG ===" << WHI << std::endl;
	std::cout << "Client " << client.getNickname() << " (fd:" << client.GetFd() << ") joining " << channelName << WHI << std::endl;

	Channel *channel = server.findChannel(channelName);
	if (!channel)
	{
		channel = server.createChannel(channelName);
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
	server.sendToClient(client.GetFd(), joinMsg);
	server.broadcastToChannel(channel, joinMsg, &client);

	// TODO: Handle channel topic when new use enter
	// Send topic if exists, otherwise tell client there's no topic
	if (!channel->getTopic().empty())
	{
		server.sendToClient(client.GetFd(), ":server 332 " + client.getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
		// optional: if you keep who/time info in channel
		// sendToClient(client.GetFd(), ":server 333 " + client.getNickname() + " " + channelName + " <setter> <timestamp>\r\n");
	}
	else
	{
		server.sendToClient(client.GetFd(), ":server 331 " + client.getNickname() + " " + channelName + " :No topic is set\r\n");
	}

	// Send names list
	server.sendToClient(client.GetFd(), ":server 353 " + client.getNickname() + " = " + channelName + " :" + channel->getClientsList() + "\r\n");
	server.sendToClient(client.GetFd(), ":server 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list\r\n");

	std::cout << GRE << "Client " << client.getNickname() << " successfully joined " << channelName
			  << " (" << channel->getClientCount() << " clients)" << WHI << std::endl;
}
