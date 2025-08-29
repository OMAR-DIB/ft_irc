#ifndef USERCOMMAND_HPP
#define USERCOMMAND_HPP

#include "../Command.hpp"

class UserCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "USER"; }
    bool requiresAuth() const { return true; }
    bool requiresRegistration() const { return false; }
};

#endif