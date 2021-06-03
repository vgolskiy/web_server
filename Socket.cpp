/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/15 19:28:49 by mskinner          #+#    #+#             */
/*   Updated: 2021/05/24 16:34:55 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(void) : _fd(-1), _port(0), _opt(1), _host(""), _address() {};

Socket::Socket(const Socket &copy) {*this = copy;};

Socket::~Socket() {}

Socket	Socket::operator=(const Socket &other) {
	if (this != &other) {
		_fd = other._fd;
		_port = other._port;
		_opt = other._opt;
		_host = other._host;
		_address = other._address;
	}
	return (*this);
};

int		Socket::get_fd(void) const {
	return (_fd);
}

// non-blocking - "non-blocking" mode, 
// which means that a read() on a slow file will return immediately, even if no bytes are available.

// протокол HTTP сам по себе синхронный (пакеты отправляются синхронно),
// но при наличии неблокирующего ввода-вывода программа, работающая с HTTP,
// может оставаться асинхронной, то есть успевать совершить какую-либо
// полезную работу между отправкой HTTP-запроса и получением ответа на него.
// Полезная ссылка: https://ps-group.github.io/os/nonblocking_io_posix

/*
** Creating socket file descriptor
** For TCP/IP sockets, we want to specify the IP address family (AF_INET)
** and virtual circuit service (SOCK_STREAM). Since there’s only one form
** of virtual circuit service, there are no variations of the protocol,
** so the last argument, protocol, is zero.
*/
Socket::Socket(int port, std::string host) : _port(port), _opt(1), _host(host) {
    init_socket();

    if((_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) //After that moment we are closing fds' during global structure cleaning
		exit_error(errno);
    if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0)
		exit_error(errno);
}

/*
** In global structure we are storing port in unsigned short format after htons()
** AF_INET is used as we are implementing Internet Protocol v4 addresses only
** INADDR_NONE - the string does not contain a legitimate Internet address
*/
void Socket::init_socket() {
    _address.sin_family = AF_INET;
    _address.sin_port = _port;
	_address.sin_addr.s_addr = inet_addr(_host.c_str());
    if (_address.sin_addr.s_addr == INADDR_NONE)
		exit_error(errno);
}

/*
** Forcefully attaching socket to the port 8080
** Binding an address - assigning a transport address to the socket (a port number in IP networking)
** sockaddr is a generic container that allows the OS to be able to read the first couple of bytes
** that identify the address family, which determines what variant of the sockaddr struct to use
** that contains elements that make sense for that specific communication type. 
** For IP networking, struct sockaddr_in used, which is defined in the header netinet/in.h
** 2) Timeout implementation: https://stackoverflow.com/questions/4181784/how-to-set-socket-timeout-in-c-when-making-multiple-connections
*/
void Socket::to_bind()
{
    if ((setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &_opt, sizeof(_opt)))
		|| (bind(_fd, (struct sockaddr*)&_address, sizeof(_address))))
		exit_error(errno);
}

/*
** Listen tells a socket that it should be capable of accepting incoming connections
** The second parameter (num_ports) defines the maximum number of pending connections
** that can be queued up before connections are refused.
*/
void Socket::to_listen(int num_ports)
{
    if (listen(_fd, num_ports))
		exit_error(errno);
}
