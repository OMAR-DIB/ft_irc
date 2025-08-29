#include "../../includes/commands/NickCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/IRCReplies.hpp"
#include "../../includes/Channel.hpp"

bool isValidNickname(const std::string& nick) {
    if (nick.empty() || nick.length() > 9) return false;
    
    // First character must be letter or special char
    char first = nick[0];
    if (!std::isalpha(first) && first != '[' && first != ']' && first != '{' && 
        first != '}' && first != '\\' && first != '|' && first != '_') {
        return false;
    }
    
    // Other characters can be alphanumeric, dash, or special chars
    for (size_t i = 1; i < nick.length(); i++) {
        char c = nick[i];
        if (!std::isalnum(c) && c != '-' && c != '[' && c != ']' && 
            c != '{' && c != '}' && c != '\\' && c != '|' && c != '_') {
            return false;
        }
    }
    
    return true;
}

void NickCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (message.params.empty()) {
        server->sendReply(client, ERR_NONICKNAMEGIVEN, ":No nickname given");
        return;
    }
    
    const std::string& newNick = message.params[0];
    
    if (!isValidNickname(newNick)) {
        server->sendReply(client, ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
        return;
    }
    
    // Check if nickname is already in use by another client
    if (server->isNickInUse(newNick) && server->getClientByNick(newNick) != client) {
        server->sendReply(client, ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
        return;
    }
    
    std::string oldNick = client->getNickname();
    client->setNickname(newNick);
    
    // If client is registered, notify all channels they're in about the nick change
    if (client->isRegistered() && !oldNick.empty()) {
        std::string nickChangeMsg = ":" + oldNick + "!" + client->getUsername() + "@" + 
                                   client->getHostname() + " NICK :" + newNick + "\r\n";
        
        // Send to all channels the client is in
        const std::set<Channel*>& channels = client->getJoinedChannels();
        for (std::set<Channel*>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
            (*it)->broadcastMessage(nickChangeMsg, NULL); // Send to everyone in channel
        }
        
        // Send to the client themselves
        server->sendMessage(client, nickChangeMsg);
    }
    
    // Try to complete registration if pass and user are also set
    server->tryCompleteRegistration(client);
}