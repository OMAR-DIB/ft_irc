
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

    Channel *channel = server.findChannel(channelName);
    if (!channel)
    {
        server.sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    if (!channel->hasClient(&client))
    {
        server.sendToClient(client.GetFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    if (!channel->isOperator(&client))
    {
        server.sendToClient(client.GetFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    Client *targetClient = server.findClientByNickname(targetNick);
    if (!targetClient)
    {
        server.sendToClient(client.GetFd(), ":server 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }

    if (!channel->hasClient(targetClient))
    {
        server.sendToClient(client.GetFd(), ":server 441 " + client.getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }

    if (targetClient == &client)
    {
        server.sendToClient(client.GetFd(), ":server 484 " + client.getNickname() + " " + channelName + " :Cannot kick yourself\r\n");
        return;
    }

    std::string kickReason;
    
    if (tokens.size() > 3)
    {
        std::string::size_type p1 = command.find(' ');
        if (p1 == std::string::npos)
        {
            server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
            return;
        }
        
        while (p1 + 1 < command.size() && (command[p1 + 1] == ' ' || command[p1 + 1] == '\t')) ++p1;

        std::string::size_type p2 = command.find(' ', p1 + 1);
        if (p2 == std::string::npos)
        {
            server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
            return;
        }
        
        while (p2 + 1 < command.size() && (command[p2 + 1] == ' ' || command[p2 + 1] == '\t')) ++p2;

        std::string::size_type p3 = command.find(' ', p2 + 1);
        if (p3 == std::string::npos)
        {
            server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
            return;
        }
        
        std::string::size_type k = p3;
        while (k < command.size() && (command[k] == ' ' || command[k] == '\t')) ++k;

        if (k < command.size() && command[k] == ':')
        {
            kickReason = command.substr(k + 1);
            std::cout << GRE << "Found required colon - kick reason: [" << kickReason << "]" << WHI << std::endl;
        }
        else
        {
            server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " KICK :Missing ':' for kick reason\r\n");
            return;
        }
    }
    else
    {
        kickReason = client.getNickname(); 
        std::cout << YEL << "No kick reason provided - using default: [" << kickReason << "]" << WHI << std::endl;
    }

    std::cout << "Final kick reason: [" << kickReason << "]" << std::endl;

    std::string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + channelName + " " + targetNick + " :" + kickReason + "\r\n";

    const std::vector<Client *> &channelClients = channel->getClients();
    for (size_t i = 0; i < channelClients.size(); i++)
    {
        server.sendToClient(channelClients[i]->GetFd(), kickMsg);
    }

    std::cout << GRE << "Broadcasting KICK: " << kickMsg << WHI;

    channel->removeClient(targetClient);

    if (channel->isEmpty())
    {
        server.removeChannel(channel);
        std::cout << RED << "Channel " << channelName << " deleted (empty after kick)" << WHI << std::endl;
    }

    std::cout << GRE << "KICK completed: " << client.getNickname() << " kicked " << targetNick
              << " from " << channelName << " (reason: " << kickReason << ")" << WHI << std::endl;
}