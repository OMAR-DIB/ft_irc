#include "../../includes/Cmd.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Channel.hpp"
#include "../../includes/Client.hpp"
#include <sstream>

void Cmd::handleMODE(Server &server, Client &client, const std::string &command)
{
    std::vector<std::string> tokens = server.splitCommand(command);
    
    if (tokens.size() < 2)
    {
        server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
        return;
    }
    
    std::string target = tokens[1];
    
    // Check if target is a channel (starts with # or &)
    if (target[0] != '#' && target[0] != '&')
    {
        // User mode - not implemented for this project
        server.sendToClient(client.GetFd(), ":server 502 " + client.getNickname() + " :Cannot change mode for other users\r\n");
        return;
    }
    
    // Channel mode handling
    Channel *channel = server.findChannel(target);
    if (!channel)
    {
        server.sendToClient(client.GetFd(), ":server 403 " + client.getNickname() + " " + target + " :No such channel\r\n");
        return;
    }
    
    // Check if client is in the channel
    if (!channel->hasClient(&client))
    {
        server.sendToClient(client.GetFd(), ":server 442 " + client.getNickname() + " " + target + " :You're not on that channel\r\n");
        return;
    }
    
    // If no mode string provided, return current channel modes
    if (tokens.size() == 2)
    {
        std::string modeString = "+";
        if (channel->isInviteOnly()) modeString += "i";
        if (channel->isTopicRestricted()) modeString += "t";
        if (channel->hasChannelKey()) modeString += "k";
        if (channel->hasChannelUserLimit()) modeString += "l";
        
        server.sendToClient(client.GetFd(), ":server 324 " + client.getNickname() + " " + target + " " + modeString + "\r\n");
        return;
    }
    
    // Check if client is operator for mode changes
    if (!channel->isOperator(&client))
    {
        server.sendToClient(client.GetFd(), ":server 482 " + client.getNickname() + " " + target + " :You're not channel operator\r\n");
        return;
    }
    
    std::string modeString = tokens[2];
    std::vector<std::string> modeParams;
    
    // Collect mode parameters
    for (size_t i = 3; i < tokens.size(); i++)
    {
        modeParams.push_back(tokens[i]);
    }
    
    // Process mode string
    bool adding = true; // Default to adding modes
    size_t paramIndex = 0;
    std::string appliedModes = "";
    std::string appliedParams = "";
    
    for (size_t i = 0; i < modeString.length(); i++)
    {
        char modeChar = modeString[i];
        
        if (modeChar == '+')
        {
            adding = true;
            continue;
        }
        else if (modeChar == '-')
        {
            adding = false;
            continue;
        }
        
        // Handle specific modes
        switch (modeChar)
        {
            case 'i': // Invite-only
                channel->setInviteOnly(adding);
                appliedModes += (adding ? "+" : "-");
                appliedModes += "i";
                break;
                
            case 't': // Topic restriction
                channel->setTopicRestricted(adding);
                appliedModes += (adding ? "+" : "-");
                appliedModes += "t";
                break;
                
            case 'k': // Channel key (password)
                if (adding)
                {
                    if (paramIndex < modeParams.size())
                    {
                        channel->setKey(modeParams[paramIndex]);
                        appliedModes += "+k";
                        appliedParams += " " + modeParams[paramIndex];
                        paramIndex++;
                    }
                    else
                    {
                        server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                        return;
                    }
                }
                else
                {
                    channel->removeKey();
                    appliedModes += "-k";
                }
                break;
                
            case 'l': // User limit
                if (adding)
                {
                    if (paramIndex < modeParams.size())
                    {
                        int limit = atoi(modeParams[paramIndex].c_str());
                        if (limit > 0)
                        {
                            channel->setUserLimit(static_cast<size_t>(limit));
                            appliedModes += "+l";
                            appliedParams += " " + modeParams[paramIndex];
                        }
                        else
                        {
                            server.sendToClient(client.GetFd(), ":server 696 " + client.getNickname() + " " + target + " l * :Invalid user limit\r\n");
                            return;
                        }
                        paramIndex++;
                    }
                    else
                    {
                        server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                        return;
                    }
                }
                else
                {
                    channel->removeUserLimit();
                    appliedModes += "-l";
                }
                break;
                
            case 'o': // Operator privilege
                if (paramIndex < modeParams.size())
                {
                    Client *targetClient = server.findClientByNickname(modeParams[paramIndex]);
                    if (!targetClient)
                    {
                        server.sendToClient(client.GetFd(), ":server 401 " + client.getNickname() + " " + modeParams[paramIndex] + " :No such nick/channel\r\n");
                        return;
                    }
                    
                    if (!channel->hasClient(targetClient))
                    {
                        server.sendToClient(client.GetFd(), ":server 441 " + client.getNickname() + " " + modeParams[paramIndex] + " " + target + " :They aren't on that channel\r\n");
                        return;
                    }
                    
                    if (adding)
                    {
                        if (!channel->isOperator(targetClient))
                        {
                            channel->addOperator(targetClient);
                            appliedModes += "+o";
                            appliedParams += " " + modeParams[paramIndex];
                        }
                    }
                    else
                    {
                        if (channel->isOperator(targetClient))
                        {
                            channel->removeOperator(targetClient);
                            appliedModes += "-o";
                            appliedParams += " " + modeParams[paramIndex];
                        }
                    }
                    paramIndex++;
                }
                else
                {
                    server.sendToClient(client.GetFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters\r\n");
                    return;
                }
                break;
                
            default:
                server.sendToClient(client.GetFd(), ":server 472 " + client.getNickname() + " " + modeChar + " :is unknown mode char to me\r\n");
                return;
        }
    }
    
    // Broadcast mode change to all channel members if any modes were actually applied
    if (!appliedModes.empty())
    {
        std::string modeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost MODE " + target + " " + appliedModes + appliedParams + "\r\n";
        server.broadcastToChannel(channel, modeMsg, NULL); // Send to all members including the sender
        server.sendToClient(client.GetFd(), modeMsg);
    }
}