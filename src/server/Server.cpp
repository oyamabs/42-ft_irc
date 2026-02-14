#include <csignal>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "Server.hpp"
#include "../utils/IrcReplies.hpp"
#include "../logger/Logger.hpp"

void Server::create_socket()
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
		throw CannotCreateSocketException();

	// Set options for the socket (mainly make it force to lock an address)
	int opt_val = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)))
		throw CannotSetSocketOptions();

	// set the socket as non blocking
	if (fcntl(_socket, F_SETFL, SOCK_NONBLOCK))
		throw CannotSetSocketNonBlock();
}

void Server::bind_socket()
{
	struct sockaddr_in sockaddress = {};
	sockaddress.sin_addr.s_addr = INADDR_ANY;
	sockaddress.sin_port = htons(_port);
	sockaddress.sin_family = AF_INET;

	// bind the socket to an address and port
	if (bind(_socket, (sockaddr *)&sockaddress, sizeof(sockaddress)) < 0)
		throw CannotBindSocket();
}

void Server::handle_msg(int fd)
{
	Client &author = *get_client_from_fd(fd);
	std::string &message = author.get_message_buffer();
	char buffer[100] = { 0 };
	ssize_t bytes_read = recv(fd, buffer, 99, 0);
	if ((bytes_read < 0) and (errno != EWOULDBLOCK))
	{
		remove_client(fd);
		throw CannotReadMessage();
	}
	// client disconnected
	if (bytes_read == 0)
	{
		remove_client(fd);
		return ;
	}
	message.append(buffer);
	size_t cursor = message.find("\r\n");
	while (cursor != message.npos)
	{
		author.add_to_queue(message.substr(0, cursor));
		message.erase(0, cursor + 2);
		cursor = message.find("\r\n");
	}
	while (author.get_message_queue().size())
	{
		std::string	temp = author.get_queue_front();
		std::stringstream	ss;
		ss << "client " << author.get_fd() << ":"<< temp;
		Logger::log(DEBUG, ss.str().c_str());
		if (parse_message(temp, author))
			break ;

		// send buffer back to client for debug
		// Logger::log(DEBUG, temp.c_str());
		// message.clear();
	}
}

Client *Server::get_client_from_fd(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (fd == _clients[i].get_fd())
			return &_clients[i];
	}
	return (NULL);
}

Client *Server::get_client_from_nickname(std::string nickname)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (nickname == _clients[i].get_nickname())
			return &_clients[i];
	}
	return (NULL);
}

Client *Server::get_client_from_username(std::string username)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (username == _clients[i].get_username())
			return &_clients[i];
	}
	return (NULL);
}

bool	Server::is_on_serv(Client& client) const
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (client.get_username() == _clients[i].get_username())
			return (true);
	}
	return (false);
}

bool	Server::is_on_serv_username(std::string client_username) const
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (client_username == _clients[i].get_username())
			return (true);
	}
	return (false);
}

bool	Server::is_on_serv_nickname(std::string nick) const
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (nick == _clients[i].get_nickname())
			return (true);
	}
	return (false);
}

void	Server::broadcast_to_chan(const std::vector<std::string>& user_list, std::string msg)
{
	for (std::vector<std::string>::const_iterator it = user_list.begin(); it != user_list.end(); it++)
	{
		Client	current = *get_client_from_username(*it);
		current.send_message(msg);
	}
}

