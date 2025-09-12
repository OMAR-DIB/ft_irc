#include "../includes/Server.hpp"
#include "../includes/Cmd.hpp"
#include <sstream>
#include <cerrno>    // errno
#include <cctype>    // std::toupper

bool Server::Signal = false; // initialize the static boolean

Server::Server()
{
    std::cout << YEL << "[Server] constructor" << std::endl;
    SerSocketFd = -1;
    _serverName = "irc.local"; // non-empty server prefix used in numerics/replies
}

Server::~Server()
{
    std::cout << YEL << "[Server] destructor" << std::endl;

    // Clean up channels
    for (size_t i = 0; i < channels.size(); i++)
        delete channels[i];
    channels.clear();

    // Clean up clients
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i])
        {
            close(clients[i]->GetFd());
            delete clients[i];
        }
    }
    clients.clear();
}

void Server::setPassword(const std::string &pass)
{
    password = pass;
}

void Server::SignalHandler(int signum)
{
    (void)signum;
    std::cout << std::endl << "Signal Received!" << std::endl;
    Server::Signal = true;
}

void Server::CloseFds()
{
    // close all clients
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i])
        {
            std::cout << RED << "Client <" << clients[i]->GetFd() << "> Disconnected" << WHI << std::endl;
            close(clients[i]->GetFd());
            delete clients[i];
        }
    }
    clients.clear();

    // close server socket
    if (SerSocketFd != -1)
    {
        std::cout << RED << "Server <" << SerSocketFd << "> Disconnected" << WHI << std::endl;
        close(SerSocketFd);
    }
}

void Server::ServerSocket()
{
    struct sockaddr_in add;
    struct pollfd NewPoll;

    add.sin_family = AF_INET;                   // IPv4
    add.sin_port = htons(this->Port);
    add.sin_addr.s_addr = INADDR_ANY;

    SerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (SerSocketFd == -1)
        throw(std::runtime_error("faild to create socket"));

    int en = 1;
    if (setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
        throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));

    if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));

    if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1)
        throw(std::runtime_error("faild to bind socket"));

    if (listen(SerSocketFd, SOMAXCONN) == -1)
        throw(std::runtime_error("listen() faild"));

    NewPoll.fd = SerSocketFd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;
    fds.push_back(NewPoll);
}

void Server::ServerInit(int port)
{
    this->Port = port;
    ServerSocket();

    std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
    std::cout << "Waiting to accept a connection...\n";

    while (Server::Signal == false)
    {
        if ((poll(&fds[0], fds.size(), -1) == -1) && Server::Signal == false)
            throw(std::runtime_error("poll() faild"));

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == SerSocketFd)
                {
                    std::cout << " accept a client...\n";
                    AcceptNewClient();
                }
                else
                {
                    ReceiveNewData(fds[i].fd);
                }
            }
            // NOTE: consider handling POLLOUT to flush write queues (non-blocking sends).
        }
    }
    CloseFds();
}

void Server::AcceptNewClient()
{
    Client *cli = new Client();
    struct sockaddr_in cliadd;
    struct pollfd NewPoll;
    socklen_t len = sizeof(cliadd);

    int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len);
    if (incofd == -1)
    {
        std::cout << "accept() failed" << std::endl;
        delete cli;
        return;
    }

    if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "fcntl() failed" << std::endl;
        close(incofd);
        delete cli;
        return;
    }

    NewPoll.fd = incofd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;

    cli->SetFd(incofd);
    cli->setIpAdd(inet_ntoa((cliadd.sin_addr)));
    clients.push_back(cli);
    fds.push_back(NewPoll);

    std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::ReceiveNewData(int fd)
{
    char buff[1024]; // no memset; we null-terminate manually after recv

    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
    if (bytes <= 0)
    {
        std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
        ClearClients(fd);
        close(fd);
        return;
    }
    buff[bytes] = '\0';
    std::cout << buff;

    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] && clients[i]->GetFd() == fd)
        {
            std::cout << "Client state - Auth: " << (clients[i]->isAuthenticated() ? "YES" : "NO")
                      << ", Registered: " << (clients[i]->isRegistered() ? "YES" : "NO")
                      << ", Nick: [" << clients[i]->getNickname() << "]" << std::endl;

            clients[i]->appendToBuffer(std::string(buff));

            while (clients[i]->hasCompleteCommand())
            {
                std::string command = clients[i]->extractCommand();
                if (!command.empty())
                    processCommand(*clients[i], command);
            }
            break;
        }
    }
}

