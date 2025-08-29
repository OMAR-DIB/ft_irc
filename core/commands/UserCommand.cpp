#include "../../includes/commands/UserCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/IRCReplies.hpp"

void UserCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (client->isRegistered()) {
        server->sendReply(client, ERR_ALREADYREGISTRED, ":Unauthorized command (already registered)");
        return;
    }
    
    if (message.params.size() < 4) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
        return;
    }
    
    const std::string& username = message.params[0];
    // const std::string& hostname = message.params[1]; // Usually ignored
    // const std::string& servername = message.params[2]; // Usually ignored
    const std::string& realname = message.params[3];
    
    (void)message.params[1]; // hostname - unused but required by protocol
    (void)message.params[2]; // servername - unused but required by protocol
    
    client->setUsername(username);
    client->setRealname(realname);
    
    // Try to complete registration if pass and nick are also set
    server->tryCompleteRegistration(client);
}