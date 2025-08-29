#include "../includes/Command.hpp"
#include "../includes/commands/PassCommand.hpp"
#include "../includes/commands/NickCommand.hpp"
#include "../includes/commands/UserCommand.hpp"
#include "../includes/commands/JoinCommand.hpp"
#include "../includes/commands/PartCommand.hpp"
#include "../includes/commands/PrivmsgCommand.hpp"
#include "../includes/commands/ModeCommand.hpp"
#include "../includes/commands/KickCommand.hpp"
#include "../includes/commands/InviteCommand.hpp"
#include "../includes/commands/TopicCommand.hpp"

Command* CommandFactory::createCommand(const std::string& commandName) {
    if (commandName == "PASS") {
        return new PassCommand();
    } else if (commandName == "NICK") {
        return new NickCommand();
    } else if (commandName == "USER") {
        return new UserCommand();
    } else if (commandName == "JOIN") {
        return new JoinCommand();
    } else if (commandName == "PART") {
        return new PartCommand();
    } else if (commandName == "PRIVMSG") {
        return new PrivmsgCommand();
    } else if (commandName == "MODE") {
        return new ModeCommand();
    } else if (commandName == "KICK") {
        return new KickCommand();
    } else if (commandName == "INVITE") {
        return new InviteCommand();
    } else if (commandName == "TOPIC") {
        return new TopicCommand();
    }
    
    return NULL; // Unknown command
}