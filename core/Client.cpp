#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"

Client::Client() {
    std::cout << YEL << "[client] constructor" << std::endl;
    Fd = -1;
    authenticated = false;
    registered = false;
    hostname = "localhost";
}

Client::~Client() {
    std::cout << YEL << "[client] destructor" << std::endl;
    // Leave all channels when client disconnects
    std::set<Channel*> channelsCopy = joinedChannels;
    for (std::set<Channel*>::iterator it = channelsCopy.begin(); it != channelsCopy.end(); ++it) {
        (*it)->removeMember(this);
    }
}

// Setters
void Client::SetFd(int fd) {
    Fd = fd;
}

void Client::setIpAdd(std::string ipadd) {
    IPadd = ipadd;
}

void Client::setNickname(const std::string& nick) {
    nickname = nick;
}

void Client::setUsername(const std::string& user) {
    username = user;
}

void Client::setRealname(const std::string& real) {
    realname = real;
}

void Client::setHostname(const std::string& host) {
    hostname = host;
}

void Client::setAuthenticated(bool auth) {
    authenticated = auth;
}

void Client::setRegistered(bool reg) {
    registered = reg;
}

// Getters
int Client::GetFd() const {
    return Fd;
}

std::string Client::getIpAdd() const {
    return IPadd;
}

std::string Client::getNickname() const {
    return nickname;
}

std::string Client::getUsername() const {
    return username;
}

std::string Client::getRealname() const {
    return realname;
}

std::string Client::getHostname() const {
    return hostname;
}

bool Client::isAuthenticated() const {
    return authenticated;
}

bool Client::isRegistered() const {
    return registered;
}

// Buffer management
void Client::appendToBuffer(const std::string& data) {
    buffer += data;
}

std::string Client::extractCommand() {
    size_t pos = buffer.find("\r\n");
    if (pos == std::string::npos) {
        pos = buffer.find("\n");
        if (pos == std::string::npos) {
            return "";
        }
    }
    
    std::string command = buffer.substr(0, pos);
    buffer.erase(0, pos + (buffer[pos] == '\r' ? 2 : 1));
    return command;
}

bool Client::hasCompleteCommand() const {
    return buffer.find("\r\n") != std::string::npos || buffer.find("\n") != std::string::npos;
}

void Client::clearBuffer() {
    buffer.clear();
}

// Channel management
void Client::joinChannel(Channel* channel) {
    if (channel) {
        joinedChannels.insert(channel);
    }
}

void Client::leaveChannel(Channel* channel) {
    if (channel) {
        joinedChannels.erase(channel);
    }
}

bool Client::isInChannel(Channel* channel) const {
    return joinedChannels.find(channel) != joinedChannels.end();
}

const std::set<Channel*>& Client::getJoinedChannels() const {
    return joinedChannels;
}

// IRC formatting helpers
std::string Client::getPrefix() const {
    return nickname + "!" + username + "@" + hostname;
}

std::string Client::getFullIdentifier() const {
    return ":" + getPrefix();
}