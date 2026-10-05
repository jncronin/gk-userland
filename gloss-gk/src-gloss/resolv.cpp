/* Basic userspace library for DNS lookups.  Each process has a separate instance of this
    library as part of libc.
    
    Calls to GK_resolv() are thread-safe.
    
    Each call first checks if it already knows the address, with an appropriate TTL.  If
    not, it binds a new socket (first udp, then tcp if neccessary) for communicating with
    each DNS server in order.  It waits up till 'timeout' for a response prior to moving
    to the next DNS server.
    
    During that time, any messages received are added to the cache and then checked to
    see if they contain the host we are looking for.  If they are a CNAME record then
    we recurse looking up the CNAME target - this should have been recently cached by
    the previous lookup and therefore succeed straightaway.
*/

#include <pthread.h>
#include <string>
#include <cstdint>
#include <ctime>
#include <unordered_map>
#include <map>
#include <memory>
#include <bit>
#include "gk_resolv.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <atomic>
#include <ranges>
#include <vector>
#include <string_view>
#include <cstring>
#include <sstream>
#include <numeric>
#include <new>

/* Some defines */
#define QTYPE_A         1
#define QTYPE_NS        2
#define QTYPE_CNAME     5
#define QTYPE_SOA       6
#define QTYPE_WKS       11
#define QTYPE_PTR       12
#define QTYPE_HINFO     13
#define QTYPE_MINFO     14
#define QTYPE_MX        15
#define QTYPE_TXT       16
#define QTYPE_AAAA      28
#define QTYPE_ALL       255

#define QCLASS_IN       1

/* Key type used in cache (ignores qclass - always IN which is 0x1) */
class DNSHost
{
    public:
        std::string host;
        int qtype;

        bool operator==(const DNSHost &other) const
        {
            return (qtype == other.qtype) && (host == other.host);
        }
};

/* Its hash function */
namespace std
{
    template<> struct hash<DNSHost>
    {
        std::size_t operator()(const DNSHost &h) const noexcept
        {
            return std::rotl(std::hash<std::string>{}(h.host),5) ^
                std::hash<int>{}(h.qtype);

        }
    };
}

/* Result type used in cache */
class DNSEntry
{
    public:
};

class CNameEntry : public DNSEntry
{
    public:
        std::string cname_target;
};

class AEntry : public DNSEntry
{
    public:
        uint32_t naddr;     // in network byte order
};

class AAAAEntry : public DNSEntry
{
    public:
        uint8_t naddr[16];     // in network byte order
};

class RRSet
{
    public:
        timespec tout { std::numeric_limits<time_t>::max(), std::numeric_limits<long>::max() };
        std::vector<std::unique_ptr<DNSEntry>> entries;
        int qtype;

        RRSet clone() const;
};

class Response
{
    public:
        std::unordered_map<DNSHost, RRSet> rrsets;
        uint16_t trid;
        bool needs_tcp = false;
        bool valid = false;
};

/* DNS Cache */
static pthread_mutex_t m_cache = PTHREAD_MUTEX_INITIALIZER;
static std::unordered_map<DNSHost, RRSet> cache;

/* Next transaction id */
std::atomic_uint16_t next_trid = 0;

/* helper functions */
static const RRSet check_cache(const DNSHost &h);
static ResolvResponse rrset_to_resp(const RRSet &r, int af);
static bool operator>(const timespec &a, const timespec &b);
static bool operator<(const timespec &a, const timespec &b);
static bool operator<=(const timespec &a, const timespec &b);
static std::unique_ptr<uint8_t[]> build_query(const std::string &host, uint16_t qtype,
    size_t *ret_sz, uint16_t *trid_out);
static Response parse_response(uint8_t *resp, size_t resplen);
static int parse_one_question(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr);
static int parse_one_response(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr);
static int parse_string_parts(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr, std::vector<std::string> *str_parts);
static int parse_string(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr, std::string *v);
template <typename T> static int parse_int(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr, T *v);

