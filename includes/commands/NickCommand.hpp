#ifndef NICKCOMMAND_HPP
#define NICKCOMMAND_HPP

#include "../Command.hpp"

class NickCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "NICK"; }
    bool requiresAuth() const { return true; }
    bool requiresRegistration() const { return false; }
};

#endif