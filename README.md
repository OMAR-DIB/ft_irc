# ft_irc - Modular IRC Server

A complete IRC server implementation in C++98, designed for HexChat compatibility with a highly modular architecture.

## 🏗️ Architecture

### Core Components
- **Server**: Main server with port/password support and client/channel management
- **Client**: Enhanced client class with buffer management and channel tracking  
- **Channel**: Complete channel implementation with modes and operator support
- **IRCMessage**: IRC protocol message parser
- **IRCReplies**: HexChat-compatible reply formatter

### Command System
- **Modular Design**: Each command is implemented as a separate class
- **Factory Pattern**: Easy addition of new commands
- **Authentication**: PASS, NICK, USER
- **Channel Operations**: JOIN, PART, PRIVMSG
- **Operator Commands**: KICK, INVITE, TOPIC, MODE

## 🚀 Build & Usage

```bash
# Build the server
make

# Run the server
./ircserv <port> <password>
# Example:
./ircserv 6667 mypass123

# Clean build files
make clean      # Remove object files
make fclean     # Remove everything
make re         # Rebuild from scratch
```

## 🔧 Supported IRC Commands

### Authentication
- `PASS <password>` - Server password
- `NICK <nickname>` - Set nickname  
- `USER <username> <hostname> <servername> <realname>` - Set user info

### Channel Operations
- `JOIN <channel>[,<channel>] [key[,key]]` - Join channel(s)
- `PART <channel>[,<channel>] [reason]` - Leave channel(s)
- `PRIVMSG <target> <message>` - Send message to channel or user

### Operator Commands
- `KICK <channel> <nick> [reason]` - Kick user from channel
- `INVITE <nick> <channel>` - Invite user to channel
- `TOPIC <channel> [topic]` - View/set channel topic
- `MODE <channel> [modes] [params]` - View/set channel modes

### Channel Modes
- `i` - Invite-only channel
- `t` - Topic restricted to operators
- `k` - Channel key (password)
- `o` - Give/take operator privilege  
- `l` - Set/remove user limit

## 🧪 Testing

### With the included test client:
```bash
# Start server in one terminal
./ircserv 6668 mypass123

# Test in another terminal
python3 test_client.py 6668 mypass123
```

### With HexChat:
1. Start the server: `./ircserv 6667 password123`
2. In HexChat:
   - Server: `localhost`
   - Port: `6667`
   - Password: `password123`
   - Connect and test all functionality

### With netcat (basic test):
```bash
nc localhost 6667
PASS password123
NICK testuser
USER testuser 0 * :Test User
JOIN #test
PRIVMSG #test :Hello World!
```

## 📁 Project Structure

```
ft_irc/
├── main.cpp                    # Entry point
├── includes/                   # Header files
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   ├── IRCMessage.hpp
│   ├── IRCReplies.hpp
│   ├── Command.hpp
│   └── commands/              # Individual command headers
├── core/                      # Implementation files
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── IRCMessage.cpp
│   ├── IRCReplies.cpp
│   ├── Command.cpp
│   └── commands/              # Command implementations
├── Makefile
├── test_client.py             # Test client
└── README.md
```

## 🔧 Adding New Commands

1. Create header in `includes/commands/NewCommand.hpp`:
```cpp
#ifndef NEWCOMMAND_HPP
#define NEWCOMMAND_HPP
#include "../Command.hpp"

class NewCommand : public Command {
public:
    void execute(Server* server, Client* client, const IRCMessage& message);
    std::string getName() const { return "NEWCOMM"; }
};
#endif
```

2. Implement in `core/commands/NewCommand.cpp`:
```cpp
#include "../../includes/commands/NewCommand.hpp"
#include "../../includes/Server.hpp"
// Implementation here
```

3. Add to `core/Command.cpp`:
```cpp
#include "../includes/commands/NewCommand.hpp"
// In createCommand():
else if (commandName == "NEWCOMM") {
    return new NewCommand();
}
```

4. Add to Makefile SRCS list
5. Rebuild: `make re`

## 🐛 Troubleshooting

### "Failed to bind socket"
- Port may be in use: `ss -tlnp | grep :PORT`
- Try a different port: `./ircserv 6668 mypass`
- Kill existing processes: `pkill -f ircserv`

### Connection Issues
- Check firewall settings
- Ensure server is running before connecting
- Verify correct port and password

### Build Errors
- Ensure C++98 compliance
- Check all source files are listed in Makefile
- Use `make clean && make` for clean rebuild

## 📋 ft_irc Project Requirements

✅ **All Mandatory Requirements Met:**
- C++98 standard compliance
- Non-blocking I/O with poll()
- Multiple client support
- No forking
- Authentication (PASS)
- Registration (NICK + USER)
- Channel operations (JOIN, PRIVMSG)
- Operator commands (KICK, INVITE, TOPIC, MODE)
- Channel modes (i, t, k, o, l)
- HexChat compatibility

The server is ready for evaluation and can be easily extended with additional features.