/* The main function */
ResolvResponse GK_Resolv(const char *name, int af)
{
    ResolvResponse ret;
    ret.ret = EAI_ADDRFAMILY;

    /* First, look up our target in the cache */
    if(af == AF_UNSPEC || af == AF_INET)
    {
        DNSHost ccheck;
        ccheck.host = name;
        ccheck.qtype = QTYPE_A;
        auto cret = check_cache(ccheck);
        if(!cret.entries.empty())
        {
            auto aret = rrset_to_resp(cret, af);
            if(aret.ret == 0)
            {
                ret.ret = 0;
                ret.addrs.insert(ret.addrs.end(),
                    std::make_move_iterator(aret.addrs.begin()),
                    std::make_move_iterator(aret.addrs.end()));
            }
        }
    }
    if(af == AF_UNSPEC || af == AF_INET6)
    {
        DNSHost ccheck;
        ccheck.host = name;
        ccheck.qtype = QTYPE_AAAA;
        auto cret = check_cache(ccheck);
        if(!cret.entries.empty())
        {
            auto aret = rrset_to_resp(cret, af);
            if(aret.ret == 0)
            {
                ret.ret = 0;
                ret.addrs.insert(ret.addrs.end(),
                    std::make_move_iterator(aret.addrs.begin()),
                    std::make_move_iterator(aret.addrs.end()));
            }
        }
    }
    // also handle cached cnames
    {
        DNSHost ccheck;
        ccheck.host = name;
        ccheck.qtype = QTYPE_CNAME;
        auto cret = check_cache(ccheck);
        if(!cret.entries.empty())
        {
            auto aret = rrset_to_resp(cret, af);
            if(aret.ret == 0)
            {
                ret.ret = 0;
                ret.addrs.insert(ret.addrs.end(),
                    std::make_move_iterator(aret.addrs.begin()),
                    std::make_move_iterator(aret.addrs.end()));
            }
        }
    }
    if(ret.ret == 0)
    {
        fprintf(stderr, "FOUND: %s in cache\n", name);
        return ret;
    }

    /* The host is not in the cache, begin a dns query */
    struct query
    {
        std::unique_ptr<uint8_t []> buf;
        size_t qsz;
        uint16_t trid;
        int af;
    };
    std::vector<query> queries;
    if(af == AF_UNSPEC || af == AF_INET)
    {
        query q;
        q.buf = std::move(build_query(name, QTYPE_A, &q.qsz, &q.trid));
        queries.push_back(std::move(q));
    }
    if(af == AF_UNSPEC || af == AF_INET6)
    {
        query q;
        q.buf = std::move(build_query(name, QTYPE_AAAA, &q.qsz, &q.trid));
        queries.push_back(std::move(q));
    }
    
    std::vector<uint32_t> dns_servers;
    dns_servers.push_back(0x08080808);      // test server, for now

    for(auto server : dns_servers)
    {
        sockaddr_in dest;
        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = server;
        dest.sin_port = htons(53);

        for(const auto &q : queries)
        {
            bool needs_tcp = false;
            // first, try with udp
            int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);        // AF_INET here is dns connection, not what we look up
            if(udp_socket >= 0)
            {
                auto sendret = sendto(udp_socket, q.buf.get(), q.qsz, 0, (const sockaddr *)&dest,
                    sizeof(dest));
                if(sendret >= 0 && (size_t)sendret == q.qsz)
                {
                    // all data sent, wait for a response
                    timespec now;
                    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
                    timespec tout = now;
                    tout.tv_sec += 5;

                    while(true)
                    {
                        // TODO: have this timeout
                        uint8_t recvbuf[512];
                        auto recvret = recv(udp_socket, recvbuf, sizeof(recvbuf), 0);
                        if(recvret > 0)
                        {
                            // parse return
                            auto dnsret = parse_response(recvbuf, (size_t)recvret);
                            if(dnsret.valid && dnsret.trid == q.trid)
                            {
                                needs_tcp = dnsret.needs_tcp;
                                break;
                            }
                        }

                        clock_gettime(CLOCK_MONOTONIC_RAW, &now);
                        if(now > tout)
                            break;
                    }
                }
            }
            close(udp_socket);

            if(needs_tcp)
            {
                fprintf(stderr, "GK_Resolv: tcp lookup not implemented\n");
            }
        }

        // Now check again if we have the appropriate answer(s)
        if(af == AF_UNSPEC || af == AF_INET)
        {
            DNSHost ccheck;
            ccheck.host = name;
            ccheck.qtype = QTYPE_A;
            auto cret = check_cache(ccheck);
            if(!cret.entries.empty())
            {
                auto aret = rrset_to_resp(cret, af);
                if(aret.ret == 0)
                {
                    ret.ret = 0;
                    ret.addrs.insert(ret.addrs.end(),
                        std::make_move_iterator(aret.addrs.begin()),
                        std::make_move_iterator(aret.addrs.end()));
                }
            }
        }
        if(af == AF_UNSPEC || af == AF_INET6)
        {
            DNSHost ccheck;
            ccheck.host = name;
            ccheck.qtype = QTYPE_AAAA;
            auto cret = check_cache(ccheck);
            if(!cret.entries.empty())
            {
                auto aret = rrset_to_resp(cret, af);
                if(aret.ret == 0)
                {
                    ret.ret = 0;
                    ret.addrs.insert(ret.addrs.end(),
                        std::make_move_iterator(aret.addrs.begin()),
                        std::make_move_iterator(aret.addrs.end()));
                }
            }
        }
        // also handle cached cnames
        {
            DNSHost ccheck;
            ccheck.host = name;
            ccheck.qtype = QTYPE_CNAME;
            auto cret = check_cache(ccheck);
            if(!cret.entries.empty())
            {
                auto aret = rrset_to_resp(cret, af);
                if(aret.ret == 0)
                {
                    ret.ret = 0;
                    ret.addrs.insert(ret.addrs.end(),
                        std::make_move_iterator(aret.addrs.begin()),
                        std::make_move_iterator(aret.addrs.end()));
                }
            }
        }

        if(ret.ret == 0)
        {
            fprintf(stderr, "FOUND: %s from lookup\n", name);
            // we do - return them
            return ret;
        }
    }

    return ret;
}

