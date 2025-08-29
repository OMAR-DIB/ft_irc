#ifndef TOPICCOMMAND_HPP
#define TOPICCOMMAND_HPP

#include "../Command.hpp"

class TopicCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "TOPIC"; }
};

#endif