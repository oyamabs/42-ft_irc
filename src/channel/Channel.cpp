#include <algorithm>
#include "Channel.hpp"
#include "../logger/Logger.hpp"

Channel::Channel()
{}

Channel::Channel(std::string name, Client& client, int id)
{
	// Creator becomes operator
	this->_id			= id;
	this->_name 		= name;
	this->_topic 		= "";
	this->_topic_mode 	= false;
	this->_invite_mode 	= false;
	this->_user_limit	= 0;
	this->_key			= "";
	_users.push_back(client.get_username());
	_operators.push_back(client.get_username());
	std::string	temp = "Channel " + this->get_chan_name() + " was created by " + client.get_username();
	Logger::log(INFO, temp.c_str());
}

Channel::Channel(const Channel &copy) : _id(copy._id), _name(copy._name), _topic(copy._topic),
_topic_mode(copy._topic_mode), _invite_mode(copy._invite_mode), _user_limit(copy._user_limit),
_key(copy._key), _users(copy._users), _operators(copy._operators)
{}

Channel::~Channel()
{
	// std::string	temp = "Channel " + this->get_chan_name() + " was deleted ";
	// Logger::log(INFO, temp.c_str());
}

Channel&	Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		this->_id			= other._id;
		this->_name 		= other._name;
		this->_topic 		= other._topic;
		this->_topic_mode 	= other._topic_mode;
		this->_invite_mode 	= other._invite_mode;
		this->_user_limit	= other._user_limit;
		this->_key			= other._key;
		this->_users		= other._users;
		this->_operators	= other._operators;
	}
	return (*this);
}

int		Channel::join(Client& client, std::string pass)
{
	if (this->is_on_chan(client))
		return (443);	// [443 ERR_USERONCHANNEL]

	if (_invite_mode == true)
		return (473);	// [473 ERR_INVITEONLYCHAN]

	if (_users.size() >= _user_limit && _user_limit != 0)
		return (471);	// [471 ERR_CHANNELISFULL]

	if (_key != "" && _key != pass)
		return (475);	// [475 ERR_BADCHANNELKEY]

	_users.push_back(client.get_username());
	std::string	temp = client.get_username() + " joined " + this->get_chan_name();
	if (pass != "")
		temp += " (" + pass + ")";
	Logger::log(INFO, temp.c_str());
	return (0);
}

int		Channel::part(Client& client)
{
	if (!this->is_on_chan(client))
		return (442);	// [442 ERR_NOTONCHANNEL]

	_users.erase(std::find(_users.begin(), _users.end(), client.get_username()));
	std::string	temp = client.get_username() + " left " + this->get_chan_name();
	Logger::log(INFO, temp.c_str());
	return (0);
}

int		Channel::invite(Client& host, Client& guest)
{
	if (!this->is_operator(host))
		return (482);	// [482 ERR_CHANOPRIVSNEEDED]

	if (this->is_on_chan(guest))
		return (443);	// [443 ERR_USERONCHANNEL]

	if (_users.size() >= _user_limit && _user_limit != 0)
		return (471);	// [471 ERR_CHANNELISFULL]
			// invite still successfully sent even if guest can't join
	
	_users.push_back(guest.get_username());
	std::string	temp = host.get_username() + " invited " + guest.get_username() + " to " + this->get_chan_name();
	Logger::log(INFO, temp.c_str());
	return (0);
}

int		Channel::kick(Client& host, Client& guest)
{
	if (!this->is_operator(host))
		return (482);	// [482 ERR_CHANOPRIVSNEEDED]

	if (!this->is_on_chan(guest))
		return (441);	// [441 ERR_USERNOTINCHANNEL]

	_users.erase(std::find(_users.begin(), _users.end(), guest.get_username()));
	std::string	temp = host.get_username() + " kicked " + guest.get_username() + " from " + this->get_chan_name();
	Logger::log(INFO, temp.c_str());
	return (0);
}

// mode [+ OR -] [flag letter] [message OR value if applicable]
// MODE +l 100	-->	set l flag and user limit to 100
int		Channel::mode(Client& client, char sign, char flag, std::string param)
{
	if (!this->is_operator(client))
		return (482);	// [482 ERR_CHANOPRIVSNEEDED]
	
	if (flag == 'i')
	{
		if (sign == '+')
			_invite_mode = true;
		else if (sign == '-')
			_invite_mode = false;
	}
	else if (flag == 't')
	{
		if (sign == '+')
			_topic_mode = true;
		else if (sign == '-')
			_topic_mode = false;
	}
	else if (flag == 'k')	// TODO: Check input somewhere
	{
		if (sign == '+')
			_key = param;
		else if (sign == '-')
			_key = "";
	}
	else if (flag == 'o')
	{
		if (!this->is_on_chan(param))
			return (441);	// [441 ERR_USERNOTINCHANNEL]

		if (sign == '+')
		{
			if (!this->is_operator(param))
				_operators.push_back(param);
		}
		else if (sign == '-')
		{
			if (this->is_operator(param))
				_operators.erase(std::find(_operators.begin(), _operators.end(), param));
		}
	}
	else if (flag == 'l')
	{
		if (sign == '+')
			_user_limit = std::atoi(param.c_str());
		else if (sign == '-')
			_user_limit = 0;
	}
	else
		return (472);	// [472 ERR_UNKNOWNMODE]
	
	std::string	temp = client.get_username() + " changed mode on " + this->get_chan_name();
	Logger::log(INFO, temp.c_str());
	return (0);
}

// TODO: Reduce to a simple return?
bool	Channel::is_on_chan(Client& client) const
{
	if (std::find(_users.begin(), _users.end(), client.get_username()) != _users.end())
		return (true);
	return (false);
}

bool	Channel::is_on_chan(std::string client_username) const
{
	if (std::find(_users.begin(), _users.end(), client_username) != _users.end())
		return (true);
	return (false);
}

bool	Channel::is_operator(Client& client) const
{
	if (std::find(_operators.begin(), _operators.end(), client.get_username()) != _operators.end())
		return (true);
	return (false);
}

bool	Channel::is_operator(std::string client_name) const
{
	if (std::find(_operators.begin(), _operators.end(), client_name) != _operators.end())
		return (true);
	return (false);
}

bool	Channel::has_key(void) const
{
	if (_key != "")
		return (true);
	return (false);
}

bool	Channel::is_right_key(std::string s) const
{
	if (s == _key)
		return (true);
	return (false);
}

int		Channel::set_topic(Client& client, std::string topic)
{
	if (_topic_mode == true)
	{
		if (!is_operator(client))
			return (482);	// [482 ERR_CHANOPRIVSNEEDED]
		_topic = topic;
	}
	else if (_topic_mode == false)
		_topic = topic;
	std::string	temp = client.get_username() + " changed topic on " + this->get_chan_name();
	Logger::log(INFO, temp.c_str());
	return (0);
}

const std::string&				Channel::get_key(void)			const
{ return (_key);			}

const std::string&				Channel::get_topic(void)		const
{ return (_topic); 			}

const std::string&				Channel::get_chan_name(void)	const
{ return (_name); 			}
const bool&						Channel::get_topic_mode(void)	const
{ return (_topic_mode); 	}

const bool&						Channel::get_invite_mode(void)	const
{ return (_invite_mode); 	}

const size_t&					Channel::get_user_limit(void)	const
{ return (_user_limit); 	}

const std::vector<std::string>& Channel::get_user_list(void)	const
{ return (_users); 			}

const std::vector<std::string>& Channel::get_op_list(void)		const
{ return (_operators); 		}
