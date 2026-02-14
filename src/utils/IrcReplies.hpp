#pragma once
#include "../client/Client.hpp"
#include "../utils/PocketParser.hpp"
#include <string>
#include <sstream>

class IRCReplies {
private:
	static std::stringstream _ss;

	static void	err_nosuchnick(const std::string &nick);
	static void	err_nosuchchannel(const std::string &channel);
	static void err_unknowncommand(const std::string &command);
	static void	err_nicknameinuse(const std::string &nick);
	static void	err_usernotinchannel(const std::string &nick, const std::string &channel);
	static void err_notonchannel(const std::string &channel);
	static void err_userinchannel(const std::string &nick, const std::string &channel);
	static void	err_needmoreparams(const std::string &command);
	static void	err_alreadyregistered(void);
	static void err_channelisfull(const std::string &client_nick, const std::string &channel);
	static void err_unknownmode(const std::string &mode);
	static void err_inviteonlychan(const std::string &client_nick, const std::string &channel);
	static void err_badchannelkey(const std::string &client_nick, const std::string &channel);
	static void err_chanoperprivsneeded(const std::string &channel);
	static void	err_umodeunknownflag(void);
	
	// see here for corresponding numbers
	static void	srv_rpl(std::vector<std::string> &params, int code);
	static void	srv_err(PocketParser& data, int code);

public:
	static void srv_answer(Client &client, std::vector<std::string> &params, int code);

	// standalone to be called for simple responses (or broadcasts)
	static const std::string	rpl_pong(void);
	static const std::string	rpl_join(const std::string &nick, const std::string &channel);
	static const std::string	rpl_welcome(const std::string &nick);
	static const std::string	rpl_myinfo(const std::string &nick);
	static const std::string	rpl_topic(const std::string &channel, const std::string &topic);
	static const std::string	rpl_notopic(const std::string &channel);
	static const std::string	rpl_umodeis(const std::string &nick);
	static const std::string	rpl_modechangewarn(const std::string &nick);
	static const std::string	rpl_inviting(const std::string &channel, const std::string &nick);
	static const std::string	rpl_part(Client &client, const std::string &channel, const std::string &reason);
	static const std::string	rpl_part(Client &client, const std::string &channel);
	static const std::string	rpl_kick(const Client &client, const std::string &nick, const std::string &channel, const std::string &reason);
	static const std::string	rpl_quit(const std::string &nick, const std::string &reason);
	static const std::string	err_kickedfromchan(const Client &client, const std::string &nick, const std::string &channel);
	static const std::string	err_userinchannel_join(const std::string &nick, const std::string &channel);
};