void Server::ClearClients(int fd)
{
    std::cout << RED << "=== CLEANUP START: Client fd=" << fd << " ===" << WHI << std::endl;

    int clientIndex = -1;
    Client *clientPtr = NULL;
    std::string clientNick, clientUser;

    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] && clients[i]->GetFd() == fd)
        {
            clientIndex = i;
            clientPtr = clients[i];
            clientNick = clientPtr->getNickname();
            clientUser = clientPtr->getUsername();
            break;
        }
    }

    if (clientIndex == -1 || clientPtr == NULL)
    {
        std::cout << RED << "ERROR: Client with fd " << fd << " not found!" << WHI << std::endl;
        return;
    }

    std::cout << YEL << "Disconnecting client: " << clientNick << " (fd:" << fd << ")" << WHI << std::endl;

    std::cout << YEL << "=== CHANNELS BEFORE CLEANUP ===" << WHI << std::endl;
    for (size_t i = 0; i < channels.size(); i++)
    {
        std::cout << "Channel " << channels[i]->getName() << ":";
        const std::vector<Client *> &chClients = channels[i]->getClients();
        for (size_t j = 0; j < chClients.size(); j++)
        {
            std::cout << " " << chClients[j]->getNickname() << "(fd:" << chClients[j]->GetFd() << ")";
            if (channels[i]->isOperator(chClients[j]))
                std::cout << "@";
        }
        std::cout << std::endl;
    }

    std::string quitMsg = ":" + clientNick + "!" + clientUser + "@localhost QUIT :Client disconnected\r\n";

    std::vector<Channel *> channelsToDelete;

    for (size_t i = 0; i < channels.size(); i++)
    {
        Channel *channel = channels[i];
        std::cout << YEL << "Checking channel " << channel->getName() << WHI << std::endl;

        const std::vector<Client *> &channelClients = channel->getClients();

        for (size_t j = 0; j < channelClients.size(); j++)
        {
            if (channelClients[j] && channelClients[j]->GetFd() == fd)
            {
                std::cout << GRE << "  -> " << clientNick << " IS in " << channel->getName() << WHI << std::endl;

                broadcastToChannel(channel, quitMsg, channelClients[j]);
                channel->removeClient(channelClients[j]);
                break;
            }
        }

        if (channel->isEmpty())
        {
            std::cout << RED << "  -> Channel " << channel->getName() << " is now empty" << WHI << std::endl;
            channelsToDelete.push_back(channel);
        }
        else
        {
            std::cout << GRE << "  -> Channel " << channel->getName() << " still has "
                      << channel->getClientCount() << " clients" << WHI << std::endl;
        }
    }

    for (size_t i = 0; i < channelsToDelete.size(); i++)
    {
        std::cout << RED << "Deleting empty channel: " << channelsToDelete[i]->getName() << WHI << std::endl;

        for (size_t j = 0; j < channels.size(); j++)
        {
            if (channels[j] == channelsToDelete[i])
            {
                delete channels[j];
                channels.erase(channels.begin() + j);
                break;
            }
        }
    }

    std::cout << YEL << "=== CHANNELS AFTER CLEANUP ===" << WHI << std::endl;
    for (size_t i = 0; i < channels.size(); i++)
    {
        std::cout << "Channel " << channels[i]->getName() << ":";
        const std::vector<Client *> &chClients = channels[i]->getClients();
        for (size_t j = 0; j < chClients.size(); j++)
        {
            std::cout << " " << chClients[j]->getNickname() << "(fd:" << chClients[j]->GetFd() << ")";
            if (channels[i]->isOperator(chClients[j]))
                std::cout << "@";
        }
        std::cout << std::endl;
    }

    for (size_t i = 0; i < fds.size(); i++)
    {
        if (fds[i].fd == fd)
        {
            fds.erase(fds.begin() + i);
            break;
        }
    }

    Client *toDelete = clients[clientIndex];
    clients.erase(clients.begin() + clientIndex);
    delete toDelete;

    std::cout << GRE << "=== CLEANUP COMPLETE for " << clientNick << " ===" << WHI << std::endl;
}

