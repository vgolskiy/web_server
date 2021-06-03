/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   64coder.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mskinner <v.golskiy@ya.ru>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/06/03 20:54:22 by mskinner          #+#    #+#             */
/*   Updated: 2021/06/03 21:15:02 by mskinner         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <iostream>
#include <vector>

#define BASE64 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

std::string	base64_encode(const std::string &s) {
	std::string::const_iterator	it;
	std::string					res;
	int	val = 0;
	int	valb = -6;

	for (it = s.begin(); it != s.end(); ++it) {
		val = (val << 8) + (unsigned char)(*it);
		valb += 8;
		while (valb >= 0) {
			res.push_back(BASE64[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6)
		res.push_back(BASE64[((val << 8) >> (valb+8)) & 0x3F]);
	while (res.size() % 4)
		res.push_back('=');
    return (res);
}

std::string	base64_decode(const std::string &s) {
	std::string 				base = BASE64;
	std::string::const_iterator	it;
    std::string					res;
    std::vector<int>			code(256, -1);
	int	val = 0;
	int	valb = -8;

	//decoding mask for base64
    for (int i = 0; i < 64; ++i)
		code[BASE64[i]] = i;
    for (it = s.begin(); it != s.end(); ++it) {
		if ((base.find(*it) != std::string::npos) || (*it == '=')) {
        	if (code[*it] == -1)
				break;
        	val = (val << 6) + code[*it];
        	valb += 6;
        	if (valb >= 0) {
            	res.push_back(char((val >> valb) & 0xFF));
            	valb -= 8;
			}
        }
		else
			throw (std::string("Input is not a valid base64-encoded data"));
    }
    return (res);
}

int main() {
	std::string s = "pass";
	s = base64_encode(s);
	std::cout << s << std::endl;
	s = "game";
	s = base64_encode(s);
	s = "user:pass";
	s = base64_encode(s);
	std::cout << s << std::endl;
	std::cout << base64_decode(s) << std::endl;
	try {
		std::cout << base64_decode("GETTY&") << std::endl;
	}
	catch (std::string err) {
		std::cout << err << std::endl;
	}
	return (EXIT_SUCCESS);
}
