
#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"

void Cmd::handleQUIT(Server &s, Client &client, const std::string &command)
{
	std::vector<std::string> tokens = s.splitCommand(command);

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
	for (size_t i = 0; i < s.getChannels().size(); i++)
	{
		Channel *channel = s.getChannels()[i];
		if (channel->hasClient(&client))
		{
			s.broadcastToChannel(channel, fullQuitMsg, &client);
			channel->removeClient(&client);
		}
	}

	s.sendToClient(client.GetFd(), ":server ERROR :Closing Link: " + client.getNickname() + " (" + quitMsg + ")\r\n");

	close(client.GetFd());
	s.ClearClients(client.GetFd());
}