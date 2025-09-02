#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"

Channel::Channel(const std::string &channelName) : name(channelName)
{
    // Empty topic by default
    topic = "";
}

Channel::~Channel()
{
    // Don't delete clients - they're managed by Server
    clients.clear();
    operators.clear();
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
void Channel::setTopic(const std::string &newTopic) { topic = newTopic; }

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
    // Remove from clients list
    std::vector<Client *>::iterator it = std::find(clients.begin(), clients.end(), client);
    if (it != clients.end())
    {
        clients.erase(it);
    }

    // Remove from operators list if present
    it = std::find(operators.begin(), operators.end(), client);
    if (it != operators.end())
    {
        operators.erase(it);
    }
}

bool Channel::hasClient(Client *client) const
{
    return std::find(clients.begin(), clients.end(), client) != clients.end();
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
    std::vector<Client *>::iterator it = std::find(operators.begin(), operators.end(), client);
    if (it != operators.end())
    {
        operators.erase(it);
    }
}

bool Channel::isOperator(Client *client) const
{
    return std::find(operators.begin(), operators.end(), client) != operators.end();
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