/*
	For each command, the required parameter are commented right on top of it
	anything in parenthesis is optional / situational
*/
int		Server::parse_message(std::string& buffer, Client& client)
{
	std::vector<std::string>	word;
	std::stringstream			ss(buffer);
	std::string 				msg = ss.str();

	// pre-allocate to speed-up vect processing
	word.reserve(5);

	// operator >> eat spaces and backslashes characters
	for(std::string w; ss >> w && (w.find(":") == w.npos); )
		word.push_back(w);

	if (msg.find(":") != msg.npos)
		word.push_back(msg.substr(msg.find(":")+1));

	// Debug prints yo
	// for(size_t i = 0; i < word.size(); i++)
	// 	std::cout << word[i] << std::endl;
	
	if (word.size() == 0)
		return (0);	// no message

	// unlogged commands
	// if (word[0] == "CAP")
	// {
	// 	if (word[1] == "LS")
	// 		client.send_message("CAP * LS :");
	// 	return ;
	// }
	if (word[0] == "QUIT")
	{
		// TODO: Add a print, (user leaving (disconnected))
		// same fashion as PART to cleanup + manage leaving message
		// currently QUIT is a part + disconnect, they can't be distinguished
		remove_client(client.get_fd());
		return (0);
	}

	int	ret = -1;
	if (!client.get_log_status())
		ret = Server::login_block(word, client);
	else
	{
		int	size = word[0].size();
		switch (size)
		{
		case 4:
			ret = Server::four_len_block(word, client);
			break;
		case 5:
			ret = Server::five_len_block(word, client);
			break;
		case 6:
			ret = Server::six_len_block(word, client);
			break;
		case 7:
			ret = Server::seven_len_block(word, client);
			break;
		default:
			ret = 421;	// [421 ERR_UNKNOWNCOMMAND]
			break;
		}
	}

	if (ret == 0)
		return (0);
	else
	{
		if (ret == -1)
		{
			remove_client(client.get_fd());
			return (-1);
		}
		else
			IRCReplies::srv_answer(client, word, ret);
	}
	return (0);
}

int		Server::login_block(std::vector<std::string>& word, Client& client)
{
	int	ret = 0;
	if (word[0] == "PASS")
		ret = Server::cmd_pass(word, client);
	else if (word[0] == "USER")
		ret = Server::cmd_user(word, client);
	else if (word[0] == "NICK")
		ret = Server::cmd_nick(word, client);
	else
	{
		std::stringstream	ss;
		ss << "Client " << client.get_fd() << ": Access forbidden (not logged)";
		Logger::log(CRITICAL, ss.str().c_str());
		ret = 444;	// [444 ERR_NOLOGIN]
	}
	if (client.get_pass_status() && client.get_username() != "" && client.get_nickname() != "")
	{
		client.set_log_status(true);
		Logger::log(INFO, "client successfully logged in");
		std::string	temp = IRCReplies::rpl_welcome(client.get_nickname());
		client.send_message(temp);
	}
	return (ret);
}

int		Server::four_len_block(std::vector<std::string>& word, Client& client)
{
	int	ret = 0;
	if (word[0] == "PING")
		ret = Server::cmd_ping(word, client);
	else if (word[0] == "JOIN")
		ret = Server::cmd_join(word, client);
	else if (word[0] == "PART")
		ret = Server::cmd_part(word, client);
	else if (word[0] == "KICK")
		ret = Server::cmd_kick(word, client);
	else if (word[0] == "MODE")
		ret = Server::cmd_mode(word, client);
	else if (word[0] == "NICK")
		ret = Server::cmd_nick(word, client);
	else if (word[0] == "PASS" || word[0] == "USER")
		ret = 462;	// [462 ERR_ALREADYREGISTRED]
	else
		ret = 421;	// [421 ERR_UNKNOWNCOMMAND]
	return (ret);
}

int		Server::five_len_block(std::vector<std::string>& word, Client& client)
{
	int	ret = 0;
	if (word[0] == "TOPIC")
		ret = Server::cmd_topic(word, client);
	else
		ret = 421;	// [421 ERR_UNKNOWNCOMMAND]
	return (ret);
}

int		Server::six_len_block(std::vector<std::string>& word, Client& client)
{
	int	ret = 0;
	if (word[0] == "INVITE")
		ret = Server::cmd_invite(word, client);
	else
		ret = 421;	// [421 ERR_UNKNOWNCOMMAND]
	return (ret);
}

int		Server::seven_len_block(std::vector<std::string>& word, Client& client)
{
	int	ret = 0;
	if (word[0] == "PRIVMSG")
		ret = Server::cmd_privmsg(word, client);
	else
		ret = 421;	// [421 ERR_UNKNOWNCOMMAND]
	return (ret);
}

