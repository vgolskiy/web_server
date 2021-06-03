/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/15 00:10:57 by mskinner          #+#    #+#             */
/*   Updated: 2021/06/03 21:44:40 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// int select(int nfds, fd_set *restrict readfds,
     // 	fd_set *restrict writefds, fd_set *restrict exceptfds,
	 //   	struct timeval *restrict timeout); -> pass null to either if not interested

//FD_ZERO()
// This macro clears (removes all file descriptors from) set.
// It should be employed as the first step in initializing a
// file descriptor set.

// FD_SET()
// This macro adds the file descriptor fd to set.  Adding a
// file descriptor that is already present in the set is a
// no-op, and does not produce an error -> check if present by FD_ISSET()

// FD_ISSET()
// select() modifies the contents of the sets according to
// the rules described below.  After calling select(), the
// FD_ISSET() macro can be used to test if a file descriptor
// is still present in a set.  FD_ISSET() returns nonzero if
// the file descriptor fd is present in set, and zero if it
// is not.

/*
int nfds:
Представляет собой целое число, на единицу большее максимального файлового дескриптора 
в любом из наборов. Другими словами, при добавлении файловых дескрипторов в наборы вам 
необходимо вычислять максимальное целое значение любого из них, а затем увеличивать
это значение на единицу и передавать в nfds.
*/

# include "Client.hpp"
# include "Config.hpp"
# include "Request.hpp"

/*
** Starting servers services
** Servers not empty verification
*/
int		start_servers(std::vector<t_server*> &servers)
{
	if (servers.empty())
		return (EXIT_FAILURE);
	
	//initialization should be separate - to avoid segfault in case of (to_bind || to_listen)'s error
	for (size_t i = 0; i < servers.size(); i++)
    {
		unsigned short port = servers[i]->port.front();
		std::string host = servers[i]->host;
		servers[i]->socket = new Socket(port, host);
	}
    for (size_t i = 0; i < servers.size(); i++)
    {
		servers[i]->socket->to_bind();
		servers[i]->socket->to_listen(-1);
		std::cout << "Server " << *(servers[i]->names.begin()) << " ("
			<< servers[i]->host << ')' << " is litening to port " << ntohs(servers[i]->port.front()) << std::endl;
    }
	return (EXIT_SUCCESS);
}

void	add_new_client(std::vector<t_server*> &servers, const fd_set &read_fd_sets) {
    for (size_t i = 0; i < servers.size(); i++) {
        if (FD_ISSET(servers[i]->socket->get_fd(), &read_fd_sets)) {
			Client *new_cl = new Client(servers[i]->socket);
			try {
				new_cl->accept_connection();
				servers[i]->clients.push_back(new_cl);
				std::cout << "Client connected with a " << *(servers[i]->names.begin()) << std::endl;
			}
			catch (int e) {
				error_message("Failed to connect with a client: " + std::string(strerror(e)));
			}
		}
    }
}

// Set socket fds for every server: read_fd_set for servers and both write_and_read_fds for clients
void	set_fds(std::vector<t_server*> &servers, fd_set &read_fd_sets,
				fd_set &write_fd_sets, int &nfds) {
	for (size_t i = 0; i < servers.size(); i++) {
		if (!(FD_ISSET(servers[i]->socket->get_fd(), &read_fd_sets))) {
			if (servers[i]->socket->get_fd() > nfds)
				nfds = servers[i]->socket->get_fd();
			FD_SET(servers[i]->socket->get_fd(), &read_fd_sets);
		}

		std::list<Client*>::iterator it = servers[i]->clients.begin();
		for (; it != servers[i]->clients.end(); ++it) {	
			if (!(FD_ISSET((*it)->get_fd(), &read_fd_sets))) {
				if ((*it)->get_fd() > nfds)
					nfds = (*it)->get_fd();
				FD_SET((*it)->get_fd(), &read_fd_sets);
			}
			if ((*it)->get_request()
				&& (((*it)->get_request()->get_status() == Request::BAD_REQ) 
				|| ((*it)->get_request()->get_status() == Request::DONE))) {
				if (!(FD_ISSET((*it)->get_fd(), &write_fd_sets))) {
					if ((*it)->get_fd() > nfds)
						nfds = (*it)->get_fd();
					FD_SET((*it)->get_fd(), &write_fd_sets);
				}
			}
		}
	}
}

void	delete_clients(std::vector<t_server*> &servers) {
	for (size_t i = 0; i < servers.size(); ++i) {
		std::list<Client*>::iterator it = servers[i]->clients.begin();
		std::list<Client*>::iterator ite = servers[i]->clients.end();
		for (; it != ite; it++) {
			if ((*it)->get_status() == Client::DONE || (*it)->get_status() == Client::EMPTY) {
				delete *it;
				it =  servers[i]->clients.erase(it);
				std::cout << "Client is disconnected!\n";
			}
		}
	}
}

