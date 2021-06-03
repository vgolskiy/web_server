/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/06/03 21:13:17 by mskinner          #+#    #+#             */
/*   Updated: 2021/06/03 21:14:21 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(t_server *server, Request *request)
	: _request(request) {
	_status_code = _request->get_status_code() ? _request->get_status_code() : 200;
	set_status();
	_response = "";
	_method = _request->get_method();
	_body = "";
	_loc = get_location(server, _request->get_location_name());
	_param = _request->get_uri_parameters();
	_authorize = _request->get_authorization();
	_error_page = server->error_page;
	_content_len = _request->get_content_length();
	_requested_file = _request->get_requested_file();
	_subfolder = _request->get_subfolder();
	if (_subfolder.length()) {
		if (_subfolder[_subfolder.length() - 1] != '/')
			_subfolder += "/";
	}
	_server_name = *(server->names.begin());
	
	std::vector<std::string> v;
	const char* ss[4] = {".gif", ".jpeg", ".jpg", ".png"};
	for (int i = 0; i < 4; ++i)
		v.push_back(ss[i]);
	_content_types["image/gif"] = v;
	v.clear();
	v.push_back(".mp4");
	_content_types["video/mpeg"] = v;
	v.clear();
	v.push_back(".woff");
	_content_types["font/woff"] = v;
	v.clear();
	v.push_back(".woff2");
	_content_types["font/woff2"] = v;

	std::string tmp[] = {
		"Allow",
		"Content-Language",
		"Content-Length",
		"Content-Location",
		"Content-Type",
		"Host",
		"Last-Modified",
		"Server",
		"WWW-Authenticate"};
	for (int i = 0; i < 9; ++i)
		_headers_sequence.push_back(tmp[i]);
}

Response::~Response() {}

std::string Response::get_response_body(void) { return (_response); }

void Response::cut_length(int size)
{
	_response = _response.substr(size, _response.length());
}

std::string Response::get_server_date(void)	{
	struct tm		info;
	struct timeval	time;
	char			buf[29];
	std::string		s;

	gettimeofday(&time, NULL);
	s = std::to_string(time.tv_sec);
	strptime(s.c_str(), " %s ", &info);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %X", &info);
	s = buf;
	s += " GMT";
	return (s);
}

std::string Response::get_last_modified_date(std::string file) {
	struct tm	info;
	char		buf[29];
	struct stat	st;
	std::string	s;

	if (stat(file.c_str(), &st) < 0)
		return ("");
	s = std::to_string(st.st_mtimespec.tv_sec);
	strptime(s.c_str(), " %s ", &info);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %X", &info);
	s = buf;
	s += " GMT";
	return (s);
}

void Response::get_status_line(void) {
	_response = HTTP + std::string(" ") + std::to_string(_status_code)
		+ std::string(" ") + _status[_status_code];
	_response += CRLF;
}

void Response::fill_response_body(void) {
	std::list<std::string>::iterator it;

	for (it = _headers_sequence.begin(); it != _headers_sequence.end(); ++it) {
		if (_headers[*it] != "")
			_response += *it + ": " + _headers[*it] + CRLF;
	}
	_response += CRLF;
	return ;
}

//Need to read page body once more because of error while reading
std::string	Response::get_page_body(void) {
	std::string res;

	res = read_page_body();
	if ((!res.length()) && (_status_code != 200)) {
		_response.clear();
		get_status_line();
		fill_response_body();
		res = (_method != "HEAD") ? read_page_body() : "";
	}
	return (res);
}

/*
** Cases:
** - have requested file name, so just add subfolder and get it
** - have autoindex mark - create index on the way and return it
** - have subfolder only - verify that subfolder contains at least one file from index and return it,
** else return status code 404 with excuses
** - have location only - return the first one index
*/
std::string	Response::read_page_body(void) {
	std::string		res = "";
	std::string		file = "";
	std::stringstream ss;
	DIR*			directory;

	if (_status_code == 200) {
		if (_requested_file.length())
			file = _loc->root + _subfolder + _requested_file;
		else if (_loc->auto_index) {
			create_autoindex();
			return (_body);
		}
		else if ((_subfolder.length())
			&& ((directory = opendir((_loc->root + _subfolder).c_str())))) {
			struct dirent	*ent;
			std::vector<std::string>::iterator	it;

			while ((ent = readdir(directory)) && (!file.length())) {
				for (it = _loc->index.begin(); it != _loc->index.end(); ++it) {
					if (*it == ent->d_name) {
						file = _loc->root + *it;
						closedir(directory);
						break ;
					}
				}
			}
			if (!file.length()) {
				closedir(directory);
				_status_code = 404;
				_response.clear();
				get_status_line();
				fill_response_body();
				return ("Sorry, this is a directory");
			}
		}
		else
			file = _loc->root + _loc->index[0];
	}
	else
		file = _error_page[std::to_string(_status_code)];

	std::ifstream	inf(file);

	if (!inf) {
		_status_code = 404;
		return (res);
	}
	ss << inf.rdbuf();
	res += ss.str();
	res += CRLF_2X;
	inf.close();
	return (res);
}

