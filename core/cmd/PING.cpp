
#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"

void Cmd::handlePING(Server &server,Client &client, const std::string &command)
{
	std::vector<std::string> tokens = server.splitCommand(command);

	std::cout << YEL << "PING from " << client.getNickname() << WHI << std::endl;

	// PING <server> or just PING
	std::string pongMsg = ":server PONG server";
	if (tokens.size() > 1)
	{
		pongMsg += " :" + tokens[1];
	}
	pongMsg += "\r\n";

	server.sendToClient(client.GetFd(), pongMsg);
	std::cout << GRE << "PONG sent to " << client.getNickname() << WHI << std::endl;
}