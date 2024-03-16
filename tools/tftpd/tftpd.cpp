#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <pthread.h>

struct thread_params
{
    sockaddr_in peer_addr;
    std::string fname;

    enum mode_t { netascii, octet };
    mode_t mode;

    bool is_write;
};

static void *tfer_thread(void *tp);

int main(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        printf("unable to open socket: %d\n", errno);
        return sockfd;
    }

    sockaddr_in sin;
    sin.sin_addr.s_addr = 0;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(69);

    if(bind(sockfd, (const sockaddr *)&sin, sizeof(sockaddr_in)) < 0)
    {
        printf("unable to bind socket: %d\n", errno);
        return -1;
    }

    /* receive on main socket, and spawn off handling threads for all connections */
    while(true)
    {
        char buf[512];
        sockaddr_in src;
        socklen_t srclen = sizeof(sockaddr_in);
        ssize_t rf_ret = recvfrom(sockfd, buf, 512, 0, (sockaddr *)&src, &srclen);
        if(rf_ret < 0)
        {
            printf("recvfrom failed: %d\n", errno);
            return rf_ret;
        }

        if(rf_ret < 2)
        {
            printf("invalid packet length: %d\n", rf_ret);
            continue;
        }

        auto opcode = htons(*reinterpret_cast<uint16_t *>(&buf[0]));
        switch(opcode)
        {
            case 1:
            case 2:
                {
                    // parse packet                    
                    auto idx = 2;

                    size_t slen = strnlen(&buf[idx], rf_ret - idx);
                    if(slen == (rf_ret - idx))
                    {
                        // invalid fname
                        printf("invalid fname\n");
                        break;
                    }
                    std::string fname(&buf[idx]);
                    idx += fname.length() + 1;

                    slen = strnlen(&buf[idx], rf_ret - idx);
                    if(slen == (rf_ret - idx))
                    {
                        // invalid mode
                        printf("invalid mode\n");
                        break;
                    }

                    std::string mode(&buf[idx]);
                    idx += mode.length() + 1;

                    for(auto &c : mode)
                        c = toupper(c);

                    thread_params::mode_t m;
                    if(mode == "NETASCII")
                        m = thread_params::netascii;
                    else if(mode == "OCTET")
                        m = thread_params::octet;
                    else
                    {
                        printf("invalid mode: %s\n", mode);
                        break;
                    }

                    // create new thread
                    auto tp = new thread_params;
                    tp->peer_addr = src;
                    tp->fname = fname;
                    tp->mode = m;
                    tp->is_write = opcode == 2;
                    
                    pthread_attr_t attr;
                    if(pthread_attr_init(&attr) != 0)
                    {
                        printf("pthread_attr_init failed %d\n", errno);
                        delete tp;
                        break;
                    }
                    pthread_t tid;
                    if(pthread_create(&tid, &attr, tfer_thread, tp) != 0)
                    {
                        printf("pthread_create failed %d\n", errno);
                        delete tp;
                    }
                }
                break;
            default:
                printf("invalid opcode %d\n", opcode);
                break;
        }
    }

    return 0;
}

static void *tfer_thread(void *_tp)
{
    auto tp = reinterpret_cast<thread_params *>(_tp);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        printf("client socket failed %d\n", errno);
        return nullptr;
    }

    // open socket at random ephemereal port to communicate with client
    int ntries = 0;
    sockaddr_in sin;
    while(true)
    {
        auto try_port = (rand() % (65535-1024)) + 1024;
        sin.sin_addr.s_addr = 0;
        sin.sin_port = htons(try_port);
        sin.sin_family = AF_INET;
        if(bind(sockfd, (const sockaddr *)&sin, sizeof(sockaddr_in)) != 0)
        {
            ntries++;
            if(ntries > 100)
            {
                // fail
                printf("failed to open ephemereal port\n");
                return nullptr;
            }
        }
        else
        {
            break;
        }
    }

    // now decide what to do based upon whether RRQ or WRQ

    // TODO
}
