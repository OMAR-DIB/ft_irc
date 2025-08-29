#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

class Client;

class Channel {
private:
    std::string name;
    std::string topic;
    std::string key;
    std::vector<Client*> members;
    std::set<Client*> operators;
    std::set<Client*> invited;
    
    // Channel modes
    bool inviteOnly;
    bool topicRestricted;
    bool hasKey;
    int userLimit;
    
public:
    Channel(const std::string& channelName);
    ~Channel();
    
    // Getters
    const std::string& getName() const;
    const std::string& getTopic() const;
    const std::string& getKey() const;
    const std::vector<Client*>& getMembers() const;
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    bool hasPassword() const;
    int getUserLimit() const;
    
    // Setters
    void setTopic(const std::string& newTopic);
    void setKey(const std::string& newKey);
    void setInviteOnly(bool mode);
    void setTopicRestricted(bool mode);
    void setUserLimit(int limit);
    
    // Member management
    bool addMember(Client* client);
    bool removeMember(Client* client);
    bool isMember(Client* client) const;
    bool isOperator(Client* client) const;
    bool isInvited(Client* client) const;
    
    // Operator management
    void addOperator(Client* client);
    void removeOperator(Client* client);
    
    // Invite management
    void addInvite(Client* client);
    void removeInvite(Client* client);
    
    // Messaging
    void broadcastMessage(const std::string& message, Client* sender = NULL);
    
    // Utility
    std::string getMembersList() const;
    bool isEmpty() const;
};

#endif