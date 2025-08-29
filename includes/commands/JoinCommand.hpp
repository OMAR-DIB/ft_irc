#ifndef JOINCOMMAND_HPP
#define JOINCOMMAND_HPP

#include "../Command.hpp"

class JoinCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "JOIN"; }
};

#endif