void	Response::create_autoindex() {
	//root - is a full path for a current location
	DIR *directory = opendir((_loc->root + _subfolder).c_str());
	if (!directory)
		return ;

	struct dirent *curr;

	_body += HTML_TITLE;
	_body += HTML_HEADER;
	while ((curr = readdir(directory))) {
		if (curr->d_name[0] != '.') {
			_body += HYPER_REF;
			_body += _loc->uri;
			_body += "/";
			if (_subfolder.length())
				_body += _subfolder;
			_body += curr->d_name;
			_body += "\">";
			_body += curr->d_name;
			_body += HYPER_END;
		}
	}
	_body += HTML_CLOSE;
	closedir(directory);
}

std::string	Response::get_content_type() {
	std::string	tmp;
	if (_requested_file.length()) {
		if (_requested_file.rfind(".") != std::string::npos) {
			tmp = _requested_file.substr(_requested_file.rfind("."));

			std::map<std::string, std::vector<std::string> >::iterator	itm;
			for (itm = _content_types.begin(); itm != _content_types.end(); ++itm) {
				std::vector<std::string>::iterator	itv;

				for (itv = (*itm).second.begin(); itv != (*itm).second.end(); ++itv) {
					if (tmp == (*itv))
						return ((*itm).first);
				}
			}
		}
		else
			return ("application/octet-stream");
	}
	return ("text/html");
}

//Verification of opened credentials vs opened user and encrypted password in location
bool Response::auth_by_uri_param(void) {
	std::map<std::string, std::string>::iterator	it;
	std::string					usr, psw;

	if (_param.length()) {
		if (_param.find("user=") != std::string::npos) {
			usr = _param.substr(_param.find("user=") + 5);
			psw = usr.substr(usr.find("&") + 1);
			if (!psw.find("password=")) {
				usr = usr.substr(0, usr.find("&"));
				psw = split(psw.substr(9, std::string::npos), "&")[0];
				psw = base64_encode(psw);
				if (((it = _loc->auth.find(usr)) != _loc->auth.end())
					&& ((*it).second == psw))
					return (true);
			}
		}
	}
	return (false);
}

//Verification of closed credentials vs opened user and encrypted password in location
bool Response::auth_by_header(void) {
	std::map<std::string, std::string>::iterator	it;
	std::vector<std::string>	pair;
	std::string					tmp;

	if (_authorize.length()) {
		tmp = base64_decode(_authorize);
		pair = split(tmp, ":");
		if (((it = _loc->auth.find(pair[0])) != _loc->auth.end())
					&& ((*it).second == base64_encode(pair[1])))
			return (true);
	}
    return (false);
}

void Response::put_method()
{
	std::string path = _loc->root + _subfolder + _requested_file;
	int to_ret;
	int fd;
	struct stat	buf;

	to_ret = stat(path.c_str(), &buf);
	fd = open(path.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);
	_status_code = (to_ret == 0) ? 200 : 201;
	write(fd, _request->get_body().c_str(), _request->get_body().length());
}

void Response::create_response(void) {
	std::string tmp;
	std::vector<std::string>::iterator it;

	for (it = _loc->methods.begin(); it != _loc->methods.end(); ++it)
		_headers["Allow"] += (*it) + " ";
	if (_loc)
		_headers["Location"] = _loc->uri;
	_headers["Retry-After"] = "1";
	_headers["Server"] = _server_name;
	_headers["Date"] = get_server_date();
	_headers["Content-Type"] = get_content_type();
	if (_requested_file.length()) {
		tmp = get_last_modified_date(_loc->root+_subfolder+_requested_file);
		if (tmp.length())
			_headers["Last-Modified"] = tmp;
	}
	if (_method == "HEAD") {
		get_status_line();
		fill_response_body();
	}
	else if (_method == "GET") {
		if (!_loc->auth.empty()) {
			if ((!auth_by_header()) && (!auth_by_uri_param()))
				_status_code = 401;
		}
		get_status_line();
		fill_response_body();
		_response += get_page_body();
	}
	else if (_method == "PUT")
	{
		put_method();
		get_status_line();
		fill_response_body();
	}
	else if (_method == "POST")
	{
		_request->parse_script_file_name();
		_request->set_cgi_meta_vars();
		if ((_request->get_script_path().length()) > 0 && _request->get_body().size() > 0)
		{
			_request->run_cgi_request();
			try{
				_request->read_cgi();
				if (_content_len)
					_headers["Content-Length"] = std::to_string(_content_len);
			}
			catch(const std::exception& e){
				_status_code = 500;
				error_message("Failed to open file for cgi.\n");
			}
			get_status_line();
			fill_response_body();
			_response += _request->get_body();
		}
		else {
			if (_requested_file.length())
				put_method();
			if (_status_code != 401 && _loc->auto_index)
				create_autoindex();
			get_status_line();
			fill_response_body();
			_response += _request->get_body();
		}
	}
	else {
		if (_request->get_status_code() == 0)
			_status_code = 400;
		get_status_line();	
		fill_response_body();
		_response += get_page_body();
		_response += CRLF;
	}
}

void Response::set_status()
{
	_status[200] = "OK";
	_status[204] = "No Content";
	_status[400] =  "Bad Request";
	_status[401] =  "Unauthorized";
	_status[403] =  "Forbidden";
	_status[404] =  "Not Found";
	_status[405] =  "Method Not Allowed";
	_status[408] =  "Request Timeout";
	_status[413] =  "Payload Too Large";
	_status[500] =  "Internal Server Error";
	_status[501] =  "Not Implemented";
}
