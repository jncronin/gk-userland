#include "unistd.h"
#include "shell.h"
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
    int lsck = socket(AF_INET, SOCK_STREAM, 0);
    if(lsck < 0)
    {
        printf("telnetd: socket failed\n");
        return -1;
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = 0;
    saddr.sin_port = htons(23);

    int ret = bind(lsck, (sockaddr *)&saddr, sizeof(saddr));
    if(ret < 0)
    {
        printf("telnetd: bind failed %d\n", errno);
        return -1;
    }

    ret = listen(lsck, 0);
    if(ret < 0)
    {
        printf("telnetd: listen failed %d\n", errno);
        return -1;
    }
    while(true)
    {
        sockaddr_in caddr;
        socklen_t caddrlen = sizeof(caddr);
        ret = accept(lsck, (sockaddr *)&caddr, &caddrlen);
        if(ret < 0)
        {
            printf("telnetd: accept failed %d\n", errno);
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

        close(act_stdin);
        close(act_stdout);
        close(act_stderr);
    }
}
