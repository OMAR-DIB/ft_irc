#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <vector>       
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

class Client
{
private:
    int Fd;              
    std::string IPadd;   
    std::string buffer; 
    std::string nickname;
    std::string username;
    std::string realname;
    bool authenticated; 
    bool registered;     
public:
    Client(); 
    ~Client(); 


    void SetFd(int fd);
    void setIpAdd(std::string ipadd);
    void setNickname(const std::string &nick);
    void setUsername(const std::string &user);
    void setRealname(const std::string &real);
    void setAuthenticated(bool auth);
    void setRegistered(bool reg);

    
    int GetFd();
    std::string getNickname() const;
    std::string getUsername() const;

 
    bool isAuthenticated() const;
    bool isRegistered() const;

    void appendToBuffer(const std::string &data);
    std::string extractCommand();
    bool hasCompleteCommand();
};

#endif