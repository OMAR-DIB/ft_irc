#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <vector>       //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h>  //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h>      //-> for fcntl()
#include <unistd.h>     //-> for close()
#include <arpa/inet.h>  //-> for inet_ntoa()
#include <poll.h>       //-> for poll()
#include <csignal>      //-> for signal()
//-------------------------------------------------------//

#define RED "\e[1;31m" //-> for red color
#define WHI "\e[0;37m" //-> for white color
#define GRE "\e[1;32m" //-> for green color
#define YEL "\e[1;33m" //-> for yellow color
//-------------------------------------------------------//

class Client //-> class for client
{
private:
    int Fd;            //-> client file descriptor
    std::string IPadd; //-> client ip address
    std::string buffer;           // For handling partial commands
    std::string nickname;         // Client's nickname
    std::string username;         // Client's username  
    std::string realname;         // Client's real name
    bool authenticated;           // Has client sent correct PASS?
    bool registered;              // Has client completed NICK + USER?
public:
    Client();          //-> default constructor
    ~Client();          // destructor
    
    // setter
    void SetFd(int fd);//-> setter for fd
    void setIpAdd(std::string ipadd) ; //-> setter for ipadd
    void setNickname(const std::string& nick);
    void setUsername(const std::string& user);
    void setRealname(const std::string& real);
    void setAuthenticated(bool auth);
    void setRegistered(bool reg);
    
    // getter
    int GetFd();       //-> getter for fd
    std::string getNickname() const ;
    std::string getUsername() const; 
    
    // Auth
    bool isAuthenticated() const ;
    bool isRegistered() const;


    void appendToBuffer(const std::string& data);
    std::string extractCommand();
    bool hasCompleteCommand();
};

#endif