#ifndef PARSER_H
#define PARSER_H

#include <string>

std::string parse_line_address(const std::string &s, unsigned int *p);
std::string parse_second_line_address(const std::string &s, unsigned int *p);
std::string parse_command(const std::string &s, unsigned int *p);
std::string parse_parameters(const std::string &s, unsigned int *p);

#endif
