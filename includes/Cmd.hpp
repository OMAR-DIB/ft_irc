#ifndef CMD_HPP
#define CMD_HPP

#include "Client.hpp"
#include "Server.hpp"

class Cmd
{
public:
    static void handleJOIN(Server &s, Client &client, const std::string &command);
    static void handlePRIVMSG(Server &s, Client &client, const std::string &command);
    static void handleINVITE(Server &s, Client &client, const std::string &command);
    static void handleKICK(Server &s, Client &client, const std::string &command);
    static void handleQUIT(Server &s, Client &client, const std::string &command);
    static void handleTOPIC(Server &server, Client &client, const std::string &command);
    static void handlePART(Server &server, Client &client, const std::string &command);
    static void handlePING(Server &server, Client &client, const std::string &command);
};

#endif