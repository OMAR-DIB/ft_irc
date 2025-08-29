#ifndef MODECOMMAND_HPP
#define MODECOMMAND_HPP

#include "../Command.hpp"

class ModeCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "MODE"; }
};

#endif