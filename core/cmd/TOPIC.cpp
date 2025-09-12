#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"

// Final TOPIC handler
void Cmd::handleTOPIC(Server &server, Client &client, const std::string &command)
{
    const std::string &srv = server.getServerName();

    // Tokenize enough to get the channel name
    std::vector<std::string> tokens = server.splitCommand(command);
    if (tokens.size() < 2)
    {
        server.sendToClient(client.GetFd(), ":" + srv + " 461 " + client.getNickname() +
                                           " TOPIC :Not enough parameters\r\n");
        return;
    }

    const std::string channelName = tokens[1];

    // Find channel
    Channel *channel = server.findChannel(channelName);
    if (!channel)
    {
        server.sendToClient(client.GetFd(), ":" + srv + " 403 " + client.getNickname() +
                                           " " + channelName + " :No such channel\r\n");
        return;
    }

    // Must be on channel
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

    // VIEW: "TOPIC <chan>"
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

    // SET: parse trailing to preserve spaces; accept without colon too
    std::string newTopic;
    {
        // Find end of "TOPIC" token
        std::string::size_type p1 = command.find(' ');
        if (p1 == std::string::npos)
        {
            server.sendToClient(client.GetFd(), ":" + srv + " 461 " + client.getNickname() +
                                               " TOPIC :Not enough parameters\r\n");
            return;
        }
        // Skip spaces/tabs after "TOPIC"
        while (p1 + 1 < command.size() && (command[p1 + 1] == ' ' || command[p1 + 1] == '\t')) ++p1;

        // Find end of channel token
        std::string::size_type p2 = command.find(' ', p1 + 1);
        if (p2 == std::string::npos) p2 = command.size();

        // Skip spaces/tabs after channel
        std::string::size_type k = p2;
        while (k < command.size() && (command[k] == ' ' || command[k] == '\t')) ++k;

        if (k < command.size() && command[k] == ':')
        {
            // Trailing: keep everything after ':', even empty, with spaces preserved
            newTopic = command.substr(k + 1);
        }
        else
        {
            // No explicit trailing; require tokens[2..]
            if (tokens.size() < 3)
            {
                server.sendToClient(client.GetFd(), ":" + srv + " 461 " + client.getNickname() +
                                                   " TOPIC :Not enough parameters\r\n");
                return;
            }
            newTopic = tokens[2];
            for (size_t i = 3; i < tokens.size(); ++i)
            {
                newTopic += " ";
                newTopic += tokens[i];
            }
            if (!newTopic.empty() && newTopic[0] == ':')
                newTopic.erase(0, 1);
        }
    }

    // Enforce +t
    if (channel->isTopicRestricted() && !channel->isOperator(&client))
    {
        server.sendToClient(client.GetFd(), ":" + srv + " 482 " + client.getNickname() +
                                           " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // Set and broadcast
    channel->setTopic(newTopic);
    const std::string msg = ":" + client.getNickname() + "!" + client.getUsername() +
                            "@localhost TOPIC " + channelName + " :" + newTopic + "\r\n";
    for (size_t i = 0; i < members.size(); ++i)
        server.sendToClient(members[i]->GetFd(), msg);
}
