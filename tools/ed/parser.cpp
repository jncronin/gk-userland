#include "ed.h"
#include "parser.h"

using bu = std::pair<bool, unsigned int>;
using bs = std::pair<bool, std::string>;

static bool is_eol(const std::string &s, unsigned int *ptr)
{
    if(*ptr >= s.length())
        return true;
    else
        return false;
}

static bs expect_string(const std::string &s, unsigned int *ptr, const std::string &needle)
{
    if(s.rfind(needle, *ptr) == *ptr)
    {
        *ptr += needle.length();
        return std::make_pair(true, needle);
    }
    else
    {
        return std::make_pair(false, "");
    }
}

static bu expect_int(const std::string &s, unsigned int *ptr)
{
    if(is_eol(s, ptr))
        return std::make_pair(false, 0);
    
    auto c0 = s[*ptr];

    // can't use strtol directly as it will match whitespace and plus/minus signs which we don't want
    //  therefore check we are a numeric digit first
    if(c0 >= '0' && c0 <= '9')
    {
        char *endptr;
        auto startptr = &s.c_str()[*ptr];
        errno = 0;
        auto ret = strtoul(startptr, &endptr, 10);

        if(errno == ERANGE)
        {
            return std::make_pair(false, 0);
        }
        if(endptr == startptr)
        {
            return std::make_pair(false, 0);
        }
        *ptr += endptr - startptr;
        return std::make_pair(true, ret);
    }
    return std::make_pair(false, 0);
}

parse_line_result parse_line_address(const std::string &s, unsigned int *ptr,
    const parser_buffer_desc &origbuf)
{
    bu intret;
    unsigned int orig_ptr = *ptr;     // for rewinding
    if(expect_string(s, ptr, ".").first)
    {
        // .
        return { .valid = true, .bufdesc = origbuf };
    }
    else if(expect_string(s, ptr, "$").first)
    {
        // last line
        return { .valid = true, .bufdesc = parser_buffer_desc(origbuf.nlines, origbuf.nlines, origbuf.markline) };
    }
    else if((intret = expect_int(s, ptr)).first)
    {
        return { .valid = true, .bufdesc = parser_buffer_desc(origbuf.nlines, intret.second, origbuf.markline) };
    }
    else if(expect_string(s, ptr, "+").first)
    {
        bu intret2;
        if((intret2 = expect_int(s, ptr)).first)
        {
            return { .valid = true, .bufdesc = parser_buffer_desc(origbuf.nlines, origbuf.cline + intret2.second, origbuf.markline) };
        }
        else
        {
            // may be a single or repeated '+'
            unsigned int count = 1;
            while(expect_string(s, ptr, "+").first)
                count++;
            return { .valid = true, .bufdesc = parser_buffer_desc(origbuf.nlines, origbuf.cline + count, origbuf.markline) };
        }
    }
    else if(expect_string(s, ptr, "-").first)
    {
        bu intret2;
        if((intret2 = expect_int(s, ptr)).first)
        {
            return { .valid = true, .bufdesc = parser_buffer_desc(origbuf.nlines, origbuf.cline - intret2.second, origbuf.markline) };
        }
        else
        {
            // may be a single or repeated '-'
            unsigned int count = 1;
            while(expect_string(s, ptr, "-").first)
                count++;
            return { .valid = true, .bufdesc = parser_buffer_desc(origbuf.nlines, origbuf.cline - count, origbuf.markline) };
        }
    }
    else if(expect_string(s, ptr, "/").first)
    {
        // regex - not yet supported
        return { .valid = false };
    }
    else if(expect_string(s, ptr, "?").first)
    {
        // regex - not yet supported
        return { .valid = false };
    }
    else if(expect_string(s, ptr, "\'").first && !is_eol(s, ptr) && s.c_str()[*ptr] >= 'a' && s.c_str()[*ptr] <= 'z')
    {
        // mark
        ++*ptr;
        if(origbuf.markline >= 0)
            return { .valid = true, .bufdesc = parser_buffer_desc(origbuf.nlines, (unsigned int)origbuf.markline, origbuf.markline) };
        else
        {
            *ptr = orig_ptr;
            return { .valid = false };
        }
    }
    *ptr = orig_ptr;
    return { .valid = false };
}

parse_second_line_result parse_second_line_address(const std::string &s, unsigned int *ptr,
    const parser_buffer_desc &origbufdesc,
    const parser_buffer_desc &modbufdesc)
{
    if(expect_string(s, ptr, ",").first)
    {
        return { .valid = true, .sep = ',', .line = parse_line_address(s, ptr, origbufdesc) };
    }
    else if(expect_string(s, ptr, ";").first)
    {
        return { .valid = true, .sep = ';', .line = parse_line_address(s, ptr, modbufdesc) };
    }
    else
    {
        return { .valid = false };
    }
}

static unsigned int print_suffix(const std::string &s, unsigned int *ptr)
{
    unsigned int ret = 0;
    while(!is_eol(s, ptr))
    {
        if(s[*ptr] == 'p')
        {
            ret |= PRINT_SUFFIX_P;
            ++*ptr;
        }
        else if(s[*ptr] == 'l')
        {
            ret |= PRINT_SUFFIX_L;
            ++*ptr;
        }
        else if(s[*ptr] == 'n')
        {
            ret |= PRINT_SUFFIX_N;
            ++*ptr;
        }
        else
            return 0;
    }
    return ret;
}

#define HANDLE_SIMPLECOMMAND(x, c) if(expect_string(s, ptr, #x).first) return { .valid = true, .cmd = c, .print_suffix = print_suffix(s, ptr) };
#define HANDLE_FILECOMMAND(x, c) if(expect_string(s, ptr, #x).first) return handle_file(s, ptr, c, orig_ptr);