void	Server::bot_block(std::vector<Channel>::iterator& it, Client& client, std::string& cmd)
{
	std::stringstream ss;
	ss << ":horse_bot!horse_bot@GrosseEtable PRIVMSG " << it->get_chan_name() << " :";
	if (cmd == "!time")
		ss << client.get_nickname() << " has been online for " << client.get_logtime() << " seconds \r\n";
	else if (cmd == "!key")
	{
		if (it->has_key())
			ss << "Current [key] is [" << it->get_key() << "]\r\n";
		else
			ss << "This stable doesn't have a key\r\n";
	}
	else if (cmd == "!trivia")
		ss << "I'm a fat robotic horse, and i stink, and i'm fat\r\n";
	else if (cmd == "!help")
		ss << "!time, !key, !trivia, !help, !ping available\r\n";
	else if (cmd == "!ping")
		ss << client.get_nickname() << " pong\r\n";
	else
		ss << "I can't horse with this\r\n";
	broadcast_to_chan(it->get_user_list(), ss.str());
}

// PASS pass
int	Server::cmd_pass(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 2)
		return (461);	// [461 ERR_NEEDMOREPARAMS]
	
	if (params[1] != _pass)
	{
		std::stringstream	ss;
		ss << "Client " << client.get_fd() << ": Wrong password";
		Logger::log(WARNING, ss.str().c_str());
		return (-1);
	}
	client.set_pass_status(true);
	return (0);
}

/*
	USER user
	username can only be set once unlike nickname as it is used for identification purposes
*/
int	Server::cmd_user(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 2)
		return (461);	// [461 ERR_NEEDMOREPARAMS]
	
	if (client.get_username() == "")
	{
		for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
		{
			if (it->get_username() == params[1])
			{
				Logger::log(CRITICAL, "Username already in use");
				return (462);
			}
		}
		client.set_username(params[1]);
		Logger::log(INFO, "Changed client username");
		if (params.size() < 4)
			client.set_realname(params[1]);
		else
			client.set_realname(params[4]);
		Logger::log(INFO, "Changed client realname");
	}
	return (0);
}

/*
	NICK nick
	nickname is the name shown to others
*/
int	Server::cmd_nick(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 2)
		return (461);	// [461 ERR_NEEDMOREPARAMS]
	
	for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
	{
		if (it->get_nickname() == params[1])
			return (433);	// [433 ERR_NICKNAMEINUSE]
	}
	client.set_nickname(params[1]);
	// TODO: broadcast to channels where user is present
	return (0);
}

int	Server::cmd_ping(std::vector<std::string>& params, Client& client)
{
	(void) params;

	std::string	temp = IRCReplies::rpl_pong();
	std::string	pong = "PONG sent to " + client.get_nickname();
	Logger::log(INFO, pong.c_str());
	client.send_message(temp);
	return (0);
}

// JOIN #chan (pass)
int	Server::cmd_join(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 2)
		return (461);	// [461 ERR_NEEDMOREPARAMS]
	
	std::vector<Channel>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); it++)
	{
		if (it->get_chan_name() == params[1])
			break;
	}
	if (it == _channels.end())
	{
		if (params[1][0] == '#')
		{
			Channel	new_chan(params[1], client, _channels.size() + 1);
			_channels.push_back(new_chan);
			broadcast_to_chan(new_chan.get_user_list(), IRCReplies::rpl_join(client.get_nickname(), params[1]));
			client.send_message(IRCReplies::rpl_notopic(new_chan.get_chan_name()));
		}
		else
			return (403);	// [403 ERR_NOSUCHCHANNEL]
		return (0);
	}
	
	int	ret = 0;
	if (params.size() == 3)
		ret = it->join(client, params[2]);
	else
		ret = it->join(client, "");

	std::string	temp;
	if (ret == 443)
	{
		temp = IRCReplies::err_userinchannel_join(client.get_nickname(), params[1]);
		client.send_message(temp);
		return (0);
	}
	else if (ret)
		return (ret);
	broadcast_to_chan(it->get_user_list(), IRCReplies::rpl_join(client.get_nickname(), params[1]));
	if (it->get_topic() == "")
		temp = IRCReplies::rpl_notopic(it->get_chan_name());
	else
		temp = IRCReplies::rpl_topic(it->get_chan_name(), it->get_topic());
	client.send_message(temp);
	return (ret);
}

