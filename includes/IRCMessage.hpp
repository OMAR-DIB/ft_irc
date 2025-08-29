#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <vector>
#include <sstream>

class IRCMessage {
public:
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    
    IRCMessage();
    IRCMessage(const std::string& rawMessage);
    
    static IRCMessage parse(const std::string& rawMessage);
    std::string toString() const;
    
private:
    void parseMessage(const std::string& rawMessage);
};

#endif