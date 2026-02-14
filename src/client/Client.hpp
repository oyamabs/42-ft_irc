#pragma once

#include <string>
#include <queue>
#include <sys/time.h>

class Client{
private:
	int						_fd;
	std::string				_realname;
	std::string				_username;
	std::string				_hostname;
	std::string				_nickname;
	bool					_is_away;
	std::string				_away_msg;
	std::string 			_message_buffer;
	std::queue<std::string>	_message_queue;
	struct timeval			_connection_time;
	bool					_password_ok;
	bool					_is_logged;

	Client();

public:
	Client(int fd);
	Client(const Client &client);
	Client &operator=(const Client &client);
	~Client();

	const int&				get_fd(void) 			const;
	const std::string& 		get_realname(void) 		const;
	const std::string& 		get_username(void) 		const;
	const std::string& 		get_hostname(void) 		const;
	const std::string& 		get_nickname(void) 		const;
	const bool&				get_status(void)		const;
	const std::string&		get_away_msg(void)		const;
	std::string&			get_message_buffer(void);
	unsigned long			get_logtime_ms(void);
	unsigned long			get_logtime(void);
	const bool&				get_pass_status(void)	const;
	const bool&				get_log_status(void)	const;
	std::queue<std::string>	get_message_queue(void)	const;
	std::string				get_queue_front(void);

	void					set_realname(const std::string);
	void					set_username(const std::string);
	void					set_hostname(const std::string);
	void					set_nickname(const std::string);
	void					set_away_status(bool);
	void					set_away_msg(const std::string);
	void					set_pass_status(bool);
	void					set_log_status(bool);
	int						send_message(std::string content);
	void					add_to_queue(std::string cmd);
};