// PART #chan :(exit msg)
int	Server::cmd_part(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 2)
		return (461);	// [461 ERR_NEEDMOREPARAMS]

	std::vector<Channel>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); it++)
	{
		if (it->get_chan_name() == params[1])
			break;
	}
	if (it == _channels.end())
		return (403);	// [403 ERR_NOSUCHCHANNEL]

	if (!it->is_on_chan(client))
		return (442);	// [442 ERR_NOTONCHANNEL]
	
	std::string	temp;
	if (params.size() == 3)
		temp = IRCReplies::rpl_part(client, it->get_chan_name(), params[2]);
	else
		temp = IRCReplies::rpl_part(client, it->get_chan_name());
	broadcast_to_chan(it->get_user_list(), temp);

	int	ret = it->part(client);
	if (ret)
		return (ret);

	if (it->get_user_list().size() == 0)
		_channels.erase(it);

	return (ret);
}

// KICK #chan user :(msg)
int	Server::cmd_kick(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 3)
		return (461);	// [461 ERR_NEEDMOREPARAMS]

	std::vector<Channel>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); it++)
	{
		if (it->get_chan_name() == params[1])
			break;
	}
	if (it == _channels.end())
		return (403);	// [403 ERR_NOSUCHCHANNEL]

	if (!is_on_serv_nickname(params[2]))
		return (401);	// [401 ERR_NOSUCHNICK]

	Client *guest = this->get_client_from_nickname(params[2]);
	if (guest == NULL)
		return (441);	// [441 ERR_USERNOTINCHANNEL]

	int ret = it->kick(client, *guest);
	if (ret)
		return (ret);
	
	std::string	temp;
	if (params.size() == 4)
		temp = IRCReplies::rpl_kick(client, params[2], params[1], params[3]);
	else
		temp = IRCReplies::rpl_kick(client, params[2], params[1], "");
	
	broadcast_to_chan(it->get_user_list(), temp);
	guest->send_message(IRCReplies::err_kickedfromchan(client, params[2], params[1]));

	return (ret);
}

// MODE #chan flag (param depending on set flag)
int	Server::cmd_mode(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 3)
		return (461);	// [461 ERR_NEEDMOREPARAMS]

	if (params[1][0] != '#')
	{
		Logger::log(WARNING, "Client tried to set mode for a user (unsupported)");
		if (params[2][1] == 'i')
		{
			// shush irssi automode setting
			std::string	temp = IRCReplies::rpl_umodeis(params[1]);
			temp += IRCReplies::rpl_modechangewarn(params[1]);
			client.send_message(temp);
		}
		return (0);
	}

	std::vector<Channel>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); it++)
	{
		if (it->get_chan_name() == params[1])
			break;
	}
	if (it == _channels.end())
		return (403);	// [403 ERR_NOSUCHCHANNEL]

	if (!it->is_on_chan(client))
		return (442);	// [442 ERR_NOTONCHANNEL]
	
	if (params[2].size() != 2 || (params[2][0] != '-' && params[2][0] != '+'))
		return (501);	// [501 ERR_UMODEUNKNOWNFLAG]
	
	int	ret = 0;
	if (params[2] == "+k" || params[2] == "+l" || params[2][1] == 'o')
	{
		if (params.size() < 4)
			return (461);	// [461 ERR_NEEDMOREPARAMS]
		
		if (params[2][1] == 'o')
		{
			Client *temp = get_client_from_nickname(params[3]);
			ret = it->mode(client, params[2][0], params[2][1], temp->get_username());
		}
		else
			ret = it->mode(client, params[2][0], params[2][1], params[3]);
	}
	else
		ret = it->mode(client, params[2][0], params[2][1], "");
	
	return (ret);
}

