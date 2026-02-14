#include "IrcReplies.hpp"
#include "../logger/Logger.hpp"
#include "../server/Server.hpp"
#include <algorithm>
#include <iostream>

std::stringstream IRCReplies::_ss;

void IRCReplies::err_nosuchnick(const std::string &nick)
{	_ss << ":GrosseEtable " << nick << " :No such nick\r\n";}

void IRCReplies::err_nosuchchannel(const std::string &channel)
{	_ss << ":GrosseEtable " << channel << " :No such channel\r\n";}

void IRCReplies::err_unknowncommand(const std::string &command)
{	_ss << ":GrosseEtable " << command << " :Unknown command\r\n";}

void IRCReplies::err_nicknameinuse(const std::string &nick)
{	_ss << ":GrosseEtable " << nick << " :Unauthorized command (already registered)\r\n";}

void IRCReplies::err_usernotinchannel(const std::string &nick, const std::string &channel)
{	_ss << ":GrosseEtable " << nick << " " << channel << " :They aren't on that channel\r\n";}

void IRCReplies::err_notonchannel(const std::string &channel)
{	_ss << ":GrosseEtable " << channel << " :You're not on that channel\r\n";}

void IRCReplies::err_userinchannel(const std::string &nick, const std::string &channel)
{	_ss << ":GrosseEtable " << nick << " " << channel << " :is already on channel\r\n";}

void IRCReplies::err_needmoreparams(const std::string &command)
{	_ss << ":GrosseEtable " << command << " :Not enough parameters\r\n";}

void IRCReplies::err_alreadyregistered(void)
{	_ss << ":GrosseEtable :Unauthorized command (already registered)\r\n";}

void IRCReplies::err_channelisfull(const std::string &client_nick, const std::string &channel)
{	_ss << ":GrosseEtable 471 " << client_nick << " " << channel << " :Cannot join channel (+l)\r\n";}

void IRCReplies::err_unknownmode(const std::string &mode)
{	_ss << ":GrosseEtable " << mode << " :is unknown mode char to me\r\n";}

void IRCReplies::err_inviteonlychan(const std::string &client_nick, const std::string &channel)
{	_ss << ":GrosseEtable 473 " << client_nick << " " << channel << " :Cannot join channel (+i)\r\n";}

void IRCReplies::err_badchannelkey(const std::string &client_nick, const std::string &channel)
{	_ss << ":GrosseEtable 475 " << client_nick << " " << channel << " :Cannot join channel (+k)\r\n";}

void IRCReplies::err_chanoperprivsneeded(const std::string &channel)
{	_ss << ":GrosseEtable " << channel << " :You're not channel operator\r\n";}

void IRCReplies::err_umodeunknownflag(void)
{	_ss << ":GrosseEtable :Unknown MODE flag\r\n";}

void IRCReplies::srv_rpl(std::vector<std::string> &params, int code)
{
	// currently unused
	switch (code)
	{
		case 331:
			rpl_notopic(params[1]);
			break;
		case 332:
			rpl_topic(params[1], params[2]);
			break;
		case 341:
			rpl_inviting(params[2], params[1]);
			break;
		default:
			std::stringstream	sss;
			sss << "Unexpected rpl code " << code;
			Logger::log(CRITICAL, sss.str().c_str());
			break;
	}
}

void IRCReplies::srv_err(PocketParser& data, int code)
{
	switch (code)
	{
		case 401:
			err_nosuchnick(data.get_nick());
			break;
		case 403:
			err_nosuchchannel(data.get_chan());
			break;
		case 421:
			err_unknowncommand(data.get_cmd());
			break;
		case 433:
			err_nicknameinuse(data.get_nick());
			break;
		case 441:
			err_usernotinchannel(data.get_nick(), data.get_chan());
			break;
		case 442:
			err_notonchannel(data.get_chan());
			break;
		case 443:
			err_userinchannel(data.get_nick(), data.get_chan());
			break;
		case 461:
			err_needmoreparams(data.get_cmd());
			break;
		case 462:
			err_alreadyregistered();
			break;
		case 471:
			err_channelisfull(data.get_client_nick(), data.get_chan());
			break;
		case 472:
			err_unknownmode(data.get_flag());
			break;
		case 473:
			err_inviteonlychan(data.get_client_nick(), data.get_chan());
			break;
		case 475:
			err_badchannelkey(data.get_client_nick(), data.get_chan());
			break;
		case 482:
			err_chanoperprivsneeded(data.get_chan());
			break;
		case 501:
			err_umodeunknownflag();
			break;
		default:
			std::stringstream	sss;
			sss << "Unexpected err code " << code;
			Logger::log(CRITICAL, sss.str().c_str());
			break;
	}
}

