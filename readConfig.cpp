/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/03/23 11:16:22 by mskinner          #+#    #+#             */
/*   Updated: 2021/06/03 19:15:40 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(void) : _servers() {};

Config::Config(const Config &copy) {*this = copy;};

Config	Config::operator=(const Config &other) {
		if (this != &other)
			_servers = other._servers;
		return (*this);
};

std::vector<std::string>	Config::parse_line(std::string &line) {
	std::vector<std::string>	res;
	//positions of parameter description symbols in line
	size_t						begin;
	size_t						end;

	end = 0;
	while (end < line.length()) {
		begin = line.find_first_not_of(SPACES, end);
		if (begin == std::string::npos)
			return (res);
		end = line.find_first_of(SPACES, begin);
		if (end == std::string::npos)
			end = line.length();
		res.push_back(line.substr(begin, (end - begin)));
	}
	return (res);
};

//Need to clear server and location after each push
void		Config::clear_server(t_server &server) {
	server.error_page.clear();
	server.host = "";
	server.names.clear();
	server.port.clear();
	server.location.clear();
};

void		Config::clear_location(t_location &location) {
	location.auth.clear();
	location.auto_index = false;
	location.cgi = "";
	location.cgi_path = "";
	location.index.clear();
	location.max_body = -1;
	location.methods.clear();
	location.php_path = "";
	location.root = "";
	location.uri = "";
};

//Lists for parsing
// Необходимые методы: GET, HEAD, PUT, POST
std::string const	Config::_methods[] = {
	"GET",
	"HEAD",
	"POST",
	"PUT"
};

void	Config::fix_last_uri_slash(std::string &s) {
	if (s.length() > 1)
		s = s.length() == s.rfind("/") + 1 ?  s.substr(0, s.rfind("/")) : s;
}

/*
** File content line by line is parsed into server
** or server location parameters
** In case of no location provided error message is shown
** ascii 35 #, 123 {, 125 }, 61 =
*/
int		Config::parse_configuration_file(std::vector<std::string> &file_lines) {
	std::vector<std::string>::iterator	it;
	std::vector<std::string>			tokens;
	t_server							server;
	t_location							location;

	if (!verify_brackets(file_lines))
		return (EXIT_FAILURE);
	for (it = file_lines.begin(); it != file_lines.end(); ++it) {
		tokens = parse_line(*it);
		if ((!tokens.size()) || (tokens[0][0] == 35)
			|| ((tokens[0] == SERVER) && (tokens[1][0] == 123)))
			continue ;
		if (tokens[0] == LOCATION) {
			if (((tokens.size() == 3) && (tokens[2][0] == 123))
				|| ((tokens.size() == 2) && (tokens[1][tokens[1].length() - 1] == 123))
				|| ((tokens.size() == 4) && (tokens[1][0] == 61) && (tokens[3][0] == 123))) {
				location.uri = tokens[1][0] != 61 ? tokens[1] : tokens[2];
				fix_last_uri_slash(location.uri);//all uri in saved locations does not contain last slash
				it++;
				tokens = parse_line(*it);
				while (tokens[0][0] != 125) {
					if (parse_servers_locations(tokens, location))
						return (EXIT_FAILURE);
					it++;
					tokens = parse_line(*it);
				}
				server.location.push_back(location);
				clear_location(location);
			}
			else {
				error_message("Wrong server location parameters");
				return (EXIT_FAILURE);
			}
			continue ;
		}
		else if (tokens[0][0] != 125) {
			if (parse_servers_configurations(tokens, server))
				return (EXIT_FAILURE);
		}
		if (tokens[0][0] == 125) {
			if (convert_localhost(server.host, server))
				return (EXIT_FAILURE);
			_servers.push_back(server);
			clear_server(server);
		}
	}
	if (!verify_config())
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
};

