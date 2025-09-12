// Cmd_ping.cpp (final)
#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

// PING handler:
// - "PING"           -> 409 ERR_NOORIGIN
// - "PING abc"       -> PONG ... :abc
// - "PING :a b c"    -> PONG ... :a b c
// - "PING :"         -> PONG ... :
// - "PING      abc"  -> PONG ... :abc
void Cmd::handlePING(Server &server, Client &client, const std::string &lineRaw)
{
    // 1) trim leading spaces/tabs from the whole line (handles "   PING abc")
    std::string line = lineRaw;
    std::string::size_type i = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
    if (i) line.erase(0, i);

    // 2) find the first space after "PING"
    std::string::size_type sp = line.find(' ');
    if (sp == std::string::npos) {
        // no space => no parameter provided
        const std::string &srv = server.getServerName();
        server.sendToClient(client.GetFd(),
            ":" + srv + " 409 " + client.getNickname() + " :No origin specified\r\n");
        return;
    }

    // 3) take the remainder after "PING "
    std::string rest = line.substr(sp + 1);

    // 3.a) trim leading spaces/tabs before the parameter (fixes "PING      abc")
    std::string::size_type j = 0;
    while (j < rest.size() && (rest[j] == ' ' || rest[j] == '\t')) ++j;
    if (j) rest.erase(0, j);

    // if nothing left after trimming => no parameter
    if (rest.empty()) {
        const std::string &srv = server.getServerName();
        server.sendToClient(client.GetFd(),
            ":" + srv + " 409 " + client.getNickname() + " :No origin specified\r\n");
        return;
    }

    // 4) extract payload
    std::string payload;
    if (rest[0] == ':') {
        // trailing parameter (keep spaces, even empty after ':')
        payload = rest.substr(1); // may be ""
    } else {
        // non-trailing: take only the first token up to next space
        std::string::size_type sp2 = rest.find(' ');
        payload = (sp2 == std::string::npos) ? rest : rest.substr(0, sp2);
    }

    // 5) reply
    const std::string &srv = server.getServerName();
    server.sendToClient(client.GetFd(),
        ":" + srv + " PONG " + srv + " :" + payload + "\r\n");
}
