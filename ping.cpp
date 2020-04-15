
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <chrono>

#define PACKET_SIZE 64
#define TIME_OUT 1

typedef struct icmp_packet {
    struct icmphdr hdr;
    char msg[PACKET_SIZE-sizeof(struct icmphdr)];

} icmp_packet;

int run = 1;

int count_flag = 0;
int count_val;

int wait_flag = 0;
int wait_val;

class Ping {
    private:
        int sockfd;
        addrinfo* res;

        int seq = 0;
        long double total_rtt_usec = 0;
        int received_messages = 0;

        auto set_hoplimit(int ttl) -> void {
            if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) { 
                std::cout << "Setting socket options to TTL failed!" << std::endl; 
            } else { 
                std::cout << "Socket set to TTL..\n" << std::endl; 
            } 
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

        auto get_ip() -> char* {
            struct sockaddr_in *addr;
            addr =  (struct sockaddr_in *)res->ai_addr;

            return inet_ntoa((struct in_addr)addr->sin_addr);
        }

        auto get_packetloss() -> float {
            return ((seq - received_messages)/(float)seq) * 100.0;
        }

    public:
        Ping(int sockfd, addrinfo* res) {
            this->sockfd = sockfd;
            this->res = res;
        }

        auto set_config(int argc, char **argv) -> void {
            int opt;

            while ((opt = getopt(argc, argv, "t:w:c:")) != -1) {
                switch (opt) {
                    case 't':
                        set_hoplimit(atoi(optarg));
                        break;
                    case 'w':
                        wait_flag = 1;
                        wait_val = atoi(optarg) * 1000;
                        break;            
                    case 'c':
                        count_flag = 1;
                        count_val = atoi(optarg);
                        break;
                }
            }


            struct timeval tv_out; 
            tv_out.tv_sec = TIME_OUT; 
            tv_out.tv_usec = 0; 
            setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof(tv_out)); 
        }

        auto start() -> void {
            icmp_packet packet;
            struct sockaddr_storage recv_addr;
            socklen_t recv_len = sizeof(struct sockaddr_storage);

            int send, recv, flag;
            
            do {
                flag = 1;
                bzero(&packet, sizeof(packet));

                packet.hdr.type = ICMP_ECHO; 
                packet.hdr.un.echo.id = getpid(); 
                packet.hdr.un.echo.sequence = seq++; 
                packet.hdr.checksum = checksum(&packet, sizeof(packet));

                auto start = std::chrono::high_resolution_clock::now();

                if ((send = sendto(sockfd, &packet, sizeof(packet), 0, res->ai_addr, res->ai_addrlen)) < 0) { 
                    std::cout << "Failed to send package." << std::endl; 
                    flag = 0;
                    seq--;
                } 

                if ((recv = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&recv_addr, &recv_len)) < 0) {
                    std::cout << "Failed to receive package" << std::endl;

                } else if (packet.hdr.type == 69 and packet.hdr.code == 0 && flag) {
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
                    
                    total_rtt_usec += duration.count();
                    received_messages++;

                    std::cout << recv << " bytes from: " << get_ip() <<"; icmp_seq=" << seq << "; time: " << duration.count()/1000.0 << "msec" << std::endl;   
                    
                } else {
                    std::cout << "Error: Packet received with ICMP type: " << packet.hdr.type << "and code: " << packet.hdr.code << std::endl;
                }  
                
                wait_flag ? usleep(wait_val) : sleep(1);\

                if (count_flag and seq >= count_val) {
                    break;
                }

            } while (run);
        }

        auto print_statistics() -> void {
            std::cout << std::endl;
            std::cout << "====| ping statistics |====" << std::endl;
            std::cout << "Packets transmitted: " << seq << std::endl;
            std::cout << "Packets received:  "<< received_messages << std::endl;
            std::cout << "Packet loss: " << get_packetloss() << "%" << std::endl;
            std::cout << "Total time: " << total_rtt_usec/1000.0 << " msec" << std::endl;
        }
};