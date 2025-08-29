#include "../../includes/commands/InviteCommand.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/IRCReplies.hpp"

void InviteCommand::execute(Server* server, Client* client, const IRCMessage& message) {
    if (message.params.size() < 2) {
        server->sendReply(client, ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
        return;
    }
    
    const std::string& targetNick = message.params[0];
    const std::string& channelName = message.params[1];
    
    // Get target client
    Client* targetClient = server->getClientByNick(targetNick);
    if (!targetClient) {
        server->sendReply(client, ERR_NOSUCHNICK, targetNick + " :No such nick/channel");
        return;
    }
    
    // Get channel
    Channel* channel = server->getChannel(channelName);
    if (!channel) {
        server->sendReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }
    
    // Check if inviter is in the channel
    if (!channel->isMember(client)) {
        server->sendReply(client, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    
    // Check if target is already in the channel
    if (channel->isMember(targetClient)) {
        server->sendReply(client, ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
        return;
    }
    
    // Check if inviter has operator privileges (required for invite-only channels)
    if (channel->isInviteOnly() && !channel->isOperator(client)) {
        server->sendReply(client, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }
    
    // Add target to invite list
    channel->addInvite(targetClient);
    
    // Send invite message to target
    std::string inviteMsg = ":" + client->getPrefix() + " INVITE " + targetNick + " " + channelName + "\r\n";
    server->sendMessage(targetClient, inviteMsg);
    
    // Send confirmation to inviter
    server->sendMessage(client, IRCReplies::createNumericReply(RPL_INVITING, server->getServerName(), 
                                                              client->getNickname(), 
                                                              targetNick + " " + channelName));
}