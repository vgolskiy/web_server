/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/15 19:29:09 by mskinner          #+#    #+#             */
/*   Updated: 2021/06/03 21:14:21 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

/*

1.Начальная строка сообщения. (Метод)
2.Заголовок, который может содержать одно или несколько полей заголовка.
3.Пустая строка (на самом деле эта строка не пустая, она содержит символ CTRLF), которая обозначает конец заголовка.
4.Необязательное тело сообщения или HTTP объект.

*/

/*
Headers (FOR REQUESTS only):
◦ Accept-Charsets (какие наборы символов приемлемы для ответов сервера): utf-8
◦ Accept-Language (указывает серверу приемлемые языки): ru
◦ Authorization (отправки данных авторизации на сервер): Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
◦ Content-Language: en, ru
◦ Content-Length (bytes): 1367
◦ Content-Location (альтернативное расположение содержимого сущности): 
◦ Content-Type: text/html;charset=utf-8 (не допускают пробелов вокруг символа равенства «=»)
				- Значение параметра, соответствующее производству токена, может быть передано либо в виде токена,
				- либо внутри строки в кавычках. Значения в кавычках и без кавычек эквивалентны.

◦ Date: Tue, 15 Nov 1994 08:13:45 GMT
◦ Host: ru.wikioedia.org
◦ Last-Modified (дата последней модификаци): 
◦ Referer (URI, после которого клиент сделал текущий запрос): http://en.wikipedia.org/wiki/Main_Page
◦ Transfer-Encoding (список способов кодирования): chunked
◦ User-Agent (список названий и версий клиента и его компонентов с комментариями):
*/

/*
Примеры запросов:

GET /hello.htm HTTP/1.1     (этот запрос посылает клиент)
 
Любое поле заголовка в HTTP сообщение имеет единый синтаксис:
имя_заголовка : значение

!!!!!HTTP сообщения не должны одновременно включать и поле заголовка Content-Length и применять кодирование передачи типа «chunked»!!!!!!!
Если поступило HTTP сообщение с полем Content-Length и закодированное с применением кодирования передачи «chunked», то поле Content-Length должно игнорироваться.


Если Request-URI — это absoluteURI, то хост — это часть Request-URI. Любое значение поля заголовка Host в запросе ДОЛЖНО игнорироваться (напомню про требования HTTP).
Если Request-URI — не absoluteURI, а запрос содержит поле заголовка Host, то хост определяется значением поля заголовка Host.
Если хоста, определенного правилами 1 или 2 не существует на сервере, код состояния ответа должен быть 400 (Испорченный Запрос, Bad Request).

*/

/*
Request =
	Request-Line ;
	*(( general-header ;
	request-header ;
	entity-header ) CRLF) ;
	CRLF [ message-body ] ; 
*/

/*
|CGI module|

CGI defines the abstract parameters,
known as metavariables, which
describe the client's request.

GET:
1.Получить значение переменной QUERY_STRING
2.Декодировать имена и их значения (учитывая, что все пробелы при декодировании сервером были 
  заменены символом "+" и все символы с десятичным кодом больше 128 преобразованы в символ "%" и следующим за ним шестнадцатеричным кодом символа.)
3.Сформировать структуру соответствия "имя - значение" для дальнейшего использования в cgi-модуле

POST:
1.Получить из стандартного входного потока CONTENT_LENGTH символов
2.Декодировать имена и их значения (учитывая, что все пробелы при декодировании сервером были 
  заменены символом "+" и все символы с десятичным кодом больше 128 преобразованы в символ "%" 
  и следующим за ним шестнадцатеричным кодом символа.)
3.Сформировать структуру соответствия "имя - значение" для дальнейшего использования в cgi-модуле

*/

#ifndef REQUEST_HPP
#define REQUEST_HPP

# include "Config.hpp"

# define HTTP			"HTTP/1.1"
# define HOST			"Host"
# define AUTHORIZATION	"Authorization"
# define CONTENT_LEN	"Content-Length"
# define TRANSF_ENCODE	"Transfer-Encoding"
# define USER_AGENT		"User-Agent"
# define CHUNKED		"chunked"
# define CRLF			"\r\n"
# define CRLF_2X		"\r\n\r\n"

#define TMP "tmp_file"

struct	t_location;
class Response;
class Client;
class Request
{
private:
	const int							_i;
	std::string							_param;
	std::string							_method;
	bool								_method_allowed;
	std::string							_uri;
	std::string							_version;
	std::map<std::string, std::string>	_headers;
	std::string							_authorize;
	std::string 						_body;
	std::map<std::string, std::string>	_env;
	std::string							_location;
	std::string							_subfolder;
	std::string							_script_name;
	std::string							_script_path;
	std::string							_requested_file;
	int									_max_body_size;
	bool								_curl;
	static std::string const			methods[];
	std::vector<std::string>			_headers_set;

	int									_status_code;

	Client*								_client;
	int									_status;
	int									_remain_len; // bytes left to read
	int									_content_len;
	bool								_chunk;

	std::string 						_response;
public:
	Request(Client *client, const int i);
	~Request(void);

	void		init_headers_set(void);
	void		parse_request(std::string &lines);
	bool		parse_chunk_size(std::string &lines);
	void		parse_chunk_data(std::string &lines);
	bool		check_start_line(const std::vector<std::string> &lines);
	bool		set_up_headers(std::string &lines);
	void		set_cgi_meta_vars(void);
	void		set_request_status(int status);
	void		cut_remain_len(int to_cut);
	std::string*	find_header(std::string header);
	void		verify_body(void);
	int			get_remain_len(void) const;
	int 		get_status(void) const;
	std::string	get_body(void) const;
	std::string	get_script_path(void) const;
	std::string	get_script_name(void) const;
	std::string get_method(void) const;
	std::string get_location_name(void) const;
	std::string	get_requested_file(void) const;
	std::string	get_subfolder(void) const;
	std::string	get_uri_parameters(void) const;
	std::string	get_authorization(void) const;
	std::map<std::string, std::string>	get_headers(void) const;
	std::string get_uri(void) const;
	int			get_status_code(void) const;
	void		set_status_code(int code);
	int			get_content_length(void) const;
	void		print_parsed_request(void);
	void		run_cgi_request(void);
	void		standard_body_parse(std::string &lines, std::size_t &pos);
	void		curl_body_parse(std::string &lines, std::size_t &pos);
	void		parse_script_file_name(void);
	std::vector<std::string>	convert_cgi_meta_vars(void);
	void		read_cgi();
	void		set_uri_parameters(void);
	void		set_uri_file_name(void);
	void		verify_subfolder(t_location *loc);

	enum status
	{
		REQUEST_METHOD,
		HEADERS,
		BODY_PARSE,
		CHUNK,
		CHUNK_DATA,
		CHUNK_DONE,
		C_G_I,
		BAD_REQ,
		DONE
	};
};

#endif
