#include "../../includes/commands/JoinCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/IRCReplies.hpp"

bool isValidChannelName(const std::string& name) {
    if (name.empty() || name.length() > 50) return false;
    if (name[0] != '#' && name[0] != '&') return false;
    
    // Channel names cannot contain spaces, commas, or control characters
    for (size_t i = 1; i < name.length(); i++) {
        char c = name[i];
        if (c == ' ' || c == ',' || c == '\007' || c < 32) {
            return false;
        }
    }
    
    return true;
}

void JoinCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (message.params.empty()) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
        return;
    }
    
    std::string channelNames = message.params[0];
    std::string keys = message.params.size() > 1 ? message.params[1] : "";
    
    // Parse channel names (comma-separated)
    std::vector<std::string> channels;
    std::vector<std::string> channelKeys;
    
    // Split channels by comma
    size_t pos = 0;
    while ((pos = channelNames.find(',')) != std::string::npos) {
        std::string channel = channelNames.substr(0, pos);
        if (!channel.empty()) channels.push_back(channel);
        channelNames.erase(0, pos + 1);
    }
    if (!channelNames.empty()) channels.push_back(channelNames);
    
    // Split keys by comma
    pos = 0;
    while ((pos = keys.find(',')) != std::string::npos) {
        std::string key = keys.substr(0, pos);
        channelKeys.push_back(key);
        keys.erase(0, pos + 1);
    }
    if (!keys.empty()) channelKeys.push_back(keys);
    
    // Join each channel
    for (size_t i = 0; i < channels.size(); i++) {
        const std::string& channelName = channels[i];
        std::string key = (i < channelKeys.size()) ? channelKeys[i] : "";
        
        if (!isValidChannelName(channelName)) {
            server->sendReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
            continue;
        }
        
        // Get or create channel
        Channel* channel = server->getChannel(channelName);
        if (!channel) {
            channel = server->createChannel(channelName);
        }
        
        // Check if client is already in channel
        if (channel->isMember(client)) {
            continue;
        }
        
        // Check channel modes
        if (channel->isInviteOnly() && !channel->isInvited(client)) {
            server->sendReply(client, ERR_INVITEONLYCHAN, channelName + " :Cannot join channel (+i)");
            continue;
        }
        
        if (channel->hasPassword() && channel->getKey() != key) {
            server->sendReply(client, ERR_BADCHANNELKEY, channelName + " :Cannot join channel (+k)");
            continue;
        }
        
        if (channel->getUserLimit() > 0 && 
            static_cast<int>(channel->getMembers().size()) >= channel->getUserLimit()) {
            server->sendReply(client, ERR_CHANNELISFULL, channelName + " :Cannot join channel (+l)");
            continue;
        }
        
        // Add client to channel
        if (channel->addMember(client)) {
            // Remove from invite list if they were invited
            channel->removeInvite(client);
            
            // Notify all members (including the joining client) about the join
            std::string joinMsg = ":" + client->getPrefix() + " JOIN " + channelName + "\r\n";
            channel->broadcastMessage(joinMsg, NULL); // Send to everyone including sender
            server->sendMessage(client, joinMsg); // Make sure sender gets it too
            
            // Send topic if one is set
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
            
            // Send names list
            std::string names = channel->getMembersList();
            server->sendMessage(client, IRCReplies::namReply(server->getServerName(), 
                                                           client->getNickname(), 
                                                           channelName, 
                                                           names));
            server->sendMessage(client, IRCReplies::endOfNames(server->getServerName(), 
                                                             client->getNickname(), 
                                                             channelName));
        }
    }
}