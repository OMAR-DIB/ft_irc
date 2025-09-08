#ifndef CMD_HPP
#define CMD_HPP

#include "Client.hpp"
#include "Server.hpp"

class Cmd
{
public:
    static void handleJOIN(Client &client, const std::string &command);
    static void handlePRIVMSG(Client &client, const std::string &command);
    static void handleINVITE(Client &client, const std::string &command);
    static void handleKICK(Client &client, const std::string &command);
    static void handleQUIT(Server &s, Client &client, const std::string &command);
    static void handleTOPIC(Server &server,Client &client, const std::string &command);
    static void handlePART(Server &server, Client &client, const std::string &command);
    static void handlePING(Server &server,Client &client, const std::string &command);
};

#endif