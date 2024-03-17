#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

struct thread_params
{
    sockaddr_in peer_addr;
    std::string fname;

    enum mode_t { netascii, octet };
    mode_t mode;

    bool is_write;
};

static void *tfer_thread(void *tp);

static void send_error(int sockfd, const struct sockaddr_in *dest,
    int err_number, std::string errmsg = "");
static void send_ack(int sockfd, const struct sockaddr_in *dest,
    int blockno);

static void handle_write(int sockfd, const sockaddr_in *dest, FILE *f);
static void handle_read(int sockfd, const sockaddr_in *dest, FILE *f);

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
            printf("invalid packet length: %li\n", rf_ret);
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
                        printf("invalid mode: %s\n", mode.c_str());
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

static void send_error(int sockfd, const struct sockaddr_in *dest,
    int err_number, std::string errmsg)
{
    char buf[512];
    *reinterpret_cast<uint16_t *>(&buf[0]) = htons(5);
    *reinterpret_cast<uint16_t *>(&buf[2]) = htons(err_number);
    strncpy(&buf[4], errmsg.c_str(), 511-4);
    buf[511] = 0;

    auto to_send = std::min(512U, (unsigned int)errmsg.length() + 5);

    sendto(sockfd, buf, to_send, 0, (const sockaddr *)dest, sizeof(sockaddr_in));
}

static void send_ack(int sockfd, const struct sockaddr_in *dest,
    int block_no)
{
    char buf[32];
    *reinterpret_cast<uint16_t *>(&buf[0]) = htons(4);
    *reinterpret_cast<uint16_t *>(&buf[2]) = htons(block_no);
    sendto(sockfd, buf, 4, 0, (const sockaddr *)dest, sizeof(sockaddr_in));
}

static void *tfer_thread(void *_tp)
{
    auto tp = reinterpret_cast<thread_params *>(_tp);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        printf("client socket failed %d\n", errno);
        delete tp;
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
                delete tp;
                return nullptr;
            }
        }
        else
        {
            break;
        }
    }

    // open file, mode dependent upon RRQ vs WRQ
    char fmode[8];
    if(tp->is_write)
    {
        if(tp->mode == thread_params::netascii)
        {
            strcpy(fmode, "w");
        }
        else
        {
            strcpy(fmode, "wb");
        }
    }
    else
    {
        if(tp->mode == thread_params::netascii)
        {
            strcpy(fmode, "r");
        }
        else
        {
            strcpy(fmode, "rb");
        }
    }
    auto f = fopen(tp->fname.c_str(), fmode);
    if(!f)
    {
        printf("error opening file %s: %d\n", tp->fname.c_str(), errno);
        send_error(sockfd, &tp->peer_addr, 1);
        close(sockfd);
        delete tp;
        return nullptr;
    }

    if(tp->is_write)
    {
        handle_write(sockfd, &tp->peer_addr, f);
    }
    else
    {
        handle_read(sockfd, &tp->peer_addr, f);
    }

    close(sockfd);
    fclose(f);
    delete tp;
    return nullptr;
}

static void handle_write(int sockfd, const sockaddr_in *dest, FILE *f)
{
    // writes start by acknowledging block 0 (i.e. the WRQ)
    send_ack(sockfd, dest, 0);

    int block_no = 1;
    // then expect to receive packets and acknowledge them appropriately
    while(true)
    {
        char buf[516];

        sockaddr_in from = { 0 };
        socklen_t fromlen = sizeof(sockaddr_in);
        auto br = recvfrom(sockfd, buf, 516, 0, (sockaddr *)&from, &fromlen);

        // ensure it is our peer
        if(fromlen < sizeof(sockaddr_in) || memcmp(dest, &from, sizeof(sockaddr_in)) != 0)
            continue;
        
        // check appropriate size
        if(br < 4)
        {
            printf("packet size %d, %d\n", (int)br, errno);
            return;
        }

        // check packet type
        auto ptype = ntohs(*reinterpret_cast<uint16_t *>(&buf[0]));
        if(ptype != 3)
        {
            printf("invalid ptype %d\n", ptype);
            return;
        }
        
        // ensure it is the next expected packet
        auto cur_block_id = ntohs(*reinterpret_cast<uint16_t *>(&buf[2]));
        if(cur_block_id != block_no)
        {
            printf("out of order packet %d\n", cur_block_id);
            continue;
        }

        if(br > 4)
        {
            // try writing to file
            auto bw = fwrite(&buf[4], 1, br - 4, f);
            if(bw != br - 4)
            {
                printf("fwrite failed %d, %d\n", (unsigned int)bw, errno);
                send_error(sockfd, dest, 0, "fwrite failed");
                return;
            }
            printf("wrote %d bytes, pkt %d\n", (int)bw, cur_block_id);
        }

        // send ack
        send_ack(sockfd, dest, block_no);
        block_no++;

        if(br < 516)
        {
            // done
            return;
        }
    }
}

static void handle_read(int sockfd, const sockaddr_in *dest, FILE *f)
{
    int block_no = 1;

    // reads start by writing data straight away
    while(true)
    {
        char buf[516];

        *reinterpret_cast<uint16_t *>(&buf[0]) = ntohs(3);
        *reinterpret_cast<uint16_t *>(&buf[2]) = ntohs(block_no);

        auto br = fread(&buf[4], 1, 512, f);
        if(br < 0)
        {
            printf("fread failed %d\n", errno);
            send_error(sockfd, dest, 0, "fread failed");
            return;
        }

        auto bw = sendto(sockfd, buf, br + 4, 0, (const sockaddr *)dest, sizeof(sockaddr_in));
        if(bw != br + 4)
        {
            printf("sendto failed %d, %d\n", (int)bw, errno);
            send_error(sockfd, dest, 0, "sendto failed");
            return;
        }

        // await ack
        while(true)
        {
            sockaddr_in from = { 0 };
            socklen_t fromlen = sizeof(sockaddr_in);
            br = recvfrom(sockfd, buf, 516, 0, (sockaddr *)&from, &fromlen);

            printf("recv %u\n", (unsigned int)br);

            // ensure it is our peer
            if(fromlen < sizeof(sockaddr_in) || memcmp(dest, &from, sizeof(sockaddr_in)) != 0)
            {
                printf("recv not from peer\n");
                continue;
            }
            
            // check appropriate size
            if(br < 4)
            {
                printf("packet size %u, %d\n", (unsigned int)br, errno);
                return;
            }

            // check packet type
            auto ptype = ntohs(*reinterpret_cast<uint16_t *>(&buf[0]));
            if(ptype != 4)
            {
                printf("invalid ptype %d\n", ptype);
                return;
            }

            // check block_no
            auto cur_block_id = ntohs(*reinterpret_cast<uint16_t *>(&buf[2]));
            printf("ack for %u\n", cur_block_id);
            if(cur_block_id == block_no)
                break;
        }

        if(bw == 4)
        {
            // final packet
            return;
        }

        block_no++;
    }
}