/* Checks the cache for a potentially valid entry and returns it if valid */
const RRSet check_cache(const DNSHost &h)
{
    pthread_mutex_lock(&m_cache);
    auto iter = cache.find(h);
    if(iter == cache.end())
    {
        pthread_mutex_unlock(&m_cache);

        return RRSet();
    }

    /* We have some valid entries.  Check them for timeout.  All members of a
        rrset should have the same TTL, therefore just check the first. */
    timespec now;
    auto &entries = iter->second;
    if(!clock_gettime(CLOCK_MONOTONIC_RAW, &now))
    {
        if(now > entries.tout)
        {
            cache.erase(iter);
            pthread_mutex_unlock(&m_cache);
            return RRSet();
        }
    }

    /* The entries pass the timeout test, now copy them and return */
    RRSet ret = entries.clone();
    ret.qtype = iter->first.qtype;
    pthread_mutex_unlock(&m_cache);

    return std::move(ret);
}

static bool operator>(const timespec &a, const timespec &b)
{
    if(a.tv_sec > b.tv_sec)
        return true;
    else if(a.tv_sec < b.tv_sec)
        return false;
    else
        return a.tv_nsec > b.tv_nsec;
}

static bool operator<(const timespec &a, const timespec &b)
{
    if(a.tv_sec < b.tv_sec)
        return true;
    else if(a.tv_sec > b.tv_sec)
        return false;
    else
        return a.tv_nsec < b.tv_nsec;
}

static bool operator<=(const timespec &a, const timespec &b)
{
    return b > a;
}

/* Takes a RRSet response from the cache and converts it to a ResolvResponse.
    If the RRSet is of type cname, it calls GK_Resolv again to */
