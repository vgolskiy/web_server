/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/06/03 21:12:59 by mskinner          #+#    #+#             */
/*   Updated: 2021/06/03 21:14:21 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

/*
Структура Response:
1.Строки состояния HTTP ответа, в которой сервер указывает версию HTTP протокола и код состояния.
2.Нуля или нескольких полей HTTP заголовка, разделенных между собой символом CRLF.
3.Пустой строки (в этой строке должен быть только символ CRLF), эта строка обозначает окончание полей заголовка.
4.Необязательное тело HTTP сообщения.

1. Example:
Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

*/

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Config.hpp"
# include "Request.hpp"
# include <dirent.h>

# define HTML_TITLE		"<html>\n<head>\n<title>Listing of directories</title>\n</head>"
# define HTML_HEADER	"<body>\n<h1>Autoindex: </h1>\n"
# define HYPER_REF		"<a href=\""
# define HYPER_END		"</a><br>"
# define HTML_CLOSE		"</body>\n</html>\n"

struct	t_location;
struct	t_server;
class	Client;
class	Request;
class	Response
{
private:
	std::string							_response;
	std::map<std::string, std::string>	_headers;
	Request*							_request;
	std::string							_method;
	std::string							_param;
	std::string							_authorize;
	std::string 						_body;
	int									_status_code;
	std::map<int, std::string>			_status;
	t_location*							_loc;
	std::string							_subfolder;
	std::string							_requested_file;
	std::string							_server_name;
	int									_content_len;
	std::map<std::string, std::vector<std::string> >	_content_types;
	std::list<std::string>				_headers_sequence;
	std::map<std::string, std::string>	_error_page;

public:
	Response(t_server *server, Request *request);
	~Response();

	void set_status();
	std::string get_server_date(void);
	std::string get_last_modified_date(std::string file);
	void		create_response(void);
	void		fill_response_body(void);
	std::string get_response_body(void);
	std::string	read_page_body(void);
	std::string	get_page_body(void);
	std::string	get_content_type(void);
	void		get_status_line(void);
	bool		auth_by_uri_param(void);
	bool		auth_by_header(void);
	void		create_autoindex();
	void		put_method();

	void		cut_length(int size);
};


#endif //WEBSERVER_RESPONSE_HPP
