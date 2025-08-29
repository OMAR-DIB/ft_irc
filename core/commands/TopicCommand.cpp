#include "../../includes/commands/TopicCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/IRCReplies.hpp"

void TopicCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (message.params.empty()) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
        return;
    }
    
    const std::string& channelName = message.params[0];
    
    // Get channel
    Channel* channel = server->getChannel(channelName);
    if (!channel) {
        server->sendReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }
    
    // Check if client is in the channel
    if (!channel->isMember(client)) {
        server->sendReply(client, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    
    // If no topic parameter provided, return current topic
    if (message.params.size() == 1) {
        if (!channel->getTopic().empty()) {
            server->sendMessage(client, IRCReplies::topic(server->getServerName(), 
                                                        client->getNickname(), 
                                                        channelName, 
                                                        channel->getTopic()));
        } else {
            server->sendMessage(client, IRCReplies::noTopic(server->getServerName(), 
                                                          client->getNickname(), 
                                                          channelName));
        }
        return;
    }
    
    // Setting topic
    const std::string& newTopic = message.params[1];
    
    // Check if topic is restricted to operators and if client has privileges
    if (channel->isTopicRestricted() && !channel->isOperator(client)) {
        server->sendReply(client, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }
    
    // Set new topic
    channel->setTopic(newTopic);
    
    // Broadcast topic change to all channel members
    std::string topicMsg = ":" + client->getPrefix() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    channel->broadcastMessage(topicMsg, NULL);
    server->sendMessage(client, topicMsg);
}