
#include "../includes/Client.hpp"

Client::Client()
{
    std::cout << YEL << "[client] constructor" << std::endl;
    Fd = -1;
    authenticated = false;
    registered = false;
};

Client::~Client()
{
    std::cout << YEL << "[client] destructor" << std::endl;
};

int Client::GetFd()
{
    return Fd;
}

void Client::SetFd(int fd)
{
    Fd = fd;
}

void Client::setIpAdd(std::string ipadd) 
{
    IPadd = ipadd;
}

void Client::setNickname(const std::string &nick)
{
    nickname = nick;
}

void Client::setUsername(const std::string &user)
{
    username = user;
}

void Client::setRealname(const std::string &real)
{
    realname = real;
}

void Client::setAuthenticated(bool auth)
{
    authenticated = auth;
}

void Client::setRegistered(bool reg)
{
    registered = reg;
}

std::string Client::getNickname() const
{
    return nickname;
}

std::string Client::getUsername() const
{
    return username;
}

bool Client::isAuthenticated() const
{
    return authenticated;
}

bool Client::isRegistered() const
{
    return registered;
}

void Client::appendToBuffer(const std::string &data)
{
    buffer += data;
}
bool Client::hasCompleteCommand()
{ 
    return buffer.find("\n") != std::string::npos;
}

std::string Client::extractCommand()
{
    size_t pos = buffer.find('\n');
    if (pos == std::string::npos)
    {
        return "";
    }

    std::string command = buffer.substr(0, pos);
    buffer.erase(0, pos + 1);

    if (!command.empty() && command[command.length() - 1] == '\r')
    {
        command.erase(command.length() - 1);
    }

    return command;
}
