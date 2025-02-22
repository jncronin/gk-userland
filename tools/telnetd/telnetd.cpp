#include "unistd.h"
#include "shell.h"
#include "args.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>

#ifdef IN_TELNETD
int telnetd_main(int argc, const char *argv[], shell_state *sst);

int main(int argc, const char *argv[])
{
    return telnetd_main(argc, argv, nullptr);
}
#endif

int telnetd_main(int argc, const char *argv[], shell_state *sst)
{
    Args a;
    a.AddHelp();
    a.AddArg("--inet-addr", 1, "-i", "", "IP address to bind to (default is all)");
    a.AddArg("--port", 1, "-p", "23", "port to bind to (default 23)");
    a.AddArg("", 0, "-d", "", "keep accepting new connections rather than closing after each one");

    auto args = a.ParseArgs(argc, argv);
    if(args.first != 0)
        return args.first;

    std::string ip_addr;
    auto ip_addr_iter = args.second.find("--inet-addr");
    if(ip_addr_iter != args.second.end() && ip_addr_iter->second.size() > 0)
    {
        ip_addr = ip_addr_iter->second[0];
    }

    in_port_t ip_port = htons(23);
    auto ip_port_iter = args.second.find("--port");
    if(ip_port_iter != args.second.end() && ip_port_iter->second.size() > 0)
    {
        ip_port = htons(atoi(ip_port_iter->second[0].c_str()));
    }

    if(args.second.find("--help") != args.second.end())
    {
        a.ShowHelp();
        return 0;
    }

    int lsck = socket(AF_INET, SOCK_STREAM, 0);
    if(lsck < 0)
    {
        printf("telnetd: socket failed\n");
        return -1;
    }

    in_addr src_addr;
    src_addr.s_addr = 0;
    
    if(ip_addr != "")
    {
        inet_aton(ip_addr.c_str(), &src_addr);
    }
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = src_addr.s_addr;
    saddr.sin_port = ip_port;

    int ret = bind(lsck, (sockaddr *)&saddr, sizeof(saddr));
    if(ret < 0)
    {
        printf("telnetd: bind failed %d\n", errno);
        close(lsck);
        return -1;
    }
    printf("telnetd: bound to %s:%u\n", inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));

    ret = listen(lsck, 0);
    if(ret < 0)
    {
        printf("telnetd: listen failed %d\n", errno);
        close(lsck);
        return -1;
    }

    sockaddr_in caddr;
    socklen_t caddrlen = sizeof(caddr);
    ret = accept(lsck, (sockaddr *)&caddr, &caddrlen);
    if(ret < 0)
    {
        printf("telnetd: accept failed %d\n", errno);
        close(lsck);
        return -1;
    }
    else
    {
        printf("telnetd: accept succeeded %d\n", ret);
    }

    auto act_stdin = dup(STDIN_FILENO);
    auto act_stdout = dup(STDOUT_FILENO);
    auto act_stderr = dup(STDERR_FILENO);

    // update stdin/stdout
    dup2(ret, STDIN_FILENO);
    dup2(ret, STDOUT_FILENO);
    dup2(ret, STDERR_FILENO);

    fflush(stdin);
    fflush(stdout);
    fflush(stderr);

    setvbuf(stdout, nullptr, _IOLBF, BUFSIZ);
    setvbuf(stderr, nullptr, _IONBF, BUFSIZ);

    int sh_main(int argc, const char *argv[], shell_state *sst);
    const char sh[] = "sh";
    const char *sh_argv[] = { sh, nullptr };
    sh_main(1, sh_argv, nullptr);

    close(ret);

    dup2(act_stdin, STDIN_FILENO);
    dup2(act_stdout, STDOUT_FILENO);
    dup2(act_stderr, STDERR_FILENO);

    fflush(stdin);
    fflush(stdout);
    fflush(stderr);

    close(act_stdin);
    close(act_stdout);
    close(act_stderr);

    close(lsck);

    if(args.second.find("-d") != args.second.end())
    {
        execvp(argv[0], (char * const *)&argv[1]);
    }

    return 0;
}
