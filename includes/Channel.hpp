#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <algorithm>
#include <string>
#include <vector>
class Client;
class Server;
class Channel
{
private:
    std::string name;
    std::string topic;
    std::vector<Client *> clients;
    std::vector<Client *> operators;  
    std::vector<Client *> inviteList; 

   
    bool inviteOnly;
    bool topicRestricted;
    bool hasKey;
    std::string key;
    bool hasUserLimit;
    size_t userLimit;

public:
    Channel(const std::string &channelName);
    ~Channel();

    const std::string &getName() const;
    const std::string &getTopic() const;
    const std::vector<Client *> &getClients() const;
    const std::vector<Client *> &getOperators() const;

    void setTopic(const std::string &newTopic);

    void addClient(Client *client);
    void removeClient(Client *client);
    bool hasClient(Client *client) const;
    bool hasClientByFd(int fd) const;
    void addOperator(Client *client);
    void removeOperator(Client *client);
    bool isOperator(Client *client) const;

    size_t getClientCount() const;
    bool isEmpty() const;
    std::string getClientsList() const;

    void addToInviteList(Client *client);
    void removeFromInviteList(Client *client);
    bool isInvited(Client *client) const;

    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    bool hasChannelKey() const;
    const std::string &getKey() const;
    bool hasChannelUserLimit() const;
    size_t getUserLimit() const;

    void setInviteOnly(bool value);
    void setTopicRestricted(bool value);
    void setKey(const std::string &password);
    void removeKey();
    void setUserLimit(size_t limit);
    void removeUserLimit();
};

#endif