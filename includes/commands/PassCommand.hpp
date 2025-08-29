#ifndef PASSCOMMAND_HPP
#define PASSCOMMAND_HPP

#include "../Command.hpp"

class PassCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "PASS"; }
    bool requiresAuth() const { return false; }
    bool requiresRegistration() const { return false; }
};

#endif