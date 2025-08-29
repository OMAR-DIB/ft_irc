#include "../../includes/commands/ModeCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/IRCReplies.hpp"
#include <sstream>

void ModeCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (message.params.empty()) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
        return;
    }
    
    const std::string& target = message.params[0];
    
    // Check if target is a channel (starts with # or &)
    if (target[0] != '#' && target[0] != '&') {
        // User mode (not implemented for ft_irc)
        server->sendReply(client, ERR_UNKNOWNCOMMAND, "MODE :User modes not supported");
        return;
    }
    
    // Channel mode
    Channel* channel = server->getChannel(target);
    if (!channel) {
        server->sendReply(client, ERR_NOSUCHCHANNEL, target + " :No such channel");
        return;
    }
    
    // Check if client is in the channel
    if (!channel->isMember(client)) {
        server->sendReply(client, ERR_NOTONCHANNEL, target + " :You're not on that channel");
        return;
    }
    
    // If no mode string provided, return current modes
    if (message.params.size() == 1) {
        std::string modeStr = "+";
        if (channel->isInviteOnly()) modeStr += "i";
        if (channel->isTopicRestricted()) modeStr += "t";
        if (channel->hasPassword()) modeStr += "k";
        if (channel->getUserLimit() > 0) modeStr += "l";
        
        server->sendMessage(client, ":" + server->getServerName() + " 324 " + 
                           client->getNickname() + " " + target + " " + modeStr + "\r\n");
        return;
    }
    
    const std::string& modeString = message.params[1];
    std::vector<std::string> modeParams;
    for (size_t i = 2; i < message.params.size(); i++) {
        modeParams.push_back(message.params[i]);
    }
    
    // Check if client has operator privileges
    if (!channel->isOperator(client)) {
        server->sendReply(client, ERR_CHANOPRIVSNEEDED, target + " :You're not channel operator");
        return;
    }
    
    bool adding = true;
    std::string appliedModes = "";
    std::string appliedParams = "";
    size_t paramIndex = 0;
    
    for (size_t i = 0; i < modeString.length(); i++) {
        char mode = modeString[i];
        
        if (mode == '+') {
            adding = true;
            continue;
        } else if (mode == '-') {
            adding = false;
            continue;
        }
        
        switch (mode) {
            case 'i': // Invite-only
                channel->setInviteOnly(adding);
                appliedModes += (adding ? "+" : "-");
                appliedModes += "i";
                break;
                
            case 't': // Topic restricted
                channel->setTopicRestricted(adding);
                appliedModes += (adding ? "+" : "-");
                appliedModes += "t";
                break;
                
            case 'k': // Channel key (password)
                if (adding && paramIndex < modeParams.size()) {
                    channel->setKey(modeParams[paramIndex]);
                    appliedModes += "+k";
                    appliedParams += " " + modeParams[paramIndex];
                    paramIndex++;
                } else if (!adding) {
                    channel->setKey("");
                    appliedModes += "-k";
                }
                break;
                
            case 'o': // Operator privilege
                if (paramIndex < modeParams.size()) {
                    Client* targetClient = server->getClientByNick(modeParams[paramIndex]);
                    if (targetClient && channel->isMember(targetClient)) {
                        if (adding) {
                            channel->addOperator(targetClient);
                        } else {
                            channel->removeOperator(targetClient);
                        }
                        appliedModes += (adding ? "+" : "-");
                        appliedModes += "o";
                        appliedParams += " " + modeParams[paramIndex];
                    }
                    paramIndex++;
                }
                break;
                
            case 'l': // User limit
                if (adding && paramIndex < modeParams.size()) {
                    int limit = std::atoi(modeParams[paramIndex].c_str());
                    if (limit > 0) {
                        channel->setUserLimit(limit);
                        appliedModes += "+l";
                        appliedParams += " " + modeParams[paramIndex];
                    }
                    paramIndex++;
                } else if (!adding) {
                    channel->setUserLimit(-1);
                    appliedModes += "-l";
                }
                break;
                
            default:
                server->sendReply(client, ERR_UNKNOWNMODE, std::string(1, mode) + " :is unknown mode char to me");
                break;
        }
    }
    
    // Broadcast mode changes to channel if any were applied
    if (!appliedModes.empty()) {
        std::string modeMsg = ":" + client->getPrefix() + " MODE " + target + " " + appliedModes + appliedParams + "\r\n";
        channel->broadcastMessage(modeMsg, NULL);
        server->sendMessage(client, modeMsg);
    }
}