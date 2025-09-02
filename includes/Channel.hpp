#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <algorithm>

class Client;

class Channel {
private:
    std::string name;
    std::string topic;
    std::vector<Client*> clients;
    std::vector<Client*> operators;  // Channel admin with @ before name
    
public:
    Channel(const std::string& channelName);
    ~Channel();
    
    // Getters
    const std::string& getName() const ;
    const std::string& getTopic() const ;
    const std::vector<Client*>& getClients() const ;
    const std::vector<Client*>& getOperators() const ;
    
    // Setters
    void setTopic(const std::string& newTopic) ;
    
    // Client management
    void addClient(Client* client);
    void removeClient(Client* client);
    bool hasClient(Client* client) const;
    
    // Operator management  
    void addOperator(Client* client);
    void removeOperator(Client* client);
    bool isOperator(Client* client) const;
    
    // Utility
    size_t getClientCount() const ;
    bool isEmpty() const;
    std::string getClientsList() const;
};

#endif