static ResolvResponse rrset_to_resp(const RRSet &r, int af)
{
    if(r.entries.empty())
    {
        // shouldn't get here
        ResolvResponse ret;
        ret.ret = EAI_ADDRFAMILY;
        return ret;
    }

    /* Are we a cname response? */
    if(r.qtype == QTYPE_CNAME)
    {
        return GK_Resolv((reinterpret_cast<CNameEntry *>(r.entries[0].get()))->cname_target.c_str(),
            af);
    }

    /* Otherwise we are the appropriate type.  Convert all entries to ResolvResponses */
    ResolvResponse ret;
    for(const auto &e : r.entries)
    {
        switch(r.qtype)
        {
            case QTYPE_A:
                {
                    const auto cure = reinterpret_cast<AEntry *>(e.get());
                    auto retaddr = std::make_unique<IP4Addr>();
                    retaddr->ver = 4;
                    retaddr->n_addr = cure->naddr;
                    ret.addrs.push_back(std::move(retaddr));
                }
                break;
            case QTYPE_AAAA:
                {
                    const auto cure = reinterpret_cast<AAAAEntry *>(e.get());
                    auto retaddr = std::make_unique<IP6Addr>();
                    retaddr->ver = 6;
                    memcpy(&retaddr->n_addr, &cure->naddr, sizeof(retaddr->n_addr));
                    ret.addrs.push_back(std::move(retaddr));
                }
                break;
        }
    }
    ret.ret = 0;
    return ret;
}

/* Perform deep copy of an RRset including the unique_ptr elements*/
RRSet RRSet::clone() const
{
    RRSet ret;
    ret.qtype = qtype;
    ret.tout = tout;
    for(const auto &e : entries)
    {
        switch(qtype)
        {
            case QTYPE_CNAME:
                {
                    const auto cure = reinterpret_cast<CNameEntry *>(e.get());
                    auto newe = std::make_unique<CNameEntry>();
                    newe->cname_target = cure->cname_target;
                    ret.entries.push_back(std::move(newe));
                }
                break;
            case QTYPE_A:
                {
                    const auto cure = reinterpret_cast<AEntry *>(e.get());
                    auto newe = std::make_unique<AEntry>();
                    newe->naddr = cure->naddr;
                    ret.entries.push_back(std::move(newe));
                }
                break;
            case QTYPE_AAAA:
                {
                    const auto cure = reinterpret_cast<AAAAEntry *>(e.get());
                    auto newe = std::make_unique<AAAAEntry>();
                    memcpy(&newe->naddr, &cure->naddr, sizeof(newe->naddr));
                    ret.entries.push_back(std::move(newe));
                }
                break;
        }
    }

    return ret;
}

static std::unique_ptr<uint8_t[]> build_query(const std::string &host, uint16_t qtype,
    size_t *ret_sz, uint16_t *trid_out)
{
	// split target into parts (www, google, com etc) as per DNS message requirements
	/*
		The format is
			QNAME =
				<PART>+				individual parts
				0x00				terminating null

			PART =
				<int8>				length
				<char>+				actual bytes
	*/
	auto split = host
		| std::views::split(std::string_view{ "." })
		| std::views::transform([](auto&& str) { return std::string_view(&*str.begin(), std::ranges::distance(str)); });
	std::vector<std::string_view> splitv(split.begin(), split.end());

	auto target_len = host.length() - splitv.size() + 1 +		// length of actual characters without separating '.' characters
		splitv.size() +											// length of 'length' characters
		1;														// end NULL


	// calculate required size
	/* The whole request is:
			QUESTION =
				<QNAME>				host name to look up
				<int16>				QTYPE, e.g. 1 for A, 5 for CNAME, 12 for PTR, 15 for MX, 16 for TXT
				<int16>				QCLASS, e.g. 1 for the internet (IN)	
	*/
	const size_t header_size = 12;
	auto req_size = header_size + target_len + 4;

	auto buf = std::unique_ptr<uint8_t[]>(new (std::nothrow) uint8_t[req_size]);
	if (!buf)
		return nullptr;

	/* Build header */
	auto trid = next_trid++;
	uint16_t* hdr = reinterpret_cast<uint16_t*>(const_cast<uint8_t *>(buf.get()));
	hdr[0] = htons(trid);
	hdr[1] = htons(0x0100);		// standard query, allow recursion
	hdr[2] = htons(0x1);		// 1 question
	hdr[3] = htons(0);			// 0 answers
	hdr[4] = htons(0);			// 0 auth rrs
	hdr[5] = htons(0);			// 0 additional rrs

	/* Add question */
	uint8_t* qptr = reinterpret_cast<uint8_t*>(&hdr[6]);
	for (auto part : splitv)
	{
		if (part.length() > 63)
		{
			// part too large to be a valid dns name
			buf = nullptr;
			return nullptr;
		}
		*qptr++ = (uint8_t)part.length();
		memcpy(qptr, part.data(), part.length());
		qptr += part.length();
	}
	*qptr++ = 0;

	/* Everything after here may be misaligned if QNAME has odd length, therefore use memcpy to assign it */
	qtype = htons(qtype);		
	memcpy(qptr, &qtype, sizeof(qtype));
	qptr += 2;

	uint16_t qclass = htons(0x1);		// class = A
	memcpy(qptr, &qclass, sizeof(qtype));
	qptr += 2;

    if(trid_out)
        *trid_out = trid;
    if(ret_sz)
        *ret_sz = qptr - reinterpret_cast<uint8_t*>(&hdr[0]);
    return buf;
}

