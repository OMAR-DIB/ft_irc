#ifndef KICKCOMMAND_HPP
#define KICKCOMMAND_HPP

#include "../Command.hpp"

class KickCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "KICK"; }
};

#endif