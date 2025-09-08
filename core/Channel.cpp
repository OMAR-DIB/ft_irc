
#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"

Channel::Channel(const std::string &channelName) : name(channelName)
{
    // Empty topic by default
    topic = "";
    
    // Initialize channel modes to default values
    inviteOnly = false;
    topicRestricted = true; // Default: only operators can change topic
    hasKey = false;
    key = "";
    hasUserLimit = false;
    userLimit = 0;
}

Channel::~Channel()
{
    // Don't delete clients - they're managed by Server
    clients.clear();
    operators.clear();
    inviteList.clear();
}

// Getters
const std::string &Channel::getName() const
{
    return name;
}

const std::string &Channel::getTopic() const
{
    return topic;
}

const std::vector<Client *> &Channel::getClients() const
{
    return clients;
}

const std::vector<Client *> &Channel::getOperators() const
{
    return operators;
}

// Setters
void Channel::setTopic(const std::string &newTopic)
{
    topic = newTopic;
}

// Client management
void Channel::addClient(Client *client)
{
    if (!hasClient(client))
    {
        clients.push_back(client);

        // First client becomes operator
        if (clients.size() == 1)
        {
            addOperator(client);
        }
    }
}
void Channel::removeClient(Client *client)
{
    if (!client)
        return;

    bool wasOperator = isOperator(client);
    int targetFd = client->GetFd();

    std::cout << YEL << "Removing client " << client->getNickname()
              << " (fd:" << targetFd << ") from channel " << name << WHI << std::endl;

    // Remove from clients list using FD comparison
    for (size_t i = 0; i < clients.size();)
    {
        if (clients[i] && clients[i]->GetFd() == targetFd)
        {
            std::cout << GRE << "  -> Found and removing client at index " << i << WHI << std::endl;
            clients.erase(clients.begin() + i);
            // Don't increment i since we removed an element
        }
        else
        {
            i++;
        }
    }

    // Remove from operators list using FD comparison
    for (size_t i = 0; i < operators.size();)
    {
        if (operators[i] && operators[i]->GetFd() == targetFd)
        {
            std::cout << GRE << "  -> Found and removing operator at index " << i << WHI << std::endl;
            operators.erase(operators.begin() + i);
            // Don't increment i since we removed an element
        }
        else
        {
            i++;
        }
    }

    std::cout << "Channel " << name << " now has " << clients.size() << " clients, "
              << operators.size() << " operators" << std::endl;

    // Auto-promote if needed
    if (wasOperator && operators.empty() && !clients.empty())
    {
        Client *newOp = clients.front();
        addOperator(newOp);
        std::cout << GRE << "Auto-promoted " << newOp->getNickname()
                  << " to operator in channel " << name << WHI << std::endl;
    }

    std::cout << "Final client list: ";
    for (size_t i = 0; i < clients.size(); i++)
    {
        std::cout << clients[i]->getNickname() << "(fd:" << clients[i]->GetFd() << ") ";
    }
    std::cout << std::endl;
}

bool Channel::hasClient(Client *client) const
{
    if (!client)
        return false;

    int targetFd = client->GetFd();

    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] && clients[i]->GetFd() == targetFd)
        {
            return true;
        }
    }
    return false;
}

bool Channel::hasClientByFd(int fd) const
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] && clients[i]->GetFd() == fd)
        {
            return true;
        }
    }
    return false;
}

// Operator management
void Channel::addOperator(Client *client)
{
    if (hasClient(client) && !isOperator(client))
    {
        operators.push_back(client);
    }
}

void Channel::removeOperator(Client *client)
{
    if (!client)
        return;
        
    int targetFd = client->GetFd();
    
    // Remove from operators list using FD comparison
    for (size_t i = 0; i < operators.size();)
    {
        if (operators[i] && operators[i]->GetFd() == targetFd)
        {
            operators.erase(operators.begin() + i);
            // Don't increment i since we removed an element
        }
        else
        {
            i++;
        }
    }
}

bool Channel::isOperator(Client *client) const
{
    if (!client)
        return false;

    int targetFd = client->GetFd();

    for (size_t i = 0; i < operators.size(); i++)
    {
        if (operators[i] && operators[i]->GetFd() == targetFd)
        {
            return true;
        }
    }
    return false;
}

// Utility
size_t Channel::getClientCount() const
{
    return clients.size();
}

bool Channel::isEmpty() const
{
    return clients.empty();
}

std::string Channel::getClientsList() const
{
    std::string list;
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (i > 0)
            list += " ";

        // Add @ prefix for operators
        if (isOperator(clients[i]))
        {
            list += "@";
        }
        list += clients[i]->getNickname();
    }
    return list;
}

// Mode management implementations
void Channel::addToInviteList(Client *client)
{
    if (client && std::find(inviteList.begin(), inviteList.end(), client) == inviteList.end())
    {
        inviteList.push_back(client);
    }
}

void Channel::removeFromInviteList(Client *client)
{
    if (client)
    {
        inviteList.erase(std::remove(inviteList.begin(), inviteList.end(), client), inviteList.end());
    }
}

bool Channel::isInvited(Client *client) const
{
    if (!client)
        return false;
    return std::find(inviteList.begin(), inviteList.end(), client) != inviteList.end();
}

// Mode getters
bool Channel::isInviteOnly() const { return inviteOnly; }
bool Channel::isTopicRestricted() const { return topicRestricted; }
bool Channel::hasChannelKey() const { return hasKey; }
const std::string &Channel::getKey() const { return key; }
bool Channel::hasChannelUserLimit() const { return hasUserLimit; }
size_t Channel::getUserLimit() const { return userLimit; }

// Mode setters
void Channel::setInviteOnly(bool value) { inviteOnly = value; }
void Channel::setTopicRestricted(bool value) { topicRestricted = value; }

void Channel::setKey(const std::string &password)
{
    hasKey = true;
    key = password;
}

void Channel::removeKey()
{
    hasKey = false;
    key = "";
}

void Channel::setUserLimit(size_t limit)
{
    hasUserLimit = true;
    userLimit = limit;
}

void Channel::removeUserLimit()
{
    hasUserLimit = false;
    userLimit = 0;
}
