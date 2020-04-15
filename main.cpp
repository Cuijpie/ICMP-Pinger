#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <getopt.h>

#include "ping.cpp"

auto get_addrinfo(char *hostname) -> addrinfo* {
    struct addrinfo hints, *res;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_flags = AI_PASSIVE; 
    hints.ai_protocol = 0;

    if (int result = getaddrinfo(hostname, NULL, &hints, &res)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        exit(1);
    }
   
    return res;
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

void interrupt_handler(int x) {
    run = 0;
}

auto main(int argc, char **argv) -> int {
    signal(SIGINT, interrupt_handler);

    if (argc < 2) {
        fprintf(stderr, "Invalid number of arguments, please provide a hostname.\n");
        return 0;
    }

    addrinfo *res = get_addrinfo(argv[argc-1]);
    int sockfd = open_socket(res->ai_family);

    Ping *p = new Ping(sockfd, res);

    p->set_config(argc, argv);

    p->start();

    p->print_statistics();
    
    return 0;
}
