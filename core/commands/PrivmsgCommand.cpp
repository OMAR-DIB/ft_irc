#include "../../includes/commands/PrivmsgCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/IRCReplies.hpp"

void PrivmsgCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (message.params.size() < 2) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
        return;
    }
    
    const std::string& target = message.params[0];
    const std::string& text = message.params[1];
    
    if (text.empty()) {
        server->sendReply(client, ERR_NOTEXTTOSEND, ":No text to send");
        return;
    }
    
    // Check if target is a channel (starts with # or &)
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = server->getChannel(target);
        if (!channel) {
            server->sendReply(client, ERR_NOSUCHCHANNEL, target + " :No such channel");
            return;
        }
        
        if (!channel->isMember(client)) {
            server->sendReply(client, ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
            return;
        }
        
        // Create message
        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text + "\r\n";
        
        // Send to all channel members except sender
        channel->broadcastMessage(msg, client);
        
    } else {
        // Private message to user
        Client* targetClient = server->getClientByNick(target);
        if (!targetClient) {
            server->sendReply(client, ERR_NOSUCHNICK, target + " :No such nick/channel");
            return;
        }
        
        // Create message
        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text + "\r\n";
        
        // Send to target client
        server->sendMessage(targetClient, msg);
    }
}