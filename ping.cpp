
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <ctime>
#include <chrono>

#define PACKET_SIZE 64

typedef struct icmp_packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE-sizeof(struct icmphdr)];

} icmp_packet;

class Ping {
    private:
        int sockfd;
        addrinfo* res;
        int run;

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
            run = 1;
        }

        auto start() -> void {
            icmp_packet packet;
            struct sockaddr_storage recv_addr;
            socklen_t recv_len = sizeof(struct sockaddr_storage);

            int send, recv;
            int seq = 0;
            
            //TODO:Compute metrics
            while(run) {
                bzero(&packet, sizeof(packet));

                packet.hdr.type = ICMP_ECHO; 
                packet.hdr.un.echo.id = getpid(); 
                packet.hdr.un.echo.sequence = seq++; 
                packet.hdr.checksum = checksum(&packet, sizeof(packet));

                //send packet 
                usleep(1000000);
                
                auto start = std::chrono::high_resolution_clock::now();

                if ((send = sendto(sockfd, &packet, sizeof(packet), 0, res->ai_addr, res->ai_addrlen)) < 0) { 
                    std::cout << "Failed to send package." << std::endl; 
                } 

                //recv package
                if ((recv = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&recv_addr, &recv_len)) < 0) {
                    std::cout << "Failed to receive package: " << recv << std::endl;
                }

                auto end = std::chrono::high_resolution_clock::now();

                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
                std::cout << recv << " bytes from" << " placeholder " <<": icmp_seq=" << seq << " time: " << duration.count()/1000.0 << std::endl;          
            } 
        }
};