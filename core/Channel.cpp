#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"
#include <algorithm>
#include <sstream>

Channel::Channel(const std::string& channelName) 
    : name(channelName), inviteOnly(false), topicRestricted(true), hasKey(false), userLimit(-1) {
}

Channel::~Channel() {
}

// Getters
const std::string& Channel::getName() const {
    return name;
}

const std::string& Channel::getTopic() const {
    return topic;
}

const std::string& Channel::getKey() const {
    return key;
}

const std::vector<Client*>& Channel::getMembers() const {
    return members;
}

bool Channel::isInviteOnly() const {
    return inviteOnly;
}

bool Channel::isTopicRestricted() const {
    return topicRestricted;
}

bool Channel::hasPassword() const {
    return hasKey;
}

int Channel::getUserLimit() const {
    return userLimit;
}

// Setters
void Channel::setTopic(const std::string& newTopic) {
    topic = newTopic;
}

void Channel::setKey(const std::string& newKey) {
    key = newKey;
    hasKey = !newKey.empty();
}

void Channel::setInviteOnly(bool mode) {
    inviteOnly = mode;
}

void Channel::setTopicRestricted(bool mode) {
    topicRestricted = mode;
}

void Channel::setUserLimit(int limit) {
    userLimit = limit;
}

// Member management
bool Channel::addMember(Client* client) {
    if (!client) return false;
    
    // Check if already a member
    if (isMember(client)) return false;
    
    // Check user limit
    if (userLimit > 0 && static_cast<int>(members.size()) >= userLimit) {
        return false;
    }
    
    members.push_back(client);
    client->joinChannel(this);
    
    // First member becomes operator
    if (members.size() == 1) {
        addOperator(client);
    }
    
    return true;
}

bool Channel::removeMember(Client* client) {
    if (!client) return false;
    
    std::vector<Client*>::iterator it = std::find(members.begin(), members.end(), client);
    if (it != members.end()) {
        members.erase(it);
        client->leaveChannel(this);
        
        // Remove from operators if they were one
        removeOperator(client);
        
        // Remove from invited list
        removeInvite(client);
        
        return true;
    }
    return false;
}

bool Channel::isMember(Client* client) const {
    if (!client) return false;
    return std::find(members.begin(), members.end(), client) != members.end();
}

bool Channel::isOperator(Client* client) const {
    if (!client) return false;
    return operators.find(client) != operators.end();
}

bool Channel::isInvited(Client* client) const {
    if (!client) return false;
    return invited.find(client) != invited.end();
}

// Operator management
void Channel::addOperator(Client* client) {
    if (client && isMember(client)) {
        operators.insert(client);
    }
}

void Channel::removeOperator(Client* client) {
    if (client) {
        operators.erase(client);
    }
}

// Invite management
void Channel::addInvite(Client* client) {
    if (client) {
        invited.insert(client);
    }
}

void Channel::removeInvite(Client* client) {
    if (client) {
        invited.erase(client);
    }
}

// Messaging
void Channel::broadcastMessage(const std::string& message, Client* sender) {
    for (std::vector<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        Client* client = *it;
        if (client && client != sender) {
            send(client->GetFd(), message.c_str(), message.length(), 0);
        }
    }
}

// Utility
std::string Channel::getMembersList() const {
    std::ostringstream oss;
    bool first = true;
    
    for (std::vector<Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        Client* client = *it;
        if (client) {
            if (!first) oss << " ";
            
            // Add operator prefix if they're an operator
            if (isOperator(client)) {
                oss << "@";
            }
            
            oss << client->getNickname();
            first = false;
        }
    }
    
    return oss.str();
}

bool Channel::isEmpty() const {
    return members.empty();
}