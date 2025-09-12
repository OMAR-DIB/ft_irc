#ifndef SERVER_HPP
# define SERVER_HPP

// Standard headers
# include <string>   // std::string
# include <vector>   // std::vector
# include <poll.h>   // struct pollfd, poll()

// Project headers
# include "Channel.hpp"
# include "Client.hpp"
# include "Cmd.hpp"

class Cmd; // forward declaration (already included above; harmless redundancy)

/**
 * Server
 * ------
 * Owns the listening socket, all connected clients, channels, and the single
 * poll() loop that drives non-blocking I/O (accept, read, write).
 *
 * Responsibilities:
 *  - Initialize and bind/listen a non-blocking TCP socket (v4/v6).
 *  - Maintain a single poll() (or equivalent) set for all FDs.
 *  - Accept new clients and create Client objects.
 *  - Receive/aggregate input until CRLF, then dispatch to command handlers.
 *  - Queue outgoing messages; flush only when FD is write-ready.
 *  - Manage channels (create/find/remove, broadcast).
 *  - Emit numerics/replies using the configured server name prefix.
 */
class Server
{
  private:
    int Port;                  // Listening port number
    int SerSocketFd;           // Listening socket file descriptor
    static bool Signal;        // Global flag toggled by SignalHandler (e.g., to exit cleanly)

    // All connected clients (owned pointers; ensure proper cleanup in destructor/ClearClients)
    std::vector<Client *> clients;

    // poll() set for all FDs (listening socket + every client fd)
    std::vector<struct pollfd> fds;

    std::string password;      // Connection password required by PASS
    std::vector<Channel *> channels; // All existing channels (owned pointers; cleanup on shutdown)

    // Server name used in message prefixes and numerics (":<server> <code> ...")
    // Configure via setServerName(), default in constructor (e.g., "irc.local")
    std::string _serverName;

  public:
    // Lifecycle
    Server();                  // Must set non-initialized fields to safe defaults
    ~Server();                 // Closes FDs, frees clients/channels

    // Bootstrapping
    void ServerInit(int port); // Parse/store port, prepare data structures
    void ServerSocket();       // Create/bind/listen non-blocking socket (sets SerSocketFd)

    // Event-loop actions (driven by a single poll() in your main server loop)
    void AcceptNewClient();    // Accept new connection when listening FD is readable
    void ReceiveNewData(int fd); // Read from 'fd', buffer data, dispatch complete lines

    // Signals
    static void SignalHandler(int signum); // Set Signal=true and handle graceful shutdown

    // Cleanup helpers
    void CloseFds();           // Close listening + all client FDs
    void ClearClients(int fd); // Remove a client (by fd), free object, shrink poll set

    // Channels accessor (returns a *copy* of the vector; consider const ref if needed)
    std::vector<Channel *> getChannels() const { return channels; }

    // Authentication / Parsing / Dispatch
    void setPassword(const std::string &pass);               // Set PASS requirement
    void processCommand(Client &client, const std::string &command); // Dispatch one IRC line
    std::vector<std::string> splitCommand(const std::string &command); // Basic tokenization
    void handlePASS(Client &client, const std::string &command); // PASS handler
    void handleNICK(Client &client, const std::string &command); // NICK handler
    void handleUSER(Client &client, const std::string &command); // USER handler

    // Outbound message enqueue (actual send() must happen in poll write-ready branch)
    void sendToClient(int fd, const std::string &message);

    // Client lookup helpers
    Client *findClientByNickname(const std::string &nickname);
    Client *findClientByFd(int fd);

    // Channel management
    Channel *findChannel(const std::string &channelName);
    Channel *createChannel(const std::string &channelName);
    void removeChannel(Channel *channel);
    void broadcastToChannel(Channel *channel, const std::string &message,
                            Client *sender = NULL); // send to all members except sender (if non-NULL)

    // Message routing
    void handleChannelMessage(Client &client, const std::string &channelName,
                              const std::string &message); // PRIVMSG/NOTICE to channel
    void handleUserMessage(Client &client, const std::string &target,
                           const std::string &message);    // PRIVMSG/NOTICE to user

    // Server name prefix accessors (used in numerics/replies like 001, 409, PONG, etc.)
    const std::string &getServerName() const { return _serverName; }
    void setServerName(const std::string &name) { _serverName = name; }
};

#endif // SERVER_HPP