void Server::processCommand(Client &client, const std::string &command)
{
    std::cout << "Processing command: [" << command << "]" << std::endl;

    std::vector<std::string> tokens = splitCommand(command);
    if (tokens.empty())
        return;

    std::string cmd = tokens[0];
    for (size_t i = 0; i < cmd.length(); i++)
        cmd[i] = std::toupper(cmd[i]);

    // allow PING before authentication
    if (!client.isAuthenticated())
    {
        if (cmd == "PASS")
        {
            handlePASS(client, command);
            return;
        }
        if (cmd == "PING")
        {
            Cmd::handlePING(*this, client, command);
            return;
        }
        sendToClient(client.GetFd(), ":" + _serverName + " 464 * :Password required\r\n");
        std::cout << RED << "Client <" << client.GetFd() << "> tried command '" << cmd << "' without authentication" << WHI << std::endl;
        return;
    }

    // allow PING before full registration too
    if (!client.isRegistered())
    {
        if (cmd == "NICK")
        {
            handleNICK(client, command);
            return;
        }
        else if (cmd == "USER")
        {
            handleUSER(client, command);
            return;
        }
        else if (cmd == "PASS")
        {
            sendToClient(client.GetFd(), ":" + _serverName + " 462 " + client.getNickname() + " :You may not reregister\r\n");
            return;
        }
        else if (cmd == "PING")
        {
            Cmd::handlePING(*this, client, command);
            return;
        }
        else
        {
            sendToClient(client.GetFd(), ":" + _serverName + " 451 * :You have not registered\r\n");
            std::cout << RED << "Client <" << client.GetFd() << "> tried command '" << cmd << "' without registration" << WHI << std::endl;
            return;
        }
    }

    // fully authenticated + registered
    if (cmd == "PRIVMSG")      { Cmd::handlePRIVMSG(*this, client, command); }
    else if (cmd == "JOIN")    { Cmd::handleJOIN(*this, client, command); }
    else if (cmd == "PART")    { Cmd::handlePART(*this, client, command); }
    else if (cmd == "QUIT")    { Cmd::handleQUIT(*this, client, command); }
    else if (cmd == "PING")    { Cmd::handlePING(*this, client, command); }
    else if (cmd == "INVITE")  { Cmd::handleINVITE(*this, client, command); }
    else if (cmd == "TOPIC")   { Cmd::handleTOPIC(*this, client, command); }
    else if (cmd == "KICK")    { Cmd::handleKICK(*this, client, command); }
    else if (cmd == "MODE")    { Cmd::handleMODE(*this, client, command); }
    else if (cmd == "PASS" || cmd == "NICK" || cmd == "USER")
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 462 " + client.getNickname() + " :You may not reregister\r\n");
    }
    else
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 421 " + client.getNickname() + " " + cmd + " :Unknown command\r\n");
    }
}

std::vector<std::string> Server::splitCommand(const std::string &command)
{
    // NOTE: simple space-splitting (trailing with spaces is lost).
    // Commands that need trailing (PING/PRIVMSG/TOPIC) should parse from the full line.
    std::vector<std::string> tokens;
    std::stringstream ss(command);
    std::string token;

    while (ss >> token)
        tokens.push_back(token);

    return tokens;
}

void Server::sendToClient(int fd, const std::string &message)
{
    // Direct send (non-blocking fd). For robustness, consider queuing + POLLOUT.
    ssize_t sent = send(fd, message.c_str(), message.length(), 0);
    if (sent < 0)
    {
        std::cout << RED << "Failed to send to client <" << fd << ">, errno=" << errno << WHI << std::endl;
        return;
    }
    std::cout << GRE << "Sent to client <" << fd << ">: " << message << WHI;
}

void Server::handlePASS(Client &client, const std::string &command)
{
    std::vector<std::string> tokens = splitCommand(command);

    if (tokens.size() < 2)
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 461 * PASS :Not enough parameters\r\n");
        return;
    }

    if (client.isAuthenticated())
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 462 * :You may not reregister\r\n");
        return;
    }

    if (tokens[1] == password)
    {
        client.setAuthenticated(true);
        std::cout << GRE << "Client <" << client.GetFd() << "> authenticated" << WHI << std::endl;
    }
    else
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 464 * :Password incorrect\r\n");
        std::cout << RED << "Client <" << client.GetFd() << "> wrong password" << WHI << std::endl;
    }
}

