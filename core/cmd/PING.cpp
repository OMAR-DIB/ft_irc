#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

void Cmd::handlePING(Server &server, Client &client, const std::string &lineRaw)
{
    std::string line = lineRaw;
    std::string::size_type i = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
    if (i) line.erase(0, i);

    std::string::size_type sp = line.find(' ');
    if (sp == std::string::npos) {
        const std::string &srv = server.getServerName();
        server.sendToClient(client.GetFd(),
            ":" + srv + " 409 " + client.getNickname() + " :No origin specified\r\n");
        return;
    }

    std::string rest = line.substr(sp + 1);

    std::string::size_type j = 0;
    while (j < rest.size() && (rest[j] == ' ' || rest[j] == '\t')) ++j;
    if (j) rest.erase(0, j);

    if (rest.empty()) {
        const std::string &srv = server.getServerName();
        server.sendToClient(client.GetFd(),
            ":" + srv + " 409 " + client.getNickname() + " :No origin specified\r\n");
        return;
    }

   
    std::string payload;
    if (rest[0] == ':') {
        payload = rest.substr(1); 
    } else {
        std::string::size_type sp2 = rest.find(' ');
        payload = (sp2 == std::string::npos) ? rest : rest.substr(0, sp2);
    }

    const std::string &srv = server.getServerName();
    server.sendToClient(client.GetFd(),
        ":" + srv + " PONG " + srv + " :" + payload + "\r\n");
}
