#pragma once

#include "../client/Client.hpp"
#include "../channel/Channel.hpp"

#include <poll.h>
#include <exception>
#include <string>
#include <vector>

#define MAX_CONNECTIONS 255

extern bool server_running;

class Server {
private:
	int 					_port;
	int 					_socket;
	std::string				_pass;
	std::vector<pollfd>		_pfds;
	std::vector<Channel>	_channels;
	std::vector<Client>		_clients;

	void	create_socket();
	void	bind_socket();
	void	handle_msg(int fd);
	Client	*get_client_from_fd(int fd);
	Client	*get_client_from_nickname(std::string nickname);
	Client	*get_client_from_username(std::string username);
	bool	is_on_serv(Client& client) const;
	bool	is_on_serv_username(std::string client_name) const;
	bool	is_on_serv_nickname(std::string nick) const;

	void	broadcast_to_chan(const std::vector<std::string>& user_list, std::string msg);
	int		parse_message(std::string& buffer, Client& client);

	// pseudo-lookup table for commands
	int		login_block(std::vector<std::string>& word, Client& client);
	int		four_len_block(std::vector<std::string>& word, Client& client);
	int		five_len_block(std::vector<std::string>& word, Client& client);
	int		six_len_block(std::vector<std::string>& word, Client& client);
	int		seven_len_block(std::vector<std::string>& word, Client& client);
	void	bot_block(std::vector<Channel>::iterator& it, Client& client, std::string& cmd);

	int		cmd_pass(std::vector<std::string>& params, Client& client);
	int		cmd_user(std::vector<std::string>& params, Client& client);
	int		cmd_nick(std::vector<std::string>& params, Client& client);
	int		cmd_ping(std::vector<std::string>& params, Client& client);
	int		cmd_join(std::vector<std::string>& params, Client& client);
	int		cmd_part(std::vector<std::string>& params, Client& client);
	int		cmd_kick(std::vector<std::string>& params, Client& client);
	int		cmd_mode(std::vector<std::string>& params, Client& client);
	int		cmd_topic(std::vector<std::string>& params, Client& client);
	int		cmd_invite(std::vector<std::string>& params, Client& client);
	int		cmd_privmsg(std::vector<std::string>& params, Client& client);

public:
	// Orthodox form
	Server(int port, std::string pass);
	Server(const Server &server);
	Server();
	~Server();
	Server&	operator=(const Server &server);
	
	// exceptions
	class CannotCreateSocketException : public std::exception {
	public:
		virtual const char *what() const throw();
	};
	class CannotSetSocketOptions: public std::exception {
	public:
		virtual const char *what() const throw();
	};
	class CannotSetSocketNonBlock: public std::exception {
	public:
		virtual const char *what() const throw();
	};
	class CannotBindSocket: public std::exception {
	public:
		virtual const char *what() const throw();
	};
	class CannotListen: public std::exception {
	public:
		virtual const char *what() const throw();
	};
	class InvalidPort: public std::exception {
	public:
		virtual const char *what() const throw();
	};
	class CannotPoll: public std::exception {
	public:
		virtual const char *what() const throw();
	};
	class CannotAcceptConnection: public std::exception {
	public:
		virtual const char *what() const throw();
	};
	class CannotReadMessage: public std::exception {
	public:
		virtual const char *what() const throw();
	};
	void	listen_activity();
	void	remove_client(int fd);
};
