/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/23 14:10:05 by mskinner          #+#    #+#             */
/*   Updated: 2021/06/03 20:38:07 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIG_HPP_
# define _CONFIG_HPP_

//Configuration
# include <stdlib.h>
# include <limits>
# include <iostream>
# include <string>
# include <errno.h>
# include <fcntl.h>
# include <unistd.h>
# include <vector>
# include <stack>
# include <list>
# include <map>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <cmath>

//Server
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include "Client.hpp"
# include "Socket.hpp"

//Response
# include <fstream>
# include <sstream>

//GET NEXT LINE
# define	BUFFER_SIZE	1024
# define	SEND_BUFFER	1000000

//PARSER CHARACTERS SET
# define	SPACES		" \f\n\r\t\v"

//DEFAULT VALUES
# define	CONFIG		"./content/default"

// COLORS
# define BLACK "\e[1;30m" 
# define RED "\e[0;31m"
# define GREEN "\e[1;32m"
# define BROWN "\e[0;33m" 
# define BLUE "\e[0;34m"
# define MAGENTA "\e[0;35m"
# define CYAN "\e[0;36m"
# define GRAY "\e[0;37m"
# define RESET "\e[m"

//SERVER KEYS
# define	SERVER		"server"
// The parameter to server_name can be a full (exact) name, a wildcard, or a regular expression.
// REGEX is not the subject of this project
// If the Host header field does not match a server name, NGINX Plus routes
// the request to the default server for the port on which the request arrived.
# define	NAME 		"server_name"
# define	ERR_PAGE 	"error_page"
// To make an internal redirect when a file is not found.
// we can return a custom page along with an error code,
// substitute a different error code in the response, or redirect the browser to a different URI
# define	PORT 		"listen" 
// If a port is omitted, 
// the standard port is used. 
// Likewise, if an address is omitted, the server listens on all addresses. 
// If the listen directive is not included at all, 
// the “standard” port is 80/tcp and the “default” port is 8000/tcp
// Ports cannot be the same within one server
# define	LOCALHOST		"localhost"
# define	LOCALHOST_IP	"127.0.0.1"
# define	ZEROES_IP		"0.0.0.0"
# define	BROADCAST_IP	"255.255.255.255"

//LOCATION KEYS
# define	LOCATION	"location"
// Inside each location block, it is usually possible (with a few exceptions)
// to place even more location directives to further refine the processing for specific groups of requests.
// In the location:  prefix strings (pathnames /) and regular expressions (~) or case-insensitive (~*)
# define	METHOD 		"methods"
# define	ROOT 		"root"
# define	INDEX 		"index"
// The index directive defines the index file’s name (the default value is index.html).	
# define	CGI_PATH 	"cgi_path"
# define	PHP_PATH 	"php_path"
# define	CGI 		"cgi"
# define	AUTO_INDEX 	"autoindex"
# define	MAX_BODY 	"max_body" // should limit client body size
# define	AUTH 		"auth"

//For iteration of static lists - elements quantity getter
# define	N_ELEMS(a) (int)(sizeof(a) / sizeof((a)[0]))
# define	BASE64 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

struct									t_location
{		
	std::vector<std::string>			index;
	std::string							root;
	std::vector<std::string>			methods;
	std::string							uri;
	std::string							php_path;
	std::string							cgi;
	std::string							cgi_path;
	std::map<std::string, std::string>	auth;
	int									auto_index;
	int									max_body;
	t_location() : auto_index(false), max_body(-1) {}
};

class Socket;
class Client;
//Default config parsing is used instead of initiation function
struct									t_server
{
	std::vector<std::string>			names;
	std::map<std::string, std::string>	error_page;
	std::vector<t_location> 			location;
	std::string							host;
	std::list<unsigned short>			port;
	long								time_start;
	Socket								*socket;
	std::list<Client*> 					clients;
};

//Global server configuration parameters structure
extern std::vector<t_server*>		g_servers;

class Config {
private:
	std::vector<t_server>		_servers;
	//Constant values for parsing
	static std::string const 	_methods[];

public:
	Config(void);
	Config(const Config &copy);
	~Config(void) {};
	Config	operator=(const Config &other);

	int		parse_configuration_file(std::vector<std::string> &file_lines);
	void	init_servers_configuration(void);

private:
	void						init_headers_set(void);
	std::vector<std::string>	parse_line(std::string &line);
	int							parse_servers_configurations(std::vector<std::string> &to_parse, t_server &server);
	int							parse_servers_locations(std::vector<std::string> &to_parse, t_location &location);
	void						clear_server(t_server &server);
	void						clear_location(t_location &location);
	bool						verify_brackets(std::vector<std::string> &lines);
	bool						verify_config(void);
	std::string					replace_localhost(std::string &s);
	bool						verify_localhost(std::string &s);
	bool						verify_port(std::string &s);
	int							convert_localhost(std::string &s, t_server &server);
	bool						verify_method(std::string &s);
	bool						verify_server_name(t_server &server, std::string &s);
	int							parse_method(t_location &location, std::string &s);
	int							parse_autoindex(t_location &location, std::string &s);
	int							parse_directory(std::string &dir, std::string &s);
	int							parse_auth(t_location &location, std::string &s);
	int							parse_error_pages(t_server &server, std::string err, std::string err_page);
	int							parse_location_index(t_location &location, std::string &s);
	void						fix_last_uri_slash(std::string &s);
};

//SERVER.CPP
t_location					*get_location(t_server *server, std::string loc);
std::string					*get_method(t_location &loc, std::string method);

//UTILS.CPP
void						error_message(std::string message);
void						exit_error(int err);
void						exit_error_msg(std::string msg);
void						signal_handler(int signal);
void						signals(void);
long						current_time(void);
std::string					tail(const std::string &s, const size_t length);
std::string					inet_ntoaddr(int n);
std::string					base64_encode(const std::string &s);
std::string					base64_decode(const std::string &s);
bool						is_hex(const std::string &hex);
void						*ft_memset(void *dest, int c, size_t len);
void						*ft_calloc(size_t nmemb, size_t size);
bool						ft_isspace(int c);
bool						ft_isdigit(int c);
bool						ft_isalpha(int c);
bool						is_all_numbers(std::string s);
int							ft_atoi(const char *str);
std::string					read_file(const char *file_path);
std::vector<std::string>	split(const std::string &s, const std::string &delimiter);
void						clear_servers_configuration(void);
bool						verify_directory(std::string &dir);
bool						verify_file(std::string &file_path);

#endif
