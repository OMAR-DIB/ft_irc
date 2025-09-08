#ifndef CMD_HPP
#define CMD_HPP

#include "Client.hpp"
#include "Server.hpp"

class Cmd
{
public:
    static void handleJOIN(Server &server, Client &client, const std::string &command);
    static void handlePRIVMSG(Server &server, Client &client, const std::string &command);
    static void handleINVITE(Server &server, Client &client, const std::string &command);
    static void handleKICK(Server &server, Client &client, const std::string &command);
    static void handleQUIT(Server &server, Client &client, const std::string &command);
    static void handleTOPIC(Server &server, Client &client, const std::string &command);
    static void handlePART(Server &server, Client &client, const std::string &command);
    static void handlePING(Server &server, Client &client, const std::string &command);
    static void handleMODE(Server &server, Client &client, const std::string &command);
};

#endif