static Response parse_response(uint8_t *resp, size_t resplen)
{
    Response ret;
    ret.valid = false;

    if(!resp || resplen < 16)
        return ret;

    auto hdr = reinterpret_cast<uint16_t *>(resp);
    ret.trid = ntohs(hdr[0]);
    auto flags = ntohs(hdr[1]);
    if(!(flags & 0x8000u))
    {
        // not response
        return ret;
    }
    ret.valid = true;
    if(flags & 0x0200u)
    {
        // truncated
        ret.needs_tcp = true;
        return ret;
    }
    if((flags & 0xfu) != 0)
    {
        // error
        return ret;
    }
    auto nquestions = ntohs(hdr[2]);
    auto nresps = ntohs(hdr[3]);
    auto qptr = reinterpret_cast<const uint8_t *>(&hdr[6]);
    const auto eptr = &resp[resplen];
    for(auto i = 0u; i < nquestions; i++)
    {
        if(parse_one_question(ret, resp, &qptr, eptr) != 0)
        {
            // fail, but return what we have
            return ret;
        }
    }
    for(auto i = 0u; i < nresps; i++)
    {
        if(parse_one_response(ret, resp, &qptr, eptr) != 0)
        {
            // fail, but return what we have
            return ret;
        }
    }

    // add responses to cache
    pthread_mutex_lock(&m_cache);
    for(auto &[key, rrset] : ret.rrsets)
    {
        cache[key] = std::move(rrset);
    }
    pthread_mutex_unlock(&m_cache);

    return ret;
}

static int parse_one_question(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr)
{
    std::string qname;
    auto qname_ret = parse_string(r, resp, qptr, eptr, &qname);
    if(qname_ret != 0)
        return qname_ret;
    auto qtype_ret = parse_int<int16_t>(r, resp, qptr, eptr, nullptr);
    if(qtype_ret != 0)
        return qtype_ret;
    auto qclass_ret = parse_int<int16_t>(r, resp, qptr, eptr, nullptr);
    if(qclass_ret != 0)
        return qclass_ret;
    fprintf(stderr, "QUESTION: QNAME: %s\n", qname.c_str());
    return 0;
}