/*
	TOPIC #chan :(msg)
	1 param --> get topic
	2 param --> set topic

	message requires a ':' or it cut to the first word
		irssi send ':' but need a RPL for topic
*/
int	Server::cmd_topic(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 2)
		return (461);	// [461 ERR_NEEDMOREPARAMS]
	
	std::vector<Channel>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); it++)
	{
		if (it->get_chan_name() == params[1])
			break;
	}
	if (it == _channels.end())
		return (403);	// [403 ERR_NOSUCHCHANNEL]

	int	ret = 0;
	if (params.size() == 3)
	{
		ret = it->set_topic(client, params[2]);
		if (ret)
			return (ret);
		broadcast_to_chan(it->get_user_list(), IRCReplies::rpl_topic(it->get_chan_name(), it->get_topic()));
	}
	else
	{
		std::string	temp;
		if (it->get_topic() == "")
			temp = IRCReplies::rpl_notopic(it->get_chan_name());
		else
			temp = IRCReplies::rpl_topic(it->get_chan_name(), it->get_topic());
		broadcast_to_chan(it->get_user_list(), temp);
	}
	return (ret);
}

// INVITE user #chan
int	Server::cmd_invite(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 3)
		return (461);	// [461 ERR_NEEDMOREPARAMS]

	if (!is_on_serv_nickname(params[1]))
		return (401);	// [401 ERR_NOSUCHNICK]

	std::vector<Channel>::iterator it;
	for (it = _channels.begin(); it != _channels.end(); it++)
	{
		if (it->get_chan_name() == params[2])
			break;
	}
	if (it == _channels.end())
		return (403);	// [403 ERR_NOSUCHCHANNEL]

	Client* guest = this->get_client_from_nickname(params[1]);
	if (guest == NULL)
		return (441);	// [441 ERR_USERNOTINCHANNEL]
	
	int	ret = it->invite(client, *guest);
	if (ret)
		return (ret);

	std::string	temp = IRCReplies::rpl_join(guest->get_nickname(), params[1]);
	guest->send_message(IRCReplies::rpl_inviting(params[2], params[1]));
	broadcast_to_chan(it->get_user_list(), temp);

	if (it->get_topic() == "")
		temp = IRCReplies::rpl_notopic(it->get_chan_name());
	else
		temp = IRCReplies::rpl_topic(it->get_chan_name(), it->get_topic());
	guest->send_message(temp);
	return (ret);
}

/*
	Client to server:
	PRIVMSG [#chan OR user] msg

	Server to client:
	:user@localhost PRIVMSG #channel :message goes here

*/
int	Server::cmd_privmsg(std::vector<std::string>& params, Client& client)
{
	if (params.size() < 3)
		return (461);	// [461 ERR_NEEDMOREPARAMS]

	if (params[1][0] == '#')
	{
		std::vector<Channel>::iterator it;
		for (it = _channels.begin(); it != _channels.end(); it++)
		{
			if (params[1] == it->get_chan_name())
				break;
		}
		if (it == _channels.end())
			return (403);	// [403 ERR_NOSUCHCHANNEL]
		if (!it->is_on_chan(client.get_username()))
			return (442);	// [442 ERR_NOTONCHANNEL]
		std::vector<std::string> user_list = it->get_user_list();
		std::string	temp = ":";
		if (it->is_operator(client))
			temp += "@";
		temp += client.get_nickname() + "@GrosseEtable PRIVMSG " + it->get_chan_name() + " :" + params[2] + "\r\n";
		// TODO: could broadcast here, see with irssi if change
		for (std::vector<std::string>::iterator iter = user_list.begin(); iter != user_list.end(); iter++)
		{
			Client	current = *get_client_from_username(*iter);
			if (current.get_username() != client.get_username())
				current.send_message(temp);
		}

		if (params[2][0] == '!')
			bot_block(it, client, params[2]);
	}
	else
	{
		if (!is_on_serv_nickname(params[1]))
			return (401);	// [401 ERR_NOSUCHNICK]

		std::string	temp = ":";
		Client&	current = *get_client_from_nickname(params[1]);
		temp += client.get_nickname() + "@GrosseEtable PRIVMSG " + current.get_nickname() + " :" + params[2] + "\r\n";
		current.send_message(temp);
	}
	return (0);
}

Server::Server(const Server &server) : _port(server._port), _pass(server._pass)
{
	if (_port < 1 || _port > 65535)
		throw InvalidPort();

	create_socket();
	bind_socket();

	if (listen(_socket, MAX_CONNECTIONS) < 0)
		throw CannotListen();
	Logger::log(INFO, "Socket created successfully");
}

