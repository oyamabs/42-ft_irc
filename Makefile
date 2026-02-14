CC = c++
# -fno-omit-frame-pointer is to prevent malloc stacktraces from being truncated,
# see "My malloc stacktraces are too short" here:
# https://github.com/google/sanitizers/wiki/AddressSanitizer
SANITIZERS = -fsanitize=address,undefined -fno-omit-frame-pointer
ifeq ($(CFLAGS),)
	CFLAGS = -Wall -Wextra -Werror -g -O2 -std=c++98
endif

UTILS_SRC = src/utils/IrcReplies.cpp 	\
			src/utils/PocketParser.cpp	\

SERVER_SRC = src/main.cpp				\
			 src/server/Server.cpp		\
			 src/channel/Channel.cpp	\
			 src/client/Client.cpp		\

LOGGER_SRC = src/logger/Logger.cpp		\

SOURCEFILES = $(UTILS_SRC) $(SERVER_SRC) $(LOGGER_SRC)

OBJECTS = $(SOURCEFILES:.cpp=.o)
NAME = ircserv 
DEPS = $(OBJECTS:.o=.d)

.PHONY: all clean fclean bonus re sane

all: $(OBJECTS) $(NAME)

-include $(DEPS)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)

%.o: %.cpp
	$(CC) -c $(CFLAGS) -o $*.o $*.cpp
	$(CC) -MM $(CFLAGS) -MT $*.o $*.cpp > $*.d

clean:
	find . -name '*.o' -print -delete
	find . -name '*.d' -print -delete

fclean: clean
	rm -f $(NAME)

# bonus: CFLAGS += -D BONUS=1
# bonus: all
# 
# bonus-sane: CFLAGS += -D BONUS=1 $(SANITIZERS)
# bonus-sane: all
sane: CFLAGS += $(SANITIZERS)
sane: all

re:
	+make fclean
	+make all

