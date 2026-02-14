# ft_irc
The instant messaging protocol of the future!

## Goal
ft_irc is a project that aims to implement a server the IRC protocol. We used [IRSSI](https://irssi.org/) as the reference client.

## Grade

![Grade](https://img.shields.io/badge/Grade-125-darkgreen)

## Tech stack

- Programming is done with C++ 98
- Testing is done with IRSSI
- Debugging is done with Netcat

## Limits

We only had one use of the `poll()` function and the server had to at least implement basic commands such as `JOIN`, `PRIVMSG`, `KICK`, `MODE`...

## How we implemented it ?

We started by creating a Server class that is the core of our program. The server handles and manages connections while also handling incoming messages.

We quickly after implemented the client, that queues the messages as we discovered IRSSI sends multiple commands in one "batch". We then implemented commands and their various error and replies to send to the clients.

After having a working server we did the bonus part. my mate implemented the bot and I was learning how file transfer with IRC works.

## Special thanks

- cczerwin (my mate for the project)

## How to use it ?

- Clone the repo `git clone https://github.com/oyamabs/42-ft_irc`
- Build the program `make`
- Launch it `./ircserv <port> <password>`
- Use your IRC client of choice. For example with IRSSI connect with the following command `/connect <your ip> <port you used for the server> <your password>`
