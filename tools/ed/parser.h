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

enum ed_default_address
{
    cur,
    l1,
    last,
    cur1
};

bool interpret_address(const parser_buffer_desc &buf,
    const parse_line_result &a,
    unsigned int *a_out,
    ed_default_address a_def = cur,
    bool a_allow0 = false);

bool interpret_address_pair(const parser_buffer_desc &buf,
    const parse_line_result &a0,
    const parse_second_line_result &a1,
    unsigned int *a0_out,
    unsigned int *a1_out,
    ed_default_address a0_def = cur,
    ed_default_address a1_def = cur,
    bool a0_allow0 = false,
    bool a1_allow0 = false);

#endif
