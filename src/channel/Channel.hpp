#pragma once

#include <string>
#include <vector>
#include "../client/Client.hpp"

/*
	user never leave the operator status, if they leave and rejoin they stay OP
	as we don't log users, we're going to have it YOLO by name / nick / realname
*/
class Channel
{
	private:
		int								_id;
		std::string						_name;
		std::string						_topic;
		bool							_topic_mode;
		bool							_invite_mode;
		size_t							_user_limit;
		std::string						_key;
		std::vector<std::string>		_users;
		std::vector<std::string>		_operators;

		Channel();

	public:
		Channel(std::string name, Client& client, int id);
		Channel(const Channel &copy);
		~Channel();
		Channel&	operator=(const Channel &other);

		// Regular CMD
		int		join(Client& client, std::string pass);
		int		part(Client& client);

		// Operator-only CMD
		int		invite(Client& host, Client& guest);
		int		kick(Client& host, Client& guest);
		int		mode(Client& client, char sign, char flag, std::string msg);

		bool	is_on_chan(Client& client)						const;
		bool	is_on_chan(std::string client_name)				const;
		bool	is_operator(Client& client)						const;
		bool	is_operator(std::string client_name)			const;
		bool	has_key(void)									const;
		bool	is_right_key(std::string s)						const;

		int		set_topic(Client& client, std::string topic);

		const std::string&				get_key(void)			const;
		const std::string&				get_topic(void)			const;
		const std::string&				get_chan_name(void)		const;
		const bool&						get_topic_mode(void)	const;
		const bool&						get_invite_mode(void)	const;
		const size_t&					get_user_limit(void)	const;
		const std::vector<std::string>& get_user_list(void)		const;
		const std::vector<std::string>& get_op_list(void)		const;
};
