#include "../../includes/commands/PartCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/IRCReplies.hpp"

void PartCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (message.params.empty()) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "PART :Not enough parameters");
        return;
    }
    
    std::string channelNames = message.params[0];
    std::string reason = message.params.size() > 1 ? message.params[1] : "";
    
    // Parse channel names (comma-separated)
    std::vector<std::string> channels;
    size_t pos = 0;
    while ((pos = channelNames.find(',')) != std::string::npos) {
        std::string channel = channelNames.substr(0, pos);
        if (!channel.empty()) channels.push_back(channel);
        channelNames.erase(0, pos + 1);
    }
    if (!channelNames.empty()) channels.push_back(channelNames);
    
    // Leave each channel
    for (size_t i = 0; i < channels.size(); i++) {
        const std::string& channelName = channels[i];
        
        Channel* channel = server->getChannel(channelName);
        if (!channel) {
            server->sendReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
            continue;
        }
        
        if (!channel->isMember(client)) {
            server->sendReply(client, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
            continue;
        }
        
        // Create part message
        std::string partMsg = ":" + client->getPrefix() + " PART " + channelName;
        if (!reason.empty()) {
            partMsg += " :" + reason;
        }
        partMsg += "\r\n";
        
        // Send to all channel members including the leaving client
        channel->broadcastMessage(partMsg, NULL);
        server->sendMessage(client, partMsg);
        
        // Remove client from channel
        channel->removeMember(client);
        
        // If channel is empty, remove it
        if (channel->isEmpty()) {
            server->removeChannel(channelName);
        }
    }
}