#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <algorithm>
# include <string>
# include <vector>
class	Client;
class	Server;
class Channel
{
  private:
	std::string name;
	std::string topic;
	std::vector<Client *> clients;
	std::vector<Client *> operators;  // Channel admin with @ before name
	std::vector<Client *> inviteList; // For invite-only mode

	// Channel modes
	bool inviteOnly;      // i: Set/remove Invite-only channel
	bool topicRestricted;
		// t: Set/remove restrictions of TOPIC command to channel operators
	bool hasKey;          // k: Set/remove the channel key (password)
	std::string key;      // The actual password for +k mode
	bool hasUserLimit;    // l: Set/remove the user limit to channel
	size_t userLimit;     // The actual user limit for +l mode

  public:
	Channel(const std::string &channelName);
	~Channel();

	// Getters
	const std::string &getName() const;
	const std::string &getTopic() const;
	const std::vector<Client *> &getClients() const;
	const std::vector<Client *> &getOperators() const;

	// Setters
	void setTopic(const std::string &newTopic);

	// Client management
	void addClient(Client *client);
	void removeClient(Client *client);
	bool hasClient(Client *client) const;
	bool hasClientByFd(int fd) const;
	// Operator management
	void addOperator(Client *client);
	void removeOperator(Client *client);
	bool isOperator(Client *client) const;

	// Utility
	size_t getClientCount() const;
	bool isEmpty() const;
	std::string getClientsList() const;

	// Mode management
	void addToInviteList(Client *client);
	void removeFromInviteList(Client *client);
	bool isInvited(Client *client) const;

	// Mode getters
	bool isInviteOnly() const;
	bool isTopicRestricted() const;
	bool hasChannelKey() const;
	const std::string &getKey() const;
	bool hasChannelUserLimit() const;
	size_t getUserLimit() const;

	// Mode setters
	void setInviteOnly(bool value);
	void setTopicRestricted(bool value);
	void setKey(const std::string &password);
	void removeKey();
	void setUserLimit(size_t limit);
	void removeUserLimit();
};

#endif