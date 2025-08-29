NAME = ircserv

SRCS =  main.cpp							\
		core/Server.cpp						\
		core/Client.cpp						\
		core/Channel.cpp					\
		core/IRCMessage.cpp					\
		core/IRCReplies.cpp					\
		core/Command.cpp					\
		core/commands/PassCommand.cpp		\
		core/commands/NickCommand.cpp		\
		core/commands/UserCommand.cpp		\
		core/commands/JoinCommand.cpp		\
		core/commands/PartCommand.cpp		\
		core/commands/PrivmsgCommand.cpp	\
		core/commands/ModeCommand.cpp		\
		core/commands/KickCommand.cpp		\
		core/commands/InviteCommand.cpp		\
		core/commands/TopicCommand.cpp

OBJDIR = objs
OBJCS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(OBJDIR): 
	mkdir -p $(OBJDIR)

$(NAME): $(OBJDIR) $(OBJCS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJCS)

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJCS)
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re