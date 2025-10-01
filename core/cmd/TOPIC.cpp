#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"

void Cmd::handleTOPIC(Server &server, Client &client, const std::string &command)
{
    const std::string &srv = server.getServerName();

    std::vector<std::string> tokens = server.splitCommand(command);
    if (tokens.size() < 2)
    {
        server.sendToClient(client.GetFd(), ":" + srv + " 461 " + client.getNickname() +
                                                " TOPIC :Not enough parameters\r\n");
        return;
    }

    const std::string channelName = tokens[1];

    Channel *channel = server.findChannel(channelName);
    if (!channel)
    {
        server.sendToClient(client.GetFd(), ":" + srv + " 403 " + client.getNickname() +
                                                " " + channelName + " :No such channel\r\n");
        return;
    }

    const std::vector<Client *> &members = channel->getClients();
    bool onChannel = false;
    for (size_t i = 0; i < members.size(); ++i)
    {
        if (members[i] && members[i]->GetFd() == client.GetFd())
        {
            onChannel = true;
            break;
        }
    }
    if (!onChannel)
    {
        server.sendToClient(client.GetFd(), ":" + srv + " 442 " + client.getNickname() +
                                                " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    if (tokens.size() == 2)
    {
        if (channel->getTopic().empty())
            server.sendToClient(client.GetFd(), ":" + srv + " 331 " + client.getNickname() +
                                                    " " + channelName + " :No topic is set\r\n");
        else
            server.sendToClient(client.GetFd(), ":" + srv + " 332 " + client.getNickname() +
                                                    " " + channelName + " :" + channel->getTopic() + "\r\n");
        return;
    }

    std::string newTopic;

    if (tokens.size() > 2)
    {
        std::string::size_type p1 = command.find(' ');
        if (p1 == std::string::npos)
        {
            server.sendToClient(client.GetFd(), ":" + srv + " 461 " + client.getNickname() +
                                                    " TOPIC :Not enough parameters\r\n");
            return;
        }

        while (p1 + 1 < command.size() && (command[p1 + 1] == ' ' || command[p1 + 1] == '\t'))
            ++p1;

        std::string::size_type p2 = command.find(' ', p1 + 1);
        if (p2 == std::string::npos)
        {
            server.sendToClient(client.GetFd(), ":" + srv + " 461 " + client.getNickname() +
                                                    " TOPIC :Not enough parameters\r\n");
            return;
        }

        std::string::size_type k = p2;
        while (k < command.size() && (command[k] == ' ' || command[k] == '\t'))
            ++k;

        if (k < command.size() && command[k] == ':')
        {
            newTopic = command.substr(k + 1);
            std::cout << GRE << "Found required colon - new topic: [" << newTopic << "]" << WHI << std::endl;
        }
        else
        {
            server.sendToClient(client.GetFd(), ":" + srv + " 461 " + client.getNickname() +
                                                    " TOPIC :Missing ':' for topic text\r\n");
            return;
        }
    }
    else
    {
        server.sendToClient(client.GetFd(), ":" + srv + " 461 " + client.getNickname() +
                                                " TOPIC :Not enough parameters\r\n");
        return;
    }

    if (channel->isTopicRestricted() && !channel->isOperator(&client))
    {
        server.sendToClient(client.GetFd(), ":" + srv + " 482 " + client.getNickname() +
                                                " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    channel->setTopic(newTopic);
    const std::string msg = ":" + client.getNickname() + "!" + client.getUsername() +
                            "@localhost TOPIC " + channelName + " :" + newTopic + "\r\n";

    std::cout << GRE << "Broadcasting TOPIC: " << msg << WHI;

    for (size_t i = 0; i < members.size(); ++i)
        server.sendToClient(members[i]->GetFd(), msg);

    std::cout << GRE << "TOPIC set by " << client.getNickname() << " in " << channelName
              << ": [" << newTopic << "]" << WHI << std::endl;
}