#include "../../includes/commands/KickCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/IRCReplies.hpp"

void KickCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (message.params.size() < 2) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
        return;
    }
    
    const std::string& channelName = message.params[0];
    const std::string& targetNick = message.params[1];
    std::string reason = message.params.size() > 2 ? message.params[2] : client->getNickname();
    
    // Get channel
    Channel* channel = server->getChannel(channelName);
    if (!channel) {
        server->sendReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }
    
    // Check if kicker is in the channel
    if (!channel->isMember(client)) {
        server->sendReply(client, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    
    // Check if kicker has operator privileges
    if (!channel->isOperator(client)) {
        server->sendReply(client, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }
    
    // Get target client
    Client* targetClient = server->getClientByNick(targetNick);
    if (!targetClient) {
        server->sendReply(client, ERR_NOSUCHNICK, targetNick + " :No such nick/channel");
        return;
    }
    
    // Check if target is in the channel
    if (!channel->isMember(targetClient)) {
        server->sendReply(client, ERR_USERNOTINCHANNEL, targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }
    
    // Create kick message
    std::string kickMsg = ":" + client->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    
    // Send kick message to all channel members
    channel->broadcastMessage(kickMsg, NULL);
    server->sendMessage(client, kickMsg);
    server->sendMessage(targetClient, kickMsg);
    
    // Remove target from channel
    channel->removeMember(targetClient);
    
    // If channel is empty, remove it
    if (channel->isEmpty()) {
        server->removeChannel(channelName);
    }
}