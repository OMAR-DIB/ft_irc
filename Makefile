NAME = ircserv

SRCS =  main.cpp					\
		core/Server.cpp				\
		core/Client.cpp		 		\
		core/Channel.cpp			\
		core/Helper.cpp				\
		core/cmd/INVITE.cpp			\
		core/cmd/JOIN.cpp			\
		core/cmd/KICK.cpp			\
		core/cmd/MODE.cpp			\
		core/cmd/PRIVMSG.cpp		\
		core/cmd/PART.cpp			\
		core/cmd/QUIT.cpp			\
		core/cmd/TOPIC.cpp			\
		core/cmd/PING.cpp			\
		core/cmd/MODE.cpp			\


OBJDIR = objs
OBJCS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

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