void Server::handleNICK(Client &client, const std::string &command)
{
    std::vector<std::string> tokens = splitCommand(command);

    if (tokens.size() < 2)
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 431 * :No nickname given\r\n");
        return;
    }

    if (!client.isAuthenticated())
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 464 * :Password required\r\n");
        return;
    }

    std::string nick = tokens[1];

    // Check if nickname is already taken
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] && clients[i]->getNickname() == nick && clients[i]->GetFd() != client.GetFd())
        {
            sendToClient(client.GetFd(), ":" + _serverName + " 433 * " + nick + " :Nickname is already in use\r\n");
            return;
        }
    }

    client.setNickname(nick);
    std::cout << GRE << "Client <" << client.GetFd() << "> set nickname: " << nick << WHI << std::endl;

    // If USER already provided, mark registered and send 001
    if (!client.getUsername().empty())
    {
        client.setRegistered(true);
        sendToClient(client.GetFd(), ":" + _serverName + " 001 " + nick + " :Welcome to the IRC Network!\r\n");
    }
}

void Server::handleUSER(Client &client, const std::string &command)
{
    std::vector<std::string> tokens = splitCommand(command);

    if (tokens.size() < 5)
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 461 * USER :Not enough parameters\r\n");
        return;
    }

    if (!client.isAuthenticated())
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 464 * :Password required\r\n");
        return;
    }

    if (client.isRegistered())
    {
        sendToClient(client.GetFd(), ":" + _serverName + " 462 * :You may not reregister\r\n");
        return;
    }

    client.setUsername(tokens[1]);

    // Realname is trailing; this simple split only takes tokens[4] w/o spaces.
    std::string realname = tokens[4];
    if (!realname.empty() && realname[0] == ':')
        realname = realname.substr(1);
    client.setRealname(realname);

    std::cout << GRE << "Client <" << client.GetFd() << "> set username: " << tokens[1] << WHI << std::endl;

    if (!client.getNickname().empty())
    {
        client.setRegistered(true);
        sendToClient(client.GetFd(), ":" + _serverName + " 001 " + client.getNickname() + " :Welcome to the IRC Network!\r\n");
    }
}

// find client by nickname
Client *Server::findClientByNickname(const std::string &nickname)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] && clients[i]->getNickname() == nickname && clients[i]->isRegistered())
            return clients[i];
    }
    return NULL;
}

// find client by fd
Client *Server::findClientByFd(int fd)
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] && clients[i]->GetFd() == fd)
            return clients[i];
    }
    return NULL;
}

// Channel management
Channel *Server::findChannel(const std::string &channelName)
{
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i]->getName() == channelName)
            return channels[i];
    }
    return NULL;
}

Channel *Server::createChannel(const std::string &channelName)
{
    Channel *channel = new Channel(channelName);
    channels.push_back(channel);
    std::cout << GRE << "Created channel: " << channelName << WHI << std::endl;
    return channel;
}

void Server::removeChannel(Channel *channel)
{
    if (channel && channel->isEmpty())
    {
        std::vector<Channel *>::iterator it = std::find(channels.begin(), channels.end(), channel);
        if (it != channels.end())
        {
            std::cout << RED << "Removing empty channel: " << channel->getName() << WHI << std::endl;
            channels.erase(it);
            delete channel;
        }
    }
}

void Server::broadcastToChannel(Channel *channel, const std::string &message, Client *sender)
{
    if (!channel)
        return;

    const std::vector<Client *> &channelClients = channel->getClients();
    for (size_t i = 0; i < channelClients.size(); i++)
    {
        if (channelClients[i] != sender)
        {
            int targetFd = channelClients[i]->GetFd();

            bool fdValid = false;
            for (size_t j = 0; j < clients.size(); j++)
            {
                if (clients[j] && clients[j]->GetFd() == targetFd)
                {
                    fdValid = true;
                    break;
                }
            }

            if (fdValid)
            {
                sendToClient(targetFd, message);
            }
            else
            {
                std::cout << RED << "Skipping broadcast to invalid fd: " << targetFd << WHI << std::endl;
            }
        }
    }
}
