#include "PocketParser.hpp"
#include "../logger/Logger.hpp"
#include <sstream>

PocketParser::PocketParser() : _cmd(""), _nick(""), _chan(""), _flag(""), _client_nick("")
{}

PocketParser::~PocketParser()
{}

void	PocketParser::parse(const Client& client, const std::vector<std::string>& params, int code)
{
	// fallthrough is used here to parse ONLY the useful parts
	switch (code)
	{
	case 462:
	case 501:
		break;
	case 421:
	case 461:
		parse_cmd(params);
		break;
	case 401:
	case 433:
		parse_nick(params);
		break;
	case 403:
	case 442:
	case 482:
		parse_chan(params);
		break;
	case 441:
	case 443:
		parse_chan(params);
		parse_nick(params);
		break;
	case 471:
	case 473:
	case 475:
		parse_chan(params);
		_client_nick = client.get_nickname();
		break ;
	case 472:
		parse_flag(params);
		break;
	default:
		std::stringstream	sss;
		sss << "PocketParsing::" << code;
		Logger::log(CRITICAL, sss.str().c_str());
		break;
	}
	_client_nick = client.get_nickname();
}

void	PocketParser::parse_cmd(const std::vector<std::string>& params)
{	_cmd = params[0];	}

void	PocketParser::parse_nick(const std::vector<std::string>& params)
{
	if (params[0] == "KICK")
		_nick = params[2];
	else if (params[0] == "MODE")
		_nick = params[3];
	else
		_nick = params[1];
}

void	PocketParser::parse_chan(const std::vector<std::string>& params)
{
	if (params[0] == "INVITE")
		_chan = params[2];
	else
		_chan = params[1];
}

void	PocketParser::parse_flag(const std::vector<std::string>& params)
{
	if (params[0] == "MODE")
		_flag = params[2];
}

const std::string& PocketParser::get_cmd(void)	const
{	return (_cmd);				}
const std::string& PocketParser::get_nick(void)	const
{	return (_nick);				}
const std::string& PocketParser::get_chan(void)	const
{	return (_chan);				}
const std::string& PocketParser::get_flag(void)	const
{	return (_flag);				}
const std::string& PocketParser::get_client_nick(void)	const
{	return (_client_nick);		}