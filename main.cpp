#include <iostream>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>

#define PACKET_SIZE 64

typedef struct icmp_packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE-sizeof(struct icmphdr)];

} icmp_packet;

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

auto checksum(void *b, int len) -> unsigned short {    
    unsigned short *buf = (u_short*)b; 
    unsigned int sum = 0; 
    unsigned short result; 
  
    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++; 
    } 
        
    if (len == 1) {
        sum += *(unsigned char*)buf;
    } 
         
    sum = (sum >> 16) + (sum & 0xFFFF); 
    sum += (sum >> 16); 
    result = ~sum; 

    return result; 
}

auto ping(int sockfd, addrinfo* res) -> void {
    icmp_packet packet;
    struct sockaddr_storage recv_addr;
    socklen_t recv_len = sizeof(struct sockaddr_storage);

    int send, recv;
    int msg_count = 0;
    int seq = 0;
    
    //TODO:Compute metrics
    while(true) {
        bzero(&packet, sizeof(packet));

        packet.hdr.type = ICMP_ECHO; 
        packet.hdr.un.echo.id = getpid(); 
        packet.hdr.un.echo.sequence = seq++; 
        packet.hdr.checksum = checksum(&packet, sizeof(packet));

        //send packet 
        
        usleep(1000000);
        
        if ((send = sendto(sockfd, &packet, sizeof(packet), 0, res->ai_addr, res->ai_addrlen)) < 0) { 
            std::cout << "Failed to send package." << std::endl; 
        } else {
            std::cout << "\nPacket succesfully send!!: " << send << std::endl;
        }

        //recv package
        if ((recv = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&recv_addr, &recv_len)) < 0) {
            std::cout << "Failed to receive package: " << recv << std::endl;
        } else {
            std::cout << "message received: " << recv << std::endl;
        }          
    } 
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
    ping(sockfd, res);

    return 0;
}
