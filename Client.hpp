/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/31 17:35:07 by mskinner          #+#    #+#             */
/*   Updated: 2021/06/01 16:36:27 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

# include "Config.hpp"
# include "Request.hpp"
# include "Response.hpp"

struct	t_location;
struct	t_server;
class Config;
class Socket;
class Request;
class Client
{
private:
	int					_fd;
	int					_status;
	unsigned short		_port;
	std::string			_host;
	struct sockaddr_in	_address;
	in_addr_t			_inet;
	Socket				*_listen_sock;
	std::string			_to_parse;
	Request				*_request;
	Response			*_response;
	long				_time_start;



public:
	/*
	Statuses:
		INIT - in a constructor;
		ALIVE - connection accepted
		NOT_DONE - response sent in progress
		DONE - response sent and nothing left (send returns >= 0) or if recv returns 0 or if send returns (<0)
		EMPTY - client without request
	*/

	enum status {
		INIT,
		ALIVE,
		DONE,
		NOT_DONE,
		EMPTY
	};

	Client(Socket *listen_sock);
	~Client();

	void	accept_connection(void);
	void	read_run_request(const int i);

	int		get_fd(void);
	int		get_s_addr(void);
	Request	*get_request(void);
	Response *get_response(void);
	void	set_response(t_server *server);
	void	set_starttime(void);
	void	clear_client(void);
	int		get_status(void) const;
	void	set_status(int status);
	long	get_start_time(void) const;
	void	verify_request_timeout(int timeout_client);
};

#endif
