/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tchampio <tchampio@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 13:23:30 by tchampio          #+#    #+#             */
/*   Updated: 2026/01/31 18:28:11 by tchampio         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "logger/Logger.hpp"
#include "server/Server.hpp"
#include <csignal>
#include <cstdlib>
#include <signal.h>
#include <iostream>
#include <sstream>

bool server_running = true;

void	signal_handler(int sig)
{
	std::stringstream	ss;
	ss << "\r[CRIMINAL] Got signal " << sig << ", killing server";
	Logger::log(CRITICAL, ss.str().c_str());
	server_running = false;
}

int	main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cout << "Expected parameters: ./ircserv <port> <password>" << std::flush;
		return (0);
	}
	struct sigaction sa;
	sa.sa_handler = signal_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	try {
		Server server(atoi(av[1]), av[2]);
		server.listen_activity();
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}
}
