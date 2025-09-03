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

    std::cout << YEL << "Removing client " << client->getNickname()
              << " (fd:" << client->GetFd() << ") from channel " << name << WHI << std::endl;

    // CRITICAL FIX: Remove from clients list using FD comparison
    for (size_t i = 0; i < clients.size();)
    {
        if (clients[i] == client)
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

    // Remove from operators list using POINTER comparison
    for (size_t i = 0; i < operators.size();)
    {
        if (operators[i] == client) // Compare pointers, not FDs
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

    // Use POINTER comparison, not FD comparison
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] == client) // Compare actual pointers
        {
            return true;
        }
    }
    return false;
}

// Alternative: Safer version using fd comparison
bool Channel::hasClientByFd(int fd) const
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i]->GetFd() == fd)
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
// void Channel::removeOperator(Client *client)
//{
//     std::vector<Client *>::iterator it = std::find(operators.begin(), operators.end(), client);
//     if (it != operators.end())
//     {
//         operators.erase(it);
//     }
//     std::cout << "Final operator list: ";
//     for (size_t i = 0; i < operators.size(); i++)
//     {
//         std::cout << operators[i]->getNickname() << "(fd:" << operators[i]->GetFd() << ") ";
//     }
//     std::cout << std::endl;
// }

bool Channel::isOperator(Client *client) const
{
    if (!client)
        return false;

    // Use POINTER comparison, not FD comparison
    for (size_t i = 0; i < operators.size(); i++)
    {
        if (operators[i] == client) // Compare actual pointers
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
