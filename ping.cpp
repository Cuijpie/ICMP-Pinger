
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define PACKET_SIZE 64

typedef struct icmp_packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE-sizeof(struct icmphdr)];

} icmp_packet;

class Ping {
    private:
        int sockfd;
        addrinfo* res;

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

    public:
        Ping(int sockfd, addrinfo* res) {
            this->sockfd = sockfd;
            this->res = res;
        }

        auto start() -> void {
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
};