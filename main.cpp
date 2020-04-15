#include <iostream>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <fcntl.h> 
#include <signal.h>
#include <stdlib.h>
#include <net/if.h>

#define PACKET_SIZE 64

typedef struct icmp_packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE-sizeof(struct icmphdr)];

} icmp_packet;

addrinfo *get_addrinfo(char *hostname) {
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

void set_hoplimit(int sockfd, int ttl) {
    if (setsockopt(sockfd, IPPROTO_ICMP, IP_TTL, &ttl, sizeof(ttl)) != 0) { 
        printf("Setting socket options to TTL failed!\n"); 
        return; 
    } else { 
        printf("Socket set to TTL..\n"); 
    } 
}

int open_socket(int protocol) {
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

unsigned short checksum(void *b, int len) {    
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

void ping(int sockfd, addrinfo* res) {
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
            printf("Packet Sending Failed!\n"); 
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

int main(int argc, char *argv[]) {
    if (argc <= 2) {
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
