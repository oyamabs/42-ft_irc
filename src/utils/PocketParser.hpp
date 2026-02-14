#pragma once

#include <string>
#include <vector>
#include "../client/Client.hpp"

class PocketParser
{
private:
	std::string	_cmd;
	std::string	_nick;
	std::string	_chan;
	std::string	_flag;
	std::string	_client_nick;

public:
	PocketParser();
	~PocketParser();

	void	parse(const Client& client, const std::vector<std::string>& params, int code);
	void	parse_cmd(const std::vector<std::string>& params);
	void	parse_nick(const std::vector<std::string>& params);
	void	parse_chan(const std::vector<std::string>& params);
	void	parse_flag(const std::vector<std::string>& params);

	const std::string& get_cmd(void)			const;
	const std::string& get_nick(void)			const;
	const std::string& get_chan(void)			const;
	const std::string& get_flag(void)			const;
	const std::string& get_client_nick(void)	const;
};