void IRCReplies::srv_answer(Client &client, std::vector<std::string> &params, int code)
{
	_ss.str(std::string());
	_ss.clear();
	PocketParser	data;
	data.parse(client, params, code);
	if (code < 400)
		srv_rpl(params, code);
	else if (code >= 400)
		srv_err(data, code);
	client.send_message(_ss.str());
}

const std::string IRCReplies::rpl_pong(void)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":GrosseEtable PONG GrosseEtable\r\n";
	return (_ss.str());
}

const std::string	IRCReplies::rpl_join(const std::string &nick, const std::string &channel)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":" << nick << "@GrosseEtable JOIN " << channel << "\r\n";
	return (_ss.str());
}

const std::string	IRCReplies::rpl_welcome(const std::string &nick)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":GrosseEtable 001 " + nick + " : Welcome to the BIG stinky stable " + nick + "\r\n";
	return (_ss.str());
}

const std::string	IRCReplies::rpl_myinfo(const std::string &nick)
{
	(void) nick;
	return ("not implemented");
}

const std::string IRCReplies::rpl_notopic(const std::string &channel)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":*@GrosseEtable TOPIC " << channel << " :No topic is set\r\n";
	return (_ss.str());
}

const std::string IRCReplies::rpl_topic(const std::string &channel, const std::string &topic)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":*@GrosseEtable TOPIC " << channel << " :" << topic << "\r\n";
	return (_ss.str());
}

// dummy answer to stop irssi from flooding [mode user +i]
const std::string	IRCReplies::rpl_umodeis(const std::string &nick)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":GrosseEtable MODE " << nick << " +i\r\n";
	return (_ss.str());
}

// dummy answer to stop irssi from flooding [mode user +i]
const std::string	IRCReplies::rpl_modechangewarn(const std::string &nick)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":GrosseEtable 221 " << nick << " :+i" << "\r\n";
	return (_ss.str());
}

const std::string	IRCReplies::rpl_inviting(const std::string &channel, const std::string &nick)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":GrosseEtable INVITE " << nick << " " << channel << "\r\n";
	return (_ss.str());
}

const std::string	IRCReplies::rpl_part(Client &client, const std::string &channel, const std::string &reason)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":" << client.get_nickname() << "!" << client.get_username() << "@";
	_ss << "GrosseEtable" << " PART " << channel << " : " << reason << "\r\n";
	return (_ss.str());
}
const std::string	IRCReplies::rpl_part(Client &client, const std::string &channel)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":" << client.get_nickname() << "!" << client.get_username() << "@";
	_ss << "GrosseEtable" << " PART " << channel << "\r\n";
	return (_ss.str());
}

const std::string	IRCReplies::rpl_quit(const std::string &nick, const std::string &reason)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":" << nick << "@GrosseEtable QUIT : " << reason << "\r\n";
	return (_ss.str());
}

const std::string	IRCReplies::rpl_kick(const Client &client, const std::string &nick, const std::string &channel, const std::string &reason)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":" << client.get_nickname() << "@GrosseEtable KICK " << channel << " " << nick << " :" << reason << "\r\n";
	return (_ss.str());
}

const std::string IRCReplies::err_kickedfromchan(const Client &client, const std::string &nick, const std::string &channel)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":" << client.get_nickname() << "@GrosseEtable KICK " << channel << " " << nick << " :You have been kicked from channel\r\n";
	return (_ss.str());
}

const std::string IRCReplies::err_userinchannel_join(const std::string &nick, const std::string &channel)
{
	_ss.str(std::string());
	_ss.clear();
	_ss << ":GrosseEtable " << nick << " " << channel << " :is already on channel\r\n";
	return (_ss.str());
}