int		Config::parse_error_pages(t_server &server, std::string err, std::string err_page) {
	size_t	pos = err_page.find_first_of("x.");

	if (pos == std::string::npos) {
		error_message("Error parameter: wrong error file name");
		return (EXIT_FAILURE);
	}
	pos = err_page.find("x") != std::string::npos ? err_page.find("x") : err_page.find(".");
	pos = pos - 1 < 0 ? 0 : pos - 1; 
	if (err.substr(0, pos) != err_page.substr(1, pos)) {
		error_message("Error parameter: wrong error number");
		return (EXIT_FAILURE);
	}
	if (server.error_page.find(err) != server.error_page.end()) {
		error_message("Error parameter: double error number");
		return (EXIT_FAILURE);		
	}
	server.error_page[err] = err_page;
	return (EXIT_SUCCESS);
};

bool	Config::verify_server_name(t_server &server, std::string &s) {
	if (s[0] == '.')
		return (false);
	for (size_t i = 0; i < s.length(); ++i)
		if ((!isalnum(s[i]) && (s[i] != '.'))
			|| ((s[i - 1] == '.') && (s[i] == '.')))
			return (false);
	server.names.push_back(s);
	return (true);
}

/*
** There could be a few servers configurations in one configuration file
** so we are using a vector container to store information
** If there were cases of double setting identification
** we are using the first one value only
** In case of no configuration file provided we are using the default one
** so _servers will always have at least one server inside
** ascii 35 # 59 ; 58 :
*/
int		Config::parse_servers_configurations(std::vector<std::string> &to_parse, t_server &server) {
	if ((to_parse.size() == 1) || ((to_parse.size() > 2) && (to_parse[2][0] != 35) 
		&& ((to_parse[0] != ERR_PAGE) && (to_parse[0] != NAME)))) {
		error_message("Invalid arguments in configurations file");
		return (EXIT_FAILURE);
	}
	if (to_parse[to_parse.size() - 1].back() != 59) {
		error_message("Directive " + to_parse[0] + " is not terminated by ;");
		return (EXIT_FAILURE);
	}
	to_parse[to_parse.size() - 1].erase(to_parse[to_parse.size() - 1].size()-1);
	if (to_parse[0] == NAME) {
		for (size_t i = 1; i != to_parse.size() - 1; ++i) {
			if (!verify_server_name(server, to_parse[i])) {
				error_message("Server name " + to_parse[i] + " is not valid");
				return (EXIT_FAILURE);
			}
		}
	}
	else if (to_parse[0] == PORT) {
		if (verify_port(to_parse[1]))
			server.port.push_back(htons(atoi(to_parse[1].c_str())));
		else if (to_parse[1].find(58) != std::string::npos)
			server.host = to_parse[1];
		else {
			error_message("Invalid arguments in configurations file");
			return (EXIT_FAILURE);
		}
	}
	else if ((to_parse[0] == ERR_PAGE)) {
		for (size_t i = 1; i != to_parse.size() - 1; ++i) {
			if (parse_error_pages(server, to_parse[i], to_parse[to_parse.size() - 1]))
				return (EXIT_FAILURE);
		}
	}
	else
		error_message("Unknown or double parameter");
	return (EXIT_SUCCESS);
};

bool	Config::verify_method(std::string &s) {
	for (int i = 0; i < N_ELEMS(_methods); ++i)
		if (_methods[i] == s)
			return (true);
	return (false);
}

