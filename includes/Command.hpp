#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "IRCMessage.hpp"
#include <string>

class Server;
class Client;

// Base command class
class Command {
public:
    virtual ~Command() {}
    virtual void execute(Server* server, Client* client, const IRCMessage& message) = 0;
    virtual std::string getName() const = 0;
    virtual bool requiresAuth() const { return true; }
    virtual bool requiresRegistration() const { return true; }
};

// Command factory
class CommandFactory {
public:
    static Command* createCommand(const std::string& commandName);
};

#endif