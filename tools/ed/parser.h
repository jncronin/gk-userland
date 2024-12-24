#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <tuple>
#include "commands.h"
#include "ed.h"

struct parser_buffer_desc
{
    unsigned int nlines;
    unsigned int cline;
    int markline;

    parser_buffer_desc() {}
    parser_buffer_desc(const ed_buffer_state &s) { nlines = s.buf.size(); cline = s.addr; markline = s.markline; }
    parser_buffer_desc(unsigned int _nlines, unsigned int _cline, int _markline) { nlines = _nlines; cline = _cline; markline = _markline; }
};

struct parse_line_result
{
    bool valid;
    parser_buffer_desc bufdesc;
};

struct parse_second_line_result
{
    bool valid;
    char sep;
    parse_line_result line;
};

struct parse_command_result
{
    bool valid;
    ed_command cmd;
    std::string file;
    std::vector<std::tuple<parse_line_result, parse_second_line_result, parse_command_result>> command_list;
    unsigned int n;
    char x;
    char print_suffix = 0;
};

using ed_command_line = std::tuple<parse_line_result, parse_second_line_result, parse_command_result>;

parse_line_result parse_line_address(const std::string &s, unsigned int *p, const parser_buffer_desc &origbufdesc);
parse_second_line_result parse_second_line_address(const std::string &s, unsigned int *p,
    const parser_buffer_desc &origbufdesc,
    const parser_buffer_desc &modbufdesc);
parse_command_result parse_command(const std::string &s, unsigned int *p,
    const parser_buffer_desc &origbufdesc);
std::string parse_parameters(const std::string &s, unsigned int *p);

#endif
