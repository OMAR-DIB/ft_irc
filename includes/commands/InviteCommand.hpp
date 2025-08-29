#ifndef INVITECOMMAND_HPP
#define INVITECOMMAND_HPP

#include "../Command.hpp"

class InviteCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "INVITE"; }
};

#endif