Server::Server() : _port(6667), _pass("s3cur3_p455")
{
	if (_port < 1 || _port > 65535)
		throw InvalidPort();

	create_socket();
	bind_socket();

	if (listen(_socket, MAX_CONNECTIONS) < 0)
		throw CannotListen();
	Logger::log(INFO, "Socket created successfully");
}

Server::Server(int port, std::string pass) : _port(port), _pass(pass)
{
	if (_port < 1 || _port > 65535)
		throw InvalidPort();

	create_socket();
	bind_socket();

	if (listen(_socket, MAX_CONNECTIONS) < 0)
		throw CannotListen();
	Logger::log(INFO, "Socket created successfully");
}

Server::~Server()
{
	for (size_t i = 0; i < _pfds.size(); i++)
		close(_pfds[i].fd);
}

const char *Server::CannotBindSocket::what() 			const throw() 	{ return "Cannot bind socket"; };

const char *Server::CannotCreateSocketException::what() const throw()	{ return "Cannot create socket"; };

const char *Server::CannotSetSocketNonBlock::what() 	const throw()	{ return "Cannot set socket as non blocking"; };

const char *Server::CannotSetSocketOptions::what() 		const throw()	{ return "Cannot set socket socket options"; };

const char *Server::CannotListen::what() 				const throw()	{ return "Socket cannot listen"; };

const char *Server::InvalidPort::what() 				const throw() 	{ return "Port must be between 1 and 65535"; };

const char *Server::CannotPoll::what() 					const throw() 	{ return "Server stopped"; };

const char *Server::CannotAcceptConnection::what() 		const throw() 	{ return "Cannot accept new connection"; };

const char *Server::CannotReadMessage::what() 			const throw() 	{ return "Cannot read message"; };

void Server::listen_activity()
{
	// Registering first fd for the poll
	// it's the server itself
	pollfd srv = {_socket, POLLIN, 0};
	_pfds.push_back(srv);
	while (server_running)
	{
		// polling all fds
		if (poll(_pfds.begin().base(), _pfds.size(), -1) < 0)
			return ;
		for (size_t i = 0; i < _pfds.size(); i++)
		{
			// no message
			if (_pfds[i].revents == 0)
				continue ;
			// Unexpected disconnect	TODO: Check POLLERR
			if ((_pfds[i].revents & POLLHUP) == POLLHUP)
			{
				Logger::log(INFO, "Client disconnected");
				remove_client(_pfds[i].fd);
				break ;
			}
			// Connection or message
			if ((_pfds[i].revents & POLLIN) == POLLIN)
			{
				// new connection
				if (_pfds[i].fd == _socket)
				{
					int fd;
					struct sockaddr_in client_sock = {};
					socklen_t client_len = sizeof(client_sock);
					fd = accept(_socket, (sockaddr *)&client_sock, &client_len);
					if (fd < 0)
						throw CannotAcceptConnection();
					pollfd clientfd = {fd, POLLIN, 0};
					Client client(clientfd.fd);
					_pfds.push_back(clientfd);
					_clients.push_back(client);
					std::stringstream	ss;
					ss << "Client connected: " << clientfd.fd;
					Logger::log(INFO, ss.str().c_str());
					break ;
				}
				// message
				handle_msg(_pfds[i].fd);
			}
		}
	}
}

// TODO: change part to quit
void Server::remove_client(int fd)
{
	Client* client = get_client_from_fd(fd);
	if (client != NULL)
	{
		for (size_t i = 0; i < _channels.size(); i++)
		{
			if (_channels[i].is_on_chan(client->get_username()))
				_channels[i].part(*client);
		}
	}
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].get_fd() == fd)
		{
			_clients.erase(_clients.begin() + i);
			break ;
		}
	}
	std::vector<pollfd>::iterator clients_begin = _pfds.begin();
	std::vector<pollfd>::iterator clients_end = _pfds.end();
	while (clients_begin != clients_end)
	{
		if (clients_begin->fd == fd)
		{
			std::stringstream	ss;
			ss << "Client " << fd << " disconnected";
			Logger::log(INFO, ss.str().c_str());
			_pfds.erase(clients_begin);
			close(fd);
			break ;
		}
		clients_begin++;
	}
}
