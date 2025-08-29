#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <vector>
#include <set>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>

#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class Channel;

class Client {
private:
    int Fd;
    std::string IPadd;
    std::string buffer;
    std::string nickname;
    std::string username;
    std::string realname;
    std::string hostname;
    bool authenticated;
    bool registered;
    std::set<Channel*> joinedChannels;
    
public:
    Client();
    ~Client();
    
    // Setters
    void SetFd(int fd);
    void setIpAdd(std::string ipadd);
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void setRealname(const std::string& real);
    void setHostname(const std::string& host);
    void setAuthenticated(bool auth);
    void setRegistered(bool reg);
    
    // Getters
    int GetFd() const;
    std::string getIpAdd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getRealname() const;
    std::string getHostname() const;
    bool isAuthenticated() const;
    bool isRegistered() const;
    
    // Buffer management
    void appendToBuffer(const std::string& data);
    std::string extractCommand();
    bool hasCompleteCommand() const;
    void clearBuffer();
    
    // Channel management
    void joinChannel(Channel* channel);
    void leaveChannel(Channel* channel);
    bool isInChannel(Channel* channel) const;
    const std::set<Channel*>& getJoinedChannels() const;
    
    // IRC formatting helpers
    std::string getPrefix() const;
    std::string getFullIdentifier() const;
};

#endif