/*
** After select() has returned, readfds will be cleared of
** all file descriptors except for those that are ready for reading
*/
void	deal_request(std::vector<t_server*> &servers,
					const fd_set &read_fd_sets, const fd_set &write_fd_sets) // doesnt work appropriately
{
	int buffer_size, ret;

	for (size_t i = 0; i != servers.size(); i++) {
		std::list<Client*>::iterator it = servers[i]->clients.begin();
		for (; it != servers[i]->clients.end(); ++it) {
			if (FD_ISSET((*it)->get_fd(), &read_fd_sets) && (*it)->get_status() != Client::NOT_DONE) {
				if ((!(*it)->get_request())
					|| (((*it)->get_request()) && ((*it)->get_request()->get_status() < Request::BAD_REQ)))
					(*it)->read_run_request(i);
				if ((*it)->get_status() == Client::EMPTY)
					return ;
			}
			if (FD_ISSET((*it)->get_fd(), &write_fd_sets)) {
				if ((*it)->get_status() == Client::NOT_DONE) {
					buffer_size = (*it)->get_response()->get_response_body().length() > SEND_BUFFER ? SEND_BUFFER : (*it)->get_response()->get_response_body().length();
					ret = send((*it)->get_fd(), (*it)->get_response()->get_response_body().c_str(), buffer_size, 0);
					if (ret < 0) {
						error_message("Failed to send a response. System call error");
						(*it)->set_status(Client::DONE);
					}
					else if ((int)(*it)->get_response()->get_response_body().length() - ret > 0)
						(*it)->get_response()->cut_length(ret);
					else
						(*it)->set_status(Client::DONE);
				}
				if ((*it)->get_status() == Client::ALIVE && (*it)->get_request()
					&& ((*it)->get_request()->get_status() == Request::DONE
					|| (*it)->get_request()->get_status() == Request::BAD_REQ)) {
					(*it)->set_response(servers[i]);
					buffer_size = (*it)->get_response()->get_response_body().length() > SEND_BUFFER ?
						SEND_BUFFER : (*it)->get_response()->get_response_body().length();
					ret = send((*it)->get_fd(), (*it)->get_response()->get_response_body().c_str(), buffer_size, 0);
					if (ret < 0) {
						error_message("Failed to send a response. System call error");
						(*it)->set_status(Client::DONE);
					}
					else if ((int)(*it)->get_response()->get_response_body().length() - ret > 0) {
						(*it)->set_status(Client::NOT_DONE);
						(*it)->get_response()->cut_length(ret);
					}
					else
						(*it)->set_status(Client::DONE);
				}
			}
		}
	}
}

// struct timeval *restrict timeout - specifies the interval that select() should block
// waiting for a file descriptor to become ready - SIGNAL HANDLER?????

int		select_loop(std::vector<t_server*> &servers) {
	fd_set	        read_fd_sets;
	fd_set	        write_fd_sets;
	// Subject: a request to your server should never hang forever -> set timeout
	struct timeval  timeout;
    int             to_select;
	//Variable is ignored. The nfds parameter is included only for compatibility with Berkeley sockets.
	//We can set it to max size of fds = 1024 or calculate in progress
	int				nfds;

	//1) Timeout implementation: https://stackoverflow.com/questions/9847441/setting-socket-timeout
	timeout.tv_sec = 600;
	timeout.tv_usec = 0;
	while (true)
	{
		FD_ZERO(&read_fd_sets);
		FD_ZERO(&write_fd_sets);
		nfds = 0;
    	// setting fds: adding readFD and writeFD for each server
		set_fds(servers, read_fd_sets, write_fd_sets, nfds);
    	// call select
    	to_select = select(nfds + 1, &read_fd_sets, &write_fd_sets, 0, &timeout);
    	if (!to_select)
			exit_error_msg("Timeout of webserver session");
		// error occured -> errno is set to indicate an error
		//https://stackoverflow.com/questions/41474299/checking-if-errno-eintr-what-does-it-mean
		else if ((to_select == -1) && (errno != EINTR))
			exit_error(errno);
    	else if ((to_select == -1) && (errno == EINTR)) {
			errno = 0;
			continue ;
		}
		add_new_client(servers, read_fd_sets);
		deal_request(servers, read_fd_sets, write_fd_sets);
		delete_clients(servers);
	}
	return (EXIT_SUCCESS);
}

t_location	*get_location(t_server *server, std::string loc) {
	std::vector<t_location>::iterator	it;

	for (it = server->location.begin(); it != server->location.end(); ++it) {
		if (((*it).uri == loc) || ((*it).uri + "/" == loc))
			return &(*it);
	}
	return (NULL);
}

std::string	*get_method(t_location &loc, std::string method) {
	std::vector<std::string>::iterator it;

	for (it = loc.methods.begin(); it != loc.methods.end(); ++it) {
		if (*it == method)
			return &(*it);
	}
	return (NULL);
}
