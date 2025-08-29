#include "../includes/IRCReplies.hpp"
#include <sstream>

std::string IRCReplies::createReply(int code, const std::string& target, const std::string& message) {
    std::ostringstream oss;
    oss << ":" << target << " " << code << " " << message << "\r\n";
    return oss.str();
}

std::string IRCReplies::createNumericReply(int code, const std::string& serverName, const std::string& target, const std::string& message) {
    std::ostringstream oss;
    oss << ":" << serverName << " ";
    if (code < 100) oss << "0";
    if (code < 10) oss << "0";
    oss << code << " " << target << " " << message << "\r\n";
    return oss.str();
}

// Welcome messages for HexChat compatibility
std::string IRCReplies::welcome(const std::string& serverName, const std::string& nick) {
    return createNumericReply(RPL_WELCOME, serverName, nick, ":Welcome to the Internet Relay Network " + nick);
}

std::string IRCReplies::yourHost(const std::string& serverName, const std::string& version) {
    return createNumericReply(RPL_YOURHOST, serverName, "", ":Your host is " + serverName + ", running version " + version);
}

std::string IRCReplies::created(const std::string& serverName, const std::string& date) {
    return createNumericReply(RPL_CREATED, serverName, "", ":This server was created " + date);
}

std::string IRCReplies::myInfo(const std::string& serverName, const std::string& version) {
    return createNumericReply(RPL_MYINFO, serverName, "", serverName + " " + version + " itkol itkol");
}

std::string IRCReplies::topic(const std::string& serverName, const std::string& nick, const std::string& channel, const std::string& topic) {
    return createNumericReply(RPL_TOPIC, serverName, nick, channel + " :" + topic);
}

std::string IRCReplies::noTopic(const std::string& serverName, const std::string& nick, const std::string& channel) {
    return createNumericReply(RPL_NOTOPIC, serverName, nick, channel + " :No topic is set");
}

std::string IRCReplies::namReply(const std::string& serverName, const std::string& nick, const std::string& channel, const std::string& names) {
    return createNumericReply(RPL_NAMREPLY, serverName, nick, "= " + channel + " :" + names);
}

std::string IRCReplies::endOfNames(const std::string& serverName, const std::string& nick, const std::string& channel) {
    return createNumericReply(RPL_ENDOFNAMES, serverName, nick, channel + " :End of NAMES list");
}

// Error replies
std::string IRCReplies::noSuchNick(const std::string& serverName, const std::string& nick, const std::string& target) {
    return createNumericReply(ERR_NOSUCHNICK, serverName, nick, target + " :No such nick/channel");
}

std::string IRCReplies::noSuchChannel(const std::string& serverName, const std::string& nick, const std::string& channel) {
    return createNumericReply(ERR_NOSUCHCHANNEL, serverName, nick, channel + " :No such channel");
}

std::string IRCReplies::notOnChannel(const std::string& serverName, const std::string& nick, const std::string& channel) {
    return createNumericReply(ERR_NOTONCHANNEL, serverName, nick, channel + " :You're not on that channel");
}

std::string IRCReplies::needMoreParams(const std::string& serverName, const std::string& nick, const std::string& command) {
    return createNumericReply(ERR_NEEDMOREPARAMS, serverName, nick, command + " :Not enough parameters");
}

std::string IRCReplies::channelOpPrivsNeeded(const std::string& serverName, const std::string& nick, const std::string& channel) {
    return createNumericReply(ERR_CHANOPRIVSNEEDED, serverName, nick, channel + " :You're not channel operator");
}