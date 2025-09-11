
#include "../../includes/Cmd.hpp"      // Cmd class declaration (command handlers)
#include "../../includes/Server.hpp"   // Server: channels/clients registry + I/O helpers
#include "../../includes/Channel.hpp"  // Channel: membership & operator checks
#include "../../includes/Client.hpp"   // Client: identity (nick/user/fd/host)

// handleKICK
// -----------------------------------------------------------------------------
// Processes: KICK <channel> <nick> [ :reason with spaces ]
// Flow: validate params -> check channel -> check membership/op -> resolve target
// -> reason (robust trailing parsing) -> broadcast -> remove -> GC empty channel.
//
// POLICY kept from your code:
// - Blocks self-kick with 484 (many IRCds allow self-kick, but we keep your behavior).
// - No multi-target (#a,#b / nick1,nick2) expansion (can be added later).
//
void Cmd::handleKICK(Server &server, Client &client, const std::string &command)
{
    // ---- 1) Tokenize the head (command + mandatory params) -------------------
    // NOTE: The trailing reason is parsed from the raw line using " :".
    std::vector<std::string> tokens = server.splitCommand(command); // space-delimited tokens

    // Need at least: KICK <channel> <nick>
    if (tokens.size() < 3)
    {
        // 461 ERR_NEEDMOREPARAMS
        server.sendToClient(client.GetFd(),
            ":server 461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
        return;
    }

    // Safe because we checked size >= 3
    const std::string channelName = tokens[1];
    const std::string targetNick  = tokens[2];

    // Debug
    std::cout << YEL << "=== KICK DEBUG ===" << WHI << std::endl;
    std::cout << "Kicker: "  << client.getNickname() << " (fd:" << client.GetFd() << ")" << std::endl;
    std::cout << "Channel: " << channelName << std::endl;
    std::cout << "Target: "  << targetNick  << std::endl;

    // ---- 2) Resolve the channel ---------------------------------------------
    Channel *channel = server.findChannel(channelName);
    if (!channel)
    {
        // 403 ERR_NOSUCHCHANNEL
        server.sendToClient(client.GetFd(),
            ":server 403 " + client.getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // ---- 3) Kicker must be on the channel -----------------------------------
    if (!channel->hasClient(&client))
    {
        // 442 ERR_NOTONCHANNEL
        server.sendToClient(client.GetFd(),
            ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // ---- 4) Kicker must be an operator --------------------------------------
    if (!channel->isOperator(&client))
    {
        // 482 ERR_CHANOPRIVSNEEDED
        server.sendToClient(client.GetFd(),
            ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    // ---- 5) Resolve the target nick -----------------------------------------
    Client *targetClient = server.findClientByNickname(targetNick);
    if (!targetClient)
    {
        // 401 ERR_NOSUCHNICK
        server.sendToClient(client.GetFd(),
            ":server 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }

    // ---- 6) Target must be on the channel -----------------------------------
    if (!channel->hasClient(targetClient))
    {
        // 441 ERR_USERNOTINCHANNEL
        server.sendToClient(client.GetFd(),
            ":server 441 " + client.getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }

    // ---- 7) Block self-kick (your policy) -----------------------------------
    if (targetClient == &client)
    {
        // NOTE: 484 is often ERR_RESTRICTED in RFCs; we keep your usage here.
        server.sendToClient(client.GetFd(),
            ":server 484 " + client.getNickname() + " " + channelName + " :Cannot kick yourself\r\n");
        return;
    }

    // ---- 8) Robust trailing-parameter parsing for reason ---------------------
    // Everything after the first " :" in the raw line is one trailing parameter.
    // It may be empty (e.g., "KICK #ch nick :") or contain spaces.
    std::string kickReason;
    std::string::size_type tp = command.find(" :");
    if (tp != std::string::npos)
        kickReason = command.substr(tp + 2); // after " :"

    // Trim right (CR, LF, spaces)
    while (!kickReason.empty() &&
          (kickReason[kickReason.size() - 1] == '\r' ||
           kickReason[kickReason.size() - 1] == '\n' ||
           kickReason[kickReason.size() - 1] == ' '))
    {
        kickReason.erase(kickReason.size() - 1, 1);
    }
    // Trim left (spaces)
    while (!kickReason.empty() && kickReason[0] == ' ')
        kickReason.erase(0, 1);

    // Fallback: if reason missing or empty, use kicker's nick (your behavior)
    if (kickReason.empty())
        kickReason = client.getNickname();

    std::cout << "Kick reason: [" << kickReason << "]" << std::endl;

    // ---- 9) Build the KICK message ------------------------------------------
    // Format: :<nick>!<user>@<host> KICK <channel> <target> :<reason>\r\n
    const std::string kickMsg =
        ":" + client.getNickname() + "!" + client.getUsername() + "@localhost"
        + " KICK " + channelName + " " + targetNick + " :" + kickReason + "\r\n";

    // ---- 10) Broadcast to all channel members --------------------------------
    const std::vector<Client*> &members = channel->getClients();
    for (size_t i = 0; i < members.size(); ++i)
        server.sendToClient(members[i]->GetFd(), kickMsg);

    std::cout << GRE << "Broadcasting KICK: " << kickMsg << WHI;

    // ---- 11) Remove target from channel --------------------------------------
    channel->removeClient(targetClient);

    // ---- 12) If empty, delete the channel ------------------------------------
    if (channel->isEmpty())
    {
        server.removeChannel(channel);
        std::cout << RED << "Channel " << channelName
                  << " deleted (empty after kick)" << WHI << std::endl;
    }

    // ---- 13) Final debug ------------------------------------------------------
    std::cout << GRE << "KICK completed: " << client.getNickname()
              << " kicked " << targetNick
              << " from " << channelName
              << " (reason: " << kickReason << ")" << WHI << std::endl;
}
