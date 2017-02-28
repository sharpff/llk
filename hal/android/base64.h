/*
 * base64.h
 *
 *  Created on: Oct 23, 2015
 *      Author: fei
 */

#ifndef BASE64_H_
#define BASE64_H_

#include <string>

std::string base64_encode_cpp(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string base64_decode_cpp(std::string const& encoded_string);

#endif /* BASE64_H_ */