static int parse_one_response(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr)
{
    std::string rname;
    auto rname_ret = parse_string(r, resp, qptr, eptr, &rname);
    if(rname_ret != 0)
        return rname_ret;
    uint16_t rtype;
    auto rtype_ret = parse_int(r, resp, qptr, eptr, &rtype);
    if(rtype_ret != 0)
        return rtype_ret;
    uint16_t rclass;
    auto rclass_ret = parse_int(r, resp, qptr, eptr, &rclass);
    if(rclass_ret != 0)
        return rclass_ret;
    uint32_t rttl;
    auto rttl_ret = parse_int(r, resp, qptr, eptr, &rttl);
    if(rttl_ret != 0)
        return rttl_ret;
    uint16_t rdlength;
    auto rdlength_ret = parse_int(r, resp, qptr, eptr, &rdlength);
    if(rdlength_ret != 0)
        return rdlength_ret;
    if((*qptr + rdlength) > eptr)
    {
        return -1;
    }
    
    fprintf(stderr, "RESPONSE: RNAME: %s, RTYPE: %u, TTL: %u, RDLENGTH: %u\n",
        rname.c_str(), rtype, rttl, rdlength);

    // update ttl
    DNSHost ckey;
    ckey.host = rname;
    ckey.qtype = rtype;

    timespec new_tout;
    clock_gettime(CLOCK_MONOTONIC_RAW, &new_tout);
    new_tout.tv_sec += rttl;
    if(new_tout < r.rrsets[ckey].tout)
    {
        r.rrsets[ckey].tout = new_tout;
    }
    r.rrsets[ckey].qtype = rtype;

    // get the appropriate rrset in the response    
    switch(rtype)
    {
        case QTYPE_A:
            {
                auto aret = std::make_unique<AEntry>();
                memcpy(&aret->naddr, *qptr, sizeof(aret->naddr));
                r.rrsets[ckey].entries.push_back(std::move(aret));
            }
            break;

        case QTYPE_AAAA:
            {
                auto aret = std::make_unique<AAAAEntry>();
                memcpy(&aret->naddr, *qptr, sizeof(aret->naddr));
                r.rrsets[ckey].entries.push_back(std::move(aret));
            }
            break;

        case QTYPE_CNAME:
            {
                auto aret = std::make_unique<CNameEntry>();
                auto cret = parse_string(r, resp, qptr, eptr, &aret->cname_target);
                *qptr = *qptr - rdlength;   // parse_string already increments qptr
                r.rrsets[ckey].entries.push_back(std::move(aret));
            }
            break;
    }

    *qptr = *qptr + rdlength;
    return 0;
}


template <typename T> static int parse_int(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr, T *v)
{
    if((*qptr + sizeof(T)) > eptr)
    {
        // too big
        return -1;
    }
    if(v)
    {
        std::make_unsigned_t<T> uval;
        memcpy(&uval, *qptr, sizeof(uval));
        switch(sizeof(T))
        {
            case 1:
                break;
            case 2:
                uval = ntohs(uval);
                break;
            case 4:
                uval = ntohl(uval);
                break;
            default:
                return -1;
        }
        *v = (T)uval;
    }
    *qptr = *qptr + sizeof(T);
    return 0;
}

static int parse_string(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr, std::string *v)
{
    if(!v)
        return parse_string_parts(r, resp, qptr, eptr, nullptr);
    else
    {
        std::vector<std::string> str_parts;
        auto ret = parse_string_parts(r, resp, qptr, eptr, &str_parts);
        if(ret != 0)
            return ret;
        *v = std::accumulate(std::begin(str_parts), std::end(str_parts), std::string(),
            [](const std::string &curs, const std::string &news)
            {
                if(curs.empty())
                    return news;
                else
                    return curs + "." + news;
            });
        return 0;
    }
}

static int parse_string_parts(Response &r, const uint8_t * resp,
    const uint8_t **qptr, const uint8_t *eptr, std::vector<std::string> *str_parts)
{
    while(true)
    {
        // get a part of string
        if(*qptr >= eptr)
        {
            // overrun - fail
            return -1;
        }

        auto schar = **qptr;
        if((schar & 0xc0u) == 0xc0u)
        {
            // this is a compressed string - follow the pointer
            if((*qptr + 1) >= eptr)
            {
                // fail
            }

            // extract using memcpy because we may be misaligned
            uint16_t target;
            memcpy(&target, *qptr, sizeof(target));
            target = ntohs(target);
            target &= 0x3fffu;

            // advance the current read pointer, then follow the target link
            *qptr = *qptr + 2;
            const uint8_t *new_qptr = resp + target;
            return parse_string_parts(r, resp, &new_qptr, eptr, str_parts);
        }
        else if(schar & 0xc0u)
        {
            // this is an error - no other bits should be set
            return -1;
        }
        else if(schar == 0)
        {
            // end of string
            *qptr = *qptr + 1;
            return 0;
        }
        else
        {
            // just a regular string - schar is its length
            *qptr = *qptr + 1;
            if((*qptr + schar) > eptr)
            {
                // buffer too small for string
                return -1;
            }
            std::string cstr((const char *)*qptr, (size_t)schar);
            *qptr = *qptr + schar;
            if(str_parts)
                str_parts->push_back(cstr);
        }
    }
}

std::string IPAddr::tostring() const
{
    return "IPADDR";
}

std::string IP4Addr::tostring() const
{
    return std::string(inet_ntoa((in_addr)n_addr));
}
