#pragma once

/*
# define FORMAT \
	std::cout << "[" __FILE_NAME__ << ":" << __LINE__ << "] ";

# define LOG(type, message) \
	FORMAT \
	std::cout << type << " : " << message << std::endl;
*/

enum LogLevel {
	DEBUG,
	INFO,
	WARNING,
	CRITICAL
};

class Logger {
public:
	Logger();
	Logger(const Logger &logger);
	~Logger();
	virtual Logger &operator=(const Logger &logger) = 0;
	static void log(LogLevel loglevel, const char *message);
};
