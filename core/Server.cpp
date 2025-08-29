#include "../includes/Server.hpp"
#include "../includes/IRCReplies.hpp"
#include <cstring>
#include <ctime>

bool Server::Signal = false;

Server::Server(int port, const std::string& password) 
    : Port(port), Password(password), SerSocketFd(-1), serverName("ft_irc.42.fr") {
}

Server::~Server() {
    // Clean up channels
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        delete it->second;
    }
    channels.clear();
}

void Server::SignalHandler(int signum) {
    (void)signum;
    std::cout << std::endl << "Signal Received!" << std::endl;
    Server::Signal = true;
}

void Server::CloseFds() {
    for (size_t i = 0; i < clients.size(); i++) {
        std::cout << RED << "Client <" << clients[i].GetFd() << "> Disconnected" << WHI << std::endl;
        close(clients[i].GetFd());
    }
    if (SerSocketFd != -1) {
        std::cout << RED << "Server <" << SerSocketFd << "> Disconnected" << WHI << std::endl;
        close(SerSocketFd);
    }
}

void Server::SerSocket() {
    struct sockaddr_in add;
    struct pollfd NewPoll;
    
    add.sin_family = AF_INET;
    add.sin_port = htons(this->Port);
    add.sin_addr.s_addr = INADDR_ANY;

    SerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (SerSocketFd == -1)
        throw(std::runtime_error("failed to create socket"));

    int en = 1;
    if (setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
        throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
    if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw(std::runtime_error("failed to set option (O_NONBLOCK) on socket"));
    if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1)
        throw(std::runtime_error("failed to bind socket"));
    if (listen(SerSocketFd, SOMAXCONN) == -1)
        throw(std::runtime_error("listen() failed"));

    NewPoll.fd = SerSocketFd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;
    fds.push_back(NewPoll);
}

void Server::ServerInit() {
    SerSocket();

    std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
    std::cout << "Waiting to accept a connection...\n";

    while (Server::Signal == false) {
        if ((poll(&fds[0], fds.size(), -1) == -1) && Server::Signal == false)
            throw(std::runtime_error("poll() failed"));

        for (size_t i = 0; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == SerSocketFd)
                    AcceptNewClient();
                else
                    ReceiveNewData(fds[i].fd);
            }
        }
    }
    CloseFds();
}

void Server::AcceptNewClient() {
    Client cli;
    struct sockaddr_in cliadd;
    struct pollfd NewPoll;
    socklen_t len = sizeof(cliadd);

    int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len);
    if (incofd == -1) {
        std::cout << "accept() failed" << std::endl;
        return;
    }

    if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) {
        std::cout << "fcntl() failed" << std::endl;
        return;
    }

    NewPoll.fd = incofd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;

    cli.SetFd(incofd);
    cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
    cli.setHostname(cli.getIpAdd()); // Use IP as hostname for now
    clients.push_back(cli);
    fds.push_back(NewPoll);

    std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::ReceiveNewData(int fd) {
    char buff[1024];
    memset(buff, 0, sizeof(buff));

    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);

    if (bytes <= 0) {
        std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
        ClearClients(fd);
        close(fd);
    } else {
        buff[bytes] = '\0';
        Client* client = getClient(fd);
        if (client) {
            client->appendToBuffer(buff);
            
            // Process complete commands
            while (client->hasCompleteCommand()) {
                std::string command = client->extractCommand();
                if (!command.empty()) {
                    std::cout << YEL << "Client <" << fd << "> Data: " << WHI << command << std::endl;
                    processMessage(client, command);
                }
            }
        }
    }
}

void Server::processMessage(Client* client, const std::string& message) {
    if (!client || message.empty()) return;
    
    IRCMessage ircMsg = IRCMessage::parse(message);
    if (!ircMsg.command.empty()) {
        executeCommand(client, ircMsg);
    }
}

void Server::executeCommand(Client* client, const IRCMessage& ircMsg) {
    Command* cmd = CommandFactory::createCommand(ircMsg.command);
    
    if (!cmd) {
        sendReply(client, ERR_UNKNOWNCOMMAND, ircMsg.command + " :Unknown command");
        return;
    }
    
    // Check authentication requirements
    if (cmd->requiresAuth() && !client->isAuthenticated()) {
        sendReply(client, ERR_NOTREGISTERED, ":You have not registered");
        delete cmd;
        return;
    }
    
    // Check registration requirements
    if (cmd->requiresRegistration() && !client->isRegistered()) {
        sendReply(client, ERR_NOTREGISTERED, ":You have not registered");
        delete cmd;
        return;
    }
    
    try {
        cmd->execute(this, client, ircMsg);
    } catch (const std::exception& e) {
        std::cerr << "Command execution error: " << e.what() << std::endl;
    }
    
    delete cmd;
}

void Server::ClearClients(int fd) {
    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i].fd == fd) {
            fds.erase(fds.begin() + i);
            break;
        }
    }
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].GetFd() == fd) {
            clients.erase(clients.begin() + i);
            break;
        }
    }
}

// Client management
Client* Server::getClient(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].GetFd() == fd) {
            return &clients[i];
        }
    }
    return NULL;
}

Client* Server::getClientByNick(const std::string& nickname) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].getNickname() == nickname) {
            return &clients[i];
        }
    }
    return NULL;
}

bool Server::isNickInUse(const std::string& nickname) {
    return getClientByNick(nickname) != NULL;
}

// Channel management
Channel* Server::getChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    return (it != channels.end()) ? it->second : NULL;
}

Channel* Server::createChannel(const std::string& name) {
    if (channels.find(name) != channels.end()) {
        return channels[name];
    }
    
    Channel* channel = new Channel(name);
    channels[name] = channel;
    return channel;
}

void Server::removeChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    if (it != channels.end()) {
        delete it->second;
        channels.erase(it);
    }
}

// Utility functions
void Server::sendMessage(Client* client, const std::string& message) {
    if (!client) return;
    
    std::string fullMessage = message;
    if (fullMessage.length() < 2 || fullMessage.substr(fullMessage.length() - 2) != "\r\n") {
        fullMessage += "\r\n";
    }
    
    send(client->GetFd(), fullMessage.c_str(), fullMessage.length(), 0);
}

void Server::sendReply(Client* client, int code, const std::string& message) {
    if (!client) return;
    
    std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
    std::string reply = IRCReplies::createNumericReply(code, serverName, nick, message);
    sendMessage(client, reply);
}

std::string Server::getServerName() const {
    return serverName;
}

bool Server::checkPassword(const std::string& password) const {
    return password == Password;
}

void Server::tryCompleteRegistration(Client* client) {
    if (!client->isAuthenticated() || client->getNickname().empty() || client->getUsername().empty()) {
        return;
    }
    
    if (!client->isRegistered()) {
        client->setRegistered(true);
        sendWelcomeMessages(client);
    }
}

void Server::sendWelcomeMessages(Client* client) {
    if (!client) return;
    
    std::string nick = client->getNickname();
    
    // Send welcome sequence for HexChat compatibility
    sendMessage(client, IRCReplies::welcome(serverName, nick));
    sendMessage(client, IRCReplies::yourHost(serverName, "1.0"));
    
    time_t now = time(0);
    sendMessage(client, IRCReplies::created(serverName, ctime(&now)));
    sendMessage(client, IRCReplies::myInfo(serverName, "1.0"));
}