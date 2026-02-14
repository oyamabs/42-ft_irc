#include <iostream>
#include "Logger.hpp"

void Logger::log(LogLevel loglevel, const char *message)
{
	switch (loglevel)
	{
		case DEBUG:
			std::cout << "\e[1;95m" << "[DEBUG] ";
			break;
		case INFO:
			std::cout << "\e[0;37m" << "[INFO] ";
			break;
		case WARNING:
			std::cout << "\e[0;93m" << "[WARNING] ";
			break;
		case CRITICAL:
			std::cout << "\e[0;31m" << "[CRITICAL] ";
			break;
	}
	std::cout << message << "\e[0m" << std::endl;
}
