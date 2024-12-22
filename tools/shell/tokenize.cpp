#include <vector>
#include <string>

static std::string deescape(const std::string &s);

std::vector<std::string> tokenize(const char *buf, unsigned int len)
{
    // tokenize, handling quotes
    std::vector<std::string> toks;

    unsigned int i = 0;
    bool in_quote = false;
    bool in_escape = false;
    bool cur_has_escape = false;
    unsigned int cur_start = 0;
    while(i < len)
    {
        if(!in_quote && !in_escape && (buf[i] == ' ' || buf[i] == 0 || buf[i] == '\n'))
        {
            // end of token
            if(i != cur_start)
            {
                // add
                std::string cur(&buf[cur_start], i - cur_start);
                if(cur_has_escape)
                {
                    toks.push_back(deescape(cur));
                    cur_has_escape = false;
                }
                else
                {
                    toks.push_back(cur);
                }
            }
            cur_start = i + 1;
            if(buf[i] == 0)
            {
                break;
            }
        }
        else if(in_escape)
        {
            // ignore
            in_escape = false;
        }
        else if(buf[i] == '\\')
        {
            in_escape = true;
            cur_has_escape = true;
        }
        else if(buf[i] == '\"')
        {
            if(in_quote)
            {
                in_quote = false;
            }
            else
            {
                in_quote = true;
            }
        }
        i++;
    }
    return toks;
}

std::string deescape(const std::string &s)
{
    auto ns = new char[s.length() + 1];
    ns[s.length()] = 0;
    bool is_escape = false;
    unsigned int i = 0;
    for(auto c : s)
    {
        if(!is_escape && c == '\\')
        {
            is_escape = true;
        }
        else
        {
            is_escape = false;
            ns[i++] = c;
        }
    }
    ns[i] = 0;
    std::string ret(ns, i);
    delete[] ns;
    return ret;
}
