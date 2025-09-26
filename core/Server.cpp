#include "../includes/Server.hpp"
#include "../includes/Cmd.hpp"

#include <sstream>
#include <cerrno>    // errno
#include <cctype>    // std::toupper
#include <algorithm> // std::find

// If not already pulled in by headers on your platform, you may keep these includes:
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <poll.h>
// #include <signal.h>

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
    std::cout << std::endl
              << "Signal " << signum << " received! Shutting down..." << std::endl;
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

    add.sin_family = AF_INET; // IPv4
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

    // Prevent SIGPIPE on send() to closed sockets
    signal(SIGPIPE, SIG_IGN);

    ServerSocket();

    std::cout << GRE << "Server <" << SerSocketFd << "> Connected" << WHI << std::endl;
    std::cout << "Waiting to accept a connection...\n";

    while (Server::Signal == false)
    {
        // Handle poll() interruption by signals properly
        int poll_result = poll(&fds[0], fds.size(), -1);

        if (poll_result == -1)
        {
            if (errno == EINTR)
            {
                // Signal interrupted poll() - check if we should exit
                if (Server::Signal)
                {
                    std::cout << YEL << "Signal received, shutting down gracefully..." << WHI << std::endl;
                    break;
                }
                // Otherwise, continue polling
                continue;
            }
            else
            {
                // Real error occurred
                throw(std::runtime_error("poll() failed"));
            }
        }

        // Process events - but check signal status frequently
        for (size_t i = 0; i < fds.size() && !Server::Signal; i++)
        {
            const int curfd = fds[i].fd;

            // Handle error/hangup conditions first
            if (fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                if (curfd != SerSocketFd)
                {
                    std::cout << RED << "FD <" << curfd << "> error/hup, removing" << WHI << std::endl;
                    ClearClients(curfd);
                    close(curfd);
                }
                else
                {
                    std::cout << RED << "Server socket error/hup" << WHI << std::endl;
                    Server::Signal = true;
                }
                // Our fds vector likely changed; restart processing this poll cycle
                break;
            }

            if (fds[i].revents & POLLIN)
            {
                if (curfd == SerSocketFd)
                {
                    std::cout << " accept a client...\n";
                    AcceptNewClient();
                }
                else
                {
                    ReceiveNewData(curfd);
                }
                // fds can change in handlers; restart loop after handling one fd
                break;
            }
        }
    }

    std::cout << YEL << "Server shutting down..." << WHI << std::endl;
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
    char buff[1024]; // no memset; we null-terminate after recv

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

    // Find the client safely
    Client *clientPtr = NULL;
    int clientIndex = -1;

    for (size_t i = 0; i < clients.size(); i++)
    {
        if (clients[i] && clients[i]->GetFd() == fd)
        {
            clientPtr = clients[i];
            clientIndex = static_cast<int>(i);
            break;
        }
    }

    if (!clientPtr || clientIndex == -1)
    {
        std::cout << RED << "ERROR: Client with fd " << fd << " not found!" << WHI << std::endl;

        // Still remove from fds vector
        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].fd == fd)
            {
                fds.erase(fds.begin() + i);
                break;
            }
        }
        return;
    }

    std::string clientNick = clientPtr->getNickname();
    std::string clientUser = clientPtr->getUsername();

    std::cout << YEL << "Disconnecting client: " << clientNick << " (fd:" << fd << ")" << WHI << std::endl;

    // Create quit message
    std::string quitMsg = ":" + clientNick + "!" + clientUser + "@localhost QUIT :Client disconnected\r\n";

    // Collect channels that need to be cleaned up
    std::vector<Channel *> channelsToCheck;

    // Remove client from all channels and broadcast quit message
    for (size_t i = 0; i < channels.size(); i++)
    {
        Channel *channel = channels[i];
        if (!channel)
            continue; // Safety check

        if (channel->hasClient(clientPtr))
        {
            std::cout << GRE << "  -> Removing " << clientNick << " from " << channel->getName() << WHI << std::endl;

            // Broadcast quit to other channel members first
            broadcastToChannel(channel, quitMsg, clientPtr);

            // Then remove the client
            channel->removeClient(clientPtr);

            // Mark channel for checking if empty
            channelsToCheck.push_back(channel);
        }
    }

    // Check for empty channels and remove them
    for (size_t i = 0; i < channelsToCheck.size(); i++)
    {
        Channel *channel = channelsToCheck[i];
        if (channel && channel->isEmpty())
        {
            std::cout << RED << "Removing empty channel: " << channel->getName() << WHI << std::endl;
            removeChannel(channel); // Use existing safe method
        }
    }

    // Remove from fds vector
    for (size_t i = 0; i < fds.size(); i++)
    {
        if (fds[i].fd == fd)
        {
            fds.erase(fds.begin() + i);
            break;
        }
    }

    // Remove from clients vector and delete
    clients.erase(clients.begin() + clientIndex);
    delete clientPtr;

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
        cmd[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(cmd[i])));

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
    if (cmd == "PRIVMSG")
    {
        Cmd::handlePRIVMSG(*this, client, command);
    }
    else if (cmd == "JOIN")
    {
        Cmd::handleJOIN(*this, client, command);
    }
    else if (cmd == "PART")
    {
        Cmd::handlePART(*this, client, command);
    }
    else if (cmd == "QUIT")
    {
        Cmd::handleQUIT(*this, client, command);
    }
    else if (cmd == "PING")
    {
        Cmd::handlePING(*this, client, command);
    }
    else if (cmd == "INVITE")
    {
        Cmd::handleINVITE(*this, client, command);
    }
    else if (cmd == "TOPIC")
    {
        Cmd::handleTOPIC(*this, client, command);
    }
    else if (cmd == "KICK")
    {
        Cmd::handleKICK(*this, client, command);
    }
    else if (cmd == "MODE")
    {
        Cmd::handleMODE(*this, client, command);
    }
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
    // Direct send (non-blocking fd). For full compliance, consider write-queue + POLLOUT.
    ssize_t sent = send(fd, message.c_str(), message.length(), 0);
    if (sent < 0)
    {
        std::cout << RED << "Failed to send to client <" << fd << ">, errno=" << errno << WHI << std::endl;
        return;
    }
    std::cout << GRE << "Sent to client <" << fd << ">: " + message << WHI;
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

// Safer channel removal
void Server::removeChannel(Channel *channel)
{
    if (!channel)
        return;

    // Only remove if empty
    if (!channel->isEmpty())
    {
        std::cout << YEL << "Warning: Attempted to remove non-empty channel " << channel->getName() << WHI << std::endl;
        return;
    }

    // Find and remove from channels vector
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i] == channel)
        {
            std::cout << RED << "Removing empty channel: " << channel->getName() << WHI << std::endl;
            channels.erase(channels.begin() + i);
            delete channel;
            return;
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
