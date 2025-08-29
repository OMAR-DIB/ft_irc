#ifndef IRCREPLIES_HPP
#define IRCREPLIES_HPP

#include <string>

// IRC Reply codes for HexChat compatibility
#define RPL_WELCOME 001
#define RPL_YOURHOST 002
#define RPL_CREATED 003
#define RPL_MYINFO 004
#define RPL_NOTOPIC 331
#define RPL_TOPIC 332
#define RPL_INVITING 341
#define RPL_NAMREPLY 353
#define RPL_ENDOFNAMES 366

// Error codes
#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHSERVER 402
#define ERR_NOSUCHCHANNEL 403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_TOOMANYCHANNELS 405
#define ERR_WASNOSUCHNICK 406
#define ERR_TOOMANYTARGETS 407
#define ERR_NORECIPIENT 411
#define ERR_NOTEXTTOSEND 412
#define ERR_UNKNOWNCOMMAND 421
#define ERR_NONICKNAMEGIVEN 431
#define ERR_ERRONEUSNICKNAME 432
#define ERR_NICKNAMEINUSE 433
#define ERR_USERNOTINCHANNEL 441
#define ERR_NOTONCHANNEL 442
#define ERR_USERONCHANNEL 443
#define ERR_NOTREGISTERED 451
#define ERR_NEEDMOREPARAMS 461
#define ERR_ALREADYREGISTRED 462
#define ERR_PASSWDMISMATCH 464
#define ERR_CHANNELISFULL 471
#define ERR_UNKNOWNMODE 472
#define ERR_INVITEONLYCHAN 473
#define ERR_BANNEDFROMCHAN 474
#define ERR_BADCHANNELKEY 475
#define ERR_NOPRIVILEGES 481
#define ERR_CHANOPRIVSNEEDED 482

class IRCReplies {
public:
    static std::string createReply(int code, const std::string& target, const std::string& message);
    static std::string createNumericReply(int code, const std::string& serverName, const std::string& target, const std::string& message);
    
    // Specific reply formatters for HexChat
    static std::string welcome(const std::string& serverName, const std::string& nick);
    static std::string yourHost(const std::string& serverName, const std::string& version);
    static std::string created(const std::string& serverName, const std::string& date);
    static std::string myInfo(const std::string& serverName, const std::string& version);
    static std::string topic(const std::string& serverName, const std::string& nick, const std::string& channel, const std::string& topic);
    static std::string noTopic(const std::string& serverName, const std::string& nick, const std::string& channel);
    static std::string namReply(const std::string& serverName, const std::string& nick, const std::string& channel, const std::string& names);
    static std::string endOfNames(const std::string& serverName, const std::string& nick, const std::string& channel);
    
    // Error replies
    static std::string noSuchNick(const std::string& serverName, const std::string& nick, const std::string& target);
    static std::string noSuchChannel(const std::string& serverName, const std::string& nick, const std::string& channel);
    static std::string notOnChannel(const std::string& serverName, const std::string& nick, const std::string& channel);
    static std::string needMoreParams(const std::string& serverName, const std::string& nick, const std::string& command);
    static std::string channelOpPrivsNeeded(const std::string& serverName, const std::string& nick, const std::string& channel);
};

#endif