static bool ends_with(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}

parse_command_result handle_file(const std::string &s, unsigned int *ptr, ed_command cmd, unsigned int orig_ptr)
{
    // expect either newline (or \) or whitespace followed by a filename
    unsigned int orig_ptr2 = *ptr;
    if(is_eol(s, ptr))
        return { .valid = true, .cmd = cmd, .file = "" };
    else if(expect_string(s, ptr, "\\").first && is_eol(s, ptr))
    {
        // backtrack so we can parse the \ again
        *ptr = orig_ptr2;
        return { .valid = true, .cmd = cmd, .file = "" };
    }
    else if(expect_string(s, ptr, " ").first)
    {
        // count any number of extra whitespace
        while(expect_string(s, ptr, " ").first);

        // read string
        auto fname = s.substr(*ptr);

        auto fname_len = fname.length();
        if(ends_with(fname, "\\"))
        {
            fname_len--;
            fname = fname.substr(0, fname_len);
        }

        if(fname_len > 0)
        {
            *ptr += fname_len;
            return { .valid = true, .cmd = cmd, .file = fname };
        }        
    }

    // backtrack
    *ptr = orig_ptr;
    return { .valid = false };
}

parse_command_result parse_command(const std::string &s, unsigned int *ptr,
    const parser_buffer_desc &origbufdesc)
{
    unsigned int orig_ptr = *ptr;

    HANDLE_SIMPLECOMMAND(a, a_append)
    else HANDLE_SIMPLECOMMAND(c, c_change)
    else HANDLE_SIMPLECOMMAND(d, d_delete)
    else HANDLE_FILECOMMAND(e, e_editfile)
    else HANDLE_FILECOMMAND(E, E_editfileunconditional)
    else HANDLE_FILECOMMAND(f, f_setfilename)
    else HANDLE_SIMPLECOMMAND(h, h_help)
    else HANDLE_SIMPLECOMMAND(H, H_helptoggle)
    else HANDLE_SIMPLECOMMAND(i, i_insert)
    else HANDLE_SIMPLECOMMAND(j, j_join)
    else HANDLE_SIMPLECOMMAND(l, l_list)
    else HANDLE_SIMPLECOMMAND(n, n_number)
    else HANDLE_SIMPLECOMMAND(p, p_print)
    else HANDLE_SIMPLECOMMAND(P, P_prompttoggle)
    else HANDLE_SIMPLECOMMAND(q, q_quit)
    else HANDLE_SIMPLECOMMAND(Q, Q_quitunconditional)
    else HANDLE_SIMPLECOMMAND(u, u_undo)
    else HANDLE_FILECOMMAND(w, w_write)
    else HANDLE_FILECOMMAND(wq, wq_writequit)
    else HANDLE_FILECOMMAND(W, W_writeappend)
    else HANDLE_SIMPLECOMMAND(x, x_paste)
    else HANDLE_SIMPLECOMMAND(y, y_yank)

    return { .valid = false };
}

static unsigned int interpret_addr_default(const parser_buffer_desc &buf, ed_default_address def)
{
    switch(def)
    {
        case ed_default_address::cur:
            return buf.cline;
        case ed_default_address::cur1:
            return buf.cline + 1;
        case ed_default_address::l1:
            return 1;
        case ed_default_address::last:
            return buf.nlines;
    }
    return 0;
}

bool interpret_address_pair(const parser_buffer_desc &buf,
    const parse_line_result &a0,
    const parse_second_line_result &a1,
    unsigned int *a0_out,
    unsigned int *a1_out,
    ed_default_address a0_def,
    ed_default_address a1_def,
    bool a0_allow0,
    bool a1_allow0)
{
    // just a ',' means whole buffer
    if(!a0.valid && a1.valid && !a1.line.valid)
    {
        if(a1.sep == ',')
        {
            *a0_out = 1;
            *a1_out = buf.nlines;
            return true;
        }
        // just a ';' is not supported
        return false;
    }

    // if no address range is specified, then return the defaults
    if(!a0.valid && !a1.valid)
    {
        *a0_out = interpret_addr_default(buf, a0_def);
        *a1_out = interpret_addr_default(buf, a1_def);
        return true;
    }

    // if only the first address is given, the second is set to the first
    if(a0.valid && (!a1.valid || !a1.line.valid))
    {
        if(a0.bufdesc.cline == 0 && (!a0_allow0 || !a1_allow0))
        {
            return false;
        }
        else
        {
            *a0_out = a0.bufdesc.cline;
            *a1_out = a0.bufdesc.cline;
            return true;
        }
    }

    // if only the second address is given the range is (1,addr) or (.$addr)
    if(!a0.valid && a1.valid && a1.line.valid)
    {
        if(a1.line.bufdesc.cline == 0 && !a1_allow0)
        {
            return false;
        }
        else
        {
            *a1_out = a1.line.bufdesc.cline;
            *a0_out = (a1.sep == ',') ? 1 : buf.cline;
            return true;
        }
    }

    // if both are specified use those
    if(a0.valid && a1.valid && a1.line.valid)
    {
        if(a0.bufdesc.cline == 0 && !a0_allow0)
        {
            return false;
        }
        else if(a1.line.bufdesc.cline == 0 && !a1_allow0)
        {
            return false;
        }
        else
        {
            *a0_out = a0.bufdesc.cline;
            *a1_out = a1.line.bufdesc.cline;
            return true;
        }
    }

    // catch error
    return false;
}
