#include "../includes/IRCMessage.hpp"

IRCMessage::IRCMessage() {
}

IRCMessage::IRCMessage(const std::string& rawMessage) {
    parseMessage(rawMessage);
}

IRCMessage IRCMessage::parse(const std::string& rawMessage) {
    IRCMessage msg;
    msg.parseMessage(rawMessage);
    return msg;
}

void IRCMessage::parseMessage(const std::string& rawMessage) {
    std::string line = rawMessage;
    
    // Remove trailing \r\n
    while (!line.empty() && (line[line.length() - 1] == '\r' || line[line.length() - 1] == '\n')) {
        line.erase(line.length() - 1);
    }
    
    if (line.empty()) return;
    
    std::istringstream iss(line);
    std::string token;
    
    // Check for prefix (starts with :)
    if (line[0] == ':') {
        iss >> prefix;
        prefix = prefix.substr(1); // Remove the ':' character
    }
    
    // Get command
    if (iss >> command) {
        // Convert command to uppercase
        for (size_t i = 0; i < command.length(); ++i) {
            command[i] = std::toupper(command[i]);
        }
    }
    
    // Get parameters
    std::string param;
    while (iss >> param) {
        if (param[0] == ':') {
            // This is the trailing parameter - get rest of line
            std::string trailing = param.substr(1);
            std::string rest;
            std::getline(iss, rest);
            if (!rest.empty()) {
                trailing += rest;
            }
            params.push_back(trailing);
            break;
        } else {
            params.push_back(param);
        }
    }
}

std::string IRCMessage::toString() const {
    std::string result;
    
    if (!prefix.empty()) {
        result += ":" + prefix + " ";
    }
    
    result += command;
    
    for (size_t i = 0; i < params.size(); ++i) {
        result += " ";
        if (i == params.size() - 1 && (params[i].find(' ') != std::string::npos || params[i].empty())) {
            result += ":";
        }
        result += params[i];
    }
    
    return result;
}