#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <poll.h>
#include "Channel.hpp"
#include "Client.hpp"
#include "Cmd.hpp"

class Cmd;

class Server
{
private:
  int Port;
  int SerSocketFd;
  static bool Signal;

  std::vector<Client *> clients;

  std::vector<struct pollfd> fds;

  std::string password;            
  std::vector<Channel *> channels;
  std::string _serverName;

public:
 
  Server();  
  ~Server(); 


  void ServerInit(int port); 
  void ServerSocket();       

  void AcceptNewClient();      
  void ReceiveNewData(int fd); 

  static void SignalHandler(int signum);

  void CloseFds();           
  void ClearClients(int fd);

  std::vector<Channel *> getChannels() const { return channels; }

  void setPassword(const std::string &pass);                        
  void processCommand(Client &client, const std::string &command);   
  std::vector<std::string> splitCommand(const std::string &command); 
  void handlePASS(Client &client, const std::string &command);       
  void handleNICK(Client &client, const std::string &command);       
  void handleUSER(Client &client, const std::string &command);       

  void sendToClient(int fd, const std::string &message);

  Client *findClientByNickname(const std::string &nickname);
  Client *findClientByFd(int fd);

  Channel *findChannel(const std::string &channelName);
  Channel *createChannel(const std::string &channelName);
  void removeChannel(Channel *channel);
  void broadcastToChannel(Channel *channel, const std::string &message,
                          Client *sender = NULL);

  void handleChannelMessage(Client &client, const std::string &channelName,
                            const std::string &message); 
  void handleUserMessage(Client &client, const std::string &target,
                         const std::string &message); 

  const std::string &getServerName() const { return _serverName; }
  void setServerName(const std::string &name) { _serverName = name; }
};

#endif
