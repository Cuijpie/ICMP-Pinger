#include <iostream>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/ip.h>

#include "ping.cpp"

auto get_addrinfo(char *hostname) -> addrinfo* {
    struct addrinfo hints, *res;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE; 
    hints.ai_protocol = 0;

    if (int result = getaddrinfo(hostname, NULL, &hints, &res)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        exit(1);
    }
   
    return res;
}

auto set_hoplimit(int sockfd, int ttl) -> void {
    if (setsockopt(sockfd, IPPROTO_ICMP, IP_TTL, &ttl, sizeof(ttl)) != 0) { 
        std::cout << "Setting socket options to TTL failed!" << std::endl; 
        return; 
    } else { 
        std::cout << "Socket set to TTL..\n" << std::endl; 
    } 
}

auto open_socket(int protocol) -> int {
    int sockfd = -1;

    if (protocol == AF_INET) {
        sockfd = socket(protocol, SOCK_RAW, IPPROTO_ICMP);
    } else if (protocol == AF_INET6) {
        sockfd = socket(protocol, SOCK_RAW, IPPROTO_ICMPV6);
    }

    if (sockfd < 0) {
        fprintf(stderr, "File descriptor not found.\n");
        return 0;
    }

    return sockfd;
}

auto main(int argc, char *argv[]) -> int {
    if (argc < 2) {
        fprintf(stderr, "Invalid number of arguments, please provide a hostname.\n");
        return 0;
    }

    addrinfo *res = get_addrinfo(argv[1]);
    int sockfd = open_socket(res->ai_family);
    
    //set ttl
    int ttl_val = argv[2] != NULL ? atoi(argv[2]) : 64;

    //hop limit
    set_hoplimit(sockfd, ttl_val);

    //start ping
    Ping *p = new Ping(sockfd, res);

    p->start();

    return 0;
}
