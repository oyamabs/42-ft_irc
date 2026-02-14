#include "Client.hpp"
#include <sys/types.h>
#include <sys/socket.h>

Client::Client(int fd) : _fd(fd), _realname(""), _username(""), _hostname(""), _nickname(""),
_is_away(false), _away_msg(""), _message_buffer(""), _password_ok(false) , _is_logged(false)
{	(void) gettimeofday(&_connection_time, NULL);	}

Client::Client(const Client &copy) : _fd(copy._fd), _realname(copy._realname),
_username(copy._username), _hostname(copy._hostname), _nickname(copy._nickname),
_is_away(copy._is_away), _away_msg(copy._away_msg), _message_buffer(copy._message_buffer),
_connection_time(copy._connection_time), _password_ok(copy._password_ok), _is_logged(copy._is_logged)
{}

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		_fd 				= other._fd;
		_realname 			= other._realname;
		_username 			= other._username;
		_hostname 			= other._hostname;
		_nickname 			= other._nickname;
		_is_away 			= other._is_away;
		_away_msg 			= other._away_msg;
		_message_buffer 	= other._message_buffer;
		_message_queue 		= other._message_queue;
		_connection_time 	= other._connection_time;
		_password_ok		= other._password_ok;
		_is_logged 			= other._is_logged;
		_connection_time	= other._connection_time;
	}
	return (*this);
}

Client::~Client()
{}

const int&				Client::get_fd(void)		const
{ return (_fd);				}

const std::string& 		Client::get_realname(void)	const
{ return (_realname);		}

const std::string& 		Client::get_username(void)	const
{ return (_username);		}

const std::string& 		Client::get_hostname(void)	const
{ return (_hostname);		}

const std::string& 		Client::get_nickname(void)	const
{ return (_nickname);		}

const bool&				Client::get_status(void)	const
{ return (_is_away);		}

const std::string&		Client::get_away_msg(void)	const
{ return (_away_msg);		}

std::string&			Client::get_message_buffer(void) 
{ return (_message_buffer);	}

/*
	Shamelessely creeped back from CPP09, previously taken from philosopher
	return time in ms, used for timeout in case client fail to identify
*/
unsigned long			Client::get_logtime_ms()
{
	struct timeval& t0 = _connection_time;
	struct timeval 	t1;
	gettimeofday(&t1, NULL);
	return ((t1.tv_sec - t0.tv_sec) * 1000UL
		+ (t1.tv_usec - t0.tv_usec / 1000UL));
}

// Return time in seconds
unsigned long			Client::get_logtime()
{
	struct timeval& t0 = _connection_time;
	struct timeval 	t1;
	gettimeofday(&t1, NULL);
	return ((t1.tv_sec - t0.tv_sec));
}

// Password flag to use during irssi login burst
const bool&				Client::get_pass_status(void)	const
{	return (_password_ok);	}

// Authentification flag
const bool&				Client::get_log_status(void)	const
{	return (_is_logged);	}

/*
	TODO: Add various checks and error code later on depending on implemented features
*/
void					Client::set_realname(const std::string real)
{	_realname = real;		}

void					Client::set_username(const std::string user)
{	_username = user;		}

void					Client::set_hostname(const std::string host)
{	_hostname = host;		}

void					Client::set_nickname(const std::string nick)
{	_nickname = nick;		}

void					Client::set_away_status(bool status)
{	_is_away = status;		}

void					Client::set_away_msg(const std::string msg)
{	_away_msg = msg;		}

void					Client::set_pass_status(bool status)
{	_password_ok = status;	}

void					Client::set_log_status(bool status)
{	_is_logged = status;	}

int						Client::send_message(std::string content)
{
	return (send(_fd, content.c_str(), content.size(), 0));
}

std::queue<std::string> Client::get_message_queue(void)	const
{
	return (_message_queue);
}

std::string 			Client::get_queue_front(void)
{
	std::string	ret = _message_queue.front();
	_message_queue.pop();
	return (ret);
}

void 					Client::add_to_queue(std::string cmd)
{	_message_queue.push(cmd);}