//Searching for provided method among avaliable
int		Config::parse_method(t_location &location, std::string &s) {
	std::vector<std::string>	tmp;

	tmp = split(s, ",");
	for (size_t i = 0; i < tmp.size(); ++i) {
		if ((tmp[i].length()) && (verify_method(tmp[i])))
			location.methods.push_back(tmp[i]);
		else {
			error_message("Methods parameter is invalid");
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

int		Config::parse_autoindex(t_location &location, std::string &s) {
	if (s == "on")
		location.auto_index = true;
	else if (s == "off")
		location.auto_index = false;
	else {
		error_message("Autoindex parameter is invalid");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

bool	verify_directory(std::string &dir) {
	struct stat	info;

	if ((!stat(dir.c_str(), &info)) && (info.st_mode & S_IFDIR))
		return (true);
	return (false);
}

//Verifies that file is accessible and not empty
bool	verify_file(std::string &file_path) {
	int			fd;
	struct stat file_statistics;

	fd = open(file_path.c_str(), O_RDONLY);
	if (errno)
		return (false);
	if (fstat(fd, &file_statistics) == -1) {
		close(fd);
		return (false);
	}
	close(fd);
	return (true);
}

// Directory should be accessible
int		Config::parse_directory(std::string &dir, std::string &s) {
	if (verify_directory(s))
		dir = (tail(s, 1) != "/") ? s + "/" : s;
	else {
		error_message("Invalid arguments in configurations file");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
** Login:pass - both should not be empty and separated by symbol :
** ascii 58 :
*/
int		Config::parse_auth(t_location &location, std::string &s) {
	std::map<std::string, std::string>::iterator	it;
	std::vector<std::string>	tmp;

	tmp = split(s, ":");
	if ((std::count(s.begin(), s.end(), 58) == 1) 
		&& (tmp[0].length()) && (tmp[1].length())
		&& ((it = location.auth.find(tmp[0])) == location.auth.end()))
		location.auth.insert(std::pair<std::string, std::string>(tmp[0], tmp[1]));
	else {
		error_message("Directive auth is invalid");
		return (EXIT_FAILURE);	
	}
	return (EXIT_SUCCESS);
}

int		Config::parse_location_index(t_location &location, std::string &s) {
	std::vector<std::string>	tmp;

	tmp = split(s, ",");
	for (size_t i = 0; i < tmp.size(); ++i) {
		if (tmp[i].length())
			location.index.push_back(tmp[i]);
		else {
			error_message("Index parameter is invalid");
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);	
}

/*
** (none) If no modifiers are present, the location is interpreted as a prefix match.
** This means that the location given will be matched against the beginning 
** of the request URI to determine a match.
** =: If an equal sign is used, this block will be considered a match 
** if the request URI exactly matches the location given
*/
//ascii 123 { 46 . 35 #
int		Config::parse_servers_locations(std::vector<std::string> &to_parse, t_location &location) {
	if ((to_parse.size() == 1)
		|| ((to_parse.size() > 2) && (to_parse[2][0] != 35)
		&& (to_parse[0] != METHOD) && (to_parse[0] != INDEX))) {
		error_message("Invalid arguments in configurations file");
		return (EXIT_FAILURE);
	}
	if (to_parse[to_parse.size() - 1].back() != 59) {
		error_message("Directive " + to_parse[0] + " is not terminated by ;");
		return (EXIT_FAILURE);
	}
	//removing the closing symbol ;
	to_parse[to_parse.size() - 1].erase(to_parse[to_parse.size() - 1].size()-1);
	if ((to_parse[0] == METHOD) && (!location.root.length())) {
		//this value is no longer needed, so using it as a temporary value holder
		to_parse[0].clear();
		for (size_t i = 1; i < to_parse.size(); ++i)
			//concatenating parts to one line for futher parsing
			to_parse[0] += to_parse[i];
		if (parse_method(location, to_parse[0]))
			return (EXIT_FAILURE);
	}
	else if ((to_parse[0] == ROOT) && (!location.root.length())) {
		if (parse_directory(location.root, to_parse[1]))
			return (EXIT_FAILURE);
	}
	else if ((to_parse[0] == INDEX) && (!location.index.size())) {
		to_parse[0].clear();
		for (size_t i = 1; i < to_parse.size(); ++i)
			to_parse[0] += to_parse[i];
		if (parse_location_index(location, to_parse[0]))
			return (EXIT_FAILURE);
	}
	else if ((to_parse[0] == CGI_PATH) && (!location.cgi_path.length()))
		location.cgi_path = to_parse[1];
	else if ((to_parse[0] == PHP_PATH) && (!location.php_path.length()))
		location.php_path = to_parse[1];
	else if ((to_parse[0] == CGI) && (!location.cgi.length()) && (to_parse[1][0] == '.')
		&& (std::count(to_parse[1].begin(), to_parse[1].end(), 46) == 1)) //file extention verification
		location.cgi = to_parse[1];
	else if ((to_parse[0] == AUTO_INDEX) && (location.auto_index == false)) {
		if (parse_autoindex(location, to_parse[1]))
			return (EXIT_FAILURE);
	}
	else if ((to_parse[0] == MAX_BODY) && (location.max_body == -1)
		&& (is_all_numbers(to_parse[1]))) {
		try {
			location.max_body = std::stoi(to_parse[1]);
		}
		catch(const std::exception& e) {
			error_message(e.what());
			return (EXIT_FAILURE);
		}
		if (location.max_body < 0)
			return (EXIT_FAILURE);
	}
	//There could be defined several location users
	else if (to_parse[0] == AUTH) {
		if (parse_auth(location, to_parse[1]))
			return (EXIT_FAILURE);
	}
	else {
		error_message("Unknown or double parameter");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
};

//Verify that all brackets were closed
bool		Config::verify_brackets(std::vector<std::string> &lines)
{
	std::stack<std::string>		to_check;

	for (size_t i = 0; i != lines.size(); i++)
	{
		for (size_t j = 0; j != lines[i].size(); j++)
		{
			if (lines[i][j] == '{')
				to_check.push(&lines[i][j]);
			else if (lines[i][j] == '}' && !to_check.empty())
				to_check.pop();
			else if (lines[i][j] == '}' && to_check.empty())
			{
				std::cerr << "Invalid brackets\n";
				return (false);
			}
		}
	}
	if (to_check.size())
	{
		std::cerr << "Invalid brackets\n";
		return (false);
	}
	return (true);
};

/*
** Verifies:
** - check all provided files accessibility
** - uri starts with '/' only
** - localhost is filled
** - ports are defined
** - ports are not the same within one server
** - check error pages order
** ascii 47 /
*/

bool	Config::verify_config() {
	std::list<unsigned short>::iterator it;
	size_t		qty = 0;
	std::string	tmp;

	for (size_t i = 0; i != _servers.size(); i++)
	{
		if (!_servers[i].host.length())
			_servers[i].host = LOCALHOST_IP;
		if (!_servers[i].port.size()) {
			error_message("Port is not set in configuration file");
			return (false);
		}
		_servers[i].port.sort();
		for (it = _servers[i].port.begin(); it != --(_servers[i].port.end()); it++) {
			if (*it == *(++it)) {
				error_message("Ports double in configuration file");
				return (false);
			}
		}
		for (size_t j = 0; j != _servers[i].location.size(); ++j) {
			if (_servers[i].location[j].uri.front() != 47) {
				error_message("Location name should start from /");
				return (false);
			}
			if (_servers[i].location[j].index.size()) {
				for (std::size_t j = 0; j != _servers[i].location[j].index.size(); ++j) {
					tmp = _servers[i].location[j].root + _servers[i].location[j].index[j];
					if (!verify_file(tmp)) {
						error_message("File is not accessible: " + tmp);
						return (false);
					}
				}
			}
			if (_servers[i].location[j].cgi_path.length()) {
				tmp = _servers[i].location[j].cgi_path;
				if (!verify_file(tmp)) {
					error_message("File is not accessible: " + tmp);
					return (false);
				}
			}
			std::map<std::string, std::string>::iterator	it_err = _servers[i].error_page.begin();

			for (; it_err != _servers[i].error_page.end(); ++it_err) {
				if (_servers[i].location[j].uri == (*it_err).second) {
					if (tail(_servers[i].location[j].root, 1) != "/")
						(*it_err).second = _servers[i].location[j].root + "/" + (*it_err).first 
							+ (*it_err).second.substr((*it_err).second.find("."), (*it_err).second.length());
					else
						(*it_err).second = _servers[i].location[j].root + (*it_err).first 
							+ (*it_err).second.substr((*it_err).second.find("."), (*it_err).second.length());
					if (!verify_file((*it_err).second)) {
						error_message("File is not accessible: " + (*it_err).first + " error page");
						return (false);
					}
					qty++;
				}
			}
		}
		if (qty != _servers[i].error_page.size()) {
			error_message("Error pages location was not set in configuration file");
			return (false);
		}
		qty = 0;
	}
	return (true);
};
