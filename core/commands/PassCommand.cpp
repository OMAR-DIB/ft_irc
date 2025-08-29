#include "../../includes/commands/PassCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/IRCReplies.hpp"

void PassCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (client->isRegistered()) {
        server->sendReply(client, ERR_ALREADYREGISTRED, ":Unauthorized command (already registered)");
        return;
    }
    
    if (message.params.empty()) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
        return;
    }
    
    const std::string& password = message.params[0];
    
    if (server->checkPassword(password)) {
        client->setAuthenticated(true);
        // Try to complete registration if nick and user are also set
        server->tryCompleteRegistration(client);
    } else {
        server->sendReply(client, ERR_PASSWDMISMATCH, ":Password incorrect");
    }
}