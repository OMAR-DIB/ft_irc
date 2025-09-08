

#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"

void Cmd::handleTOPIC(Server &server,Client &client, const std::string &command)
{
	std::vector<std::string> tokens = server.splitCommand(command);

	if (tokens.size() < 2)
	{
		server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
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
		server.sendToClient(client.GetFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		std::cout << RED << "Client " << client.getNickname() << " NOT FOUND in channel!" << WHI << std::endl;
		return;
	}

	std::cout << GRE << "Client " << client.getNickname() << " IS in channel" << WHI << std::endl;

	// Rest of TOPIC logic...
	if (tokens.size() == 2)
	{
		if (channel->getTopic().empty())
		{
			server.sendToClient(client.GetFd(), ":server 331 " + client.getNickname() + " " + channelName + " :No topic is set\r\n");
		}
		else
		{
			server.sendToClient(client.GetFd(), ":server 332 " + client.getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
		}
		return;
	}

	// Check operator privileges
	if (!channel->isOperator(&client))
	{
		server.sendToClient(client.GetFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
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
		server.sendToClient(channelClients[i]->GetFd(), topicMsg);
	}

	std::cout << GRE << "Topic changed successfully" << WHI << std::endl;
}





