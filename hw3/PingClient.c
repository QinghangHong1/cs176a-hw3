//the starter code was copied from linuxhowtos.org/data/6/client_udp.c

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void error(const char *);
int main(int argc, char *argv[])
{
	int sock, n;
	unsigned int length;
	struct sockaddr_in server, from;
	struct hostent *hp;
	char buffer[128];
	double rtt[10];
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	if (argc != 3) { printf("Usage: server port\n");
		exit(1);
	}
	sock= socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) error("socket");

	server.sin_family = AF_INET;
	hp = gethostbyname(argv[1]);
	if (hp==0) error("Unknown host");

	bcopy((char *)hp->h_addr,
			(char *)&server.sin_addr,
			hp->h_length);
	server.sin_port = htons(atoi(argv[2]));
	length=sizeof(struct sockaddr_in);
	
	// taking input
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
	int total_packets = 10;
        int received_packets = 0;
	int loss_packets = 0;       
	for(int i = 1; i <=10; i++){
		bzero(buffer,128);
		time_t time_send = time(NULL);
		struct timespec time_start, time_end;
		sprintf(buffer, "PING %d %ld\n", i, time_send);
		clock_gettime(CLOCK_MONOTONIC, &time_start);
		n=sendto(sock,buffer,
				strlen(buffer),0,(const struct sockaddr *)&server,length);
		if (n < 0) error("Sendto");
		bzero(buffer,128);
	
	// receive data from server
	//while(1){
		n = recvfrom(sock,buffer,128,0,(struct sockaddr *)&from, &length);
		
		if (n < 0){
			loss_packets ++;
			printf("Request timeout for icmp_seq %d\n", i);
			
		}else{
			clock_gettime(CLOCK_MONOTONIC,&time_end);
			double timeElapsed = ((double)(time_end.tv_nsec -
                                 time_start.tv_nsec))/1000000.0;
           double  rtt_msec = (time_end.tv_sec-
                          time_start.tv_sec) * 1000.0
                        + timeElapsed;
	   		rtt[received_packets] = rtt_msec;
	   		received_packets ++;
			printf("PING received from %s: seq#=%d time=%lf ms\n",hp->h_name, i, rtt_msec);
		}
	}
	
	close(sock);
	double max_rtt = 0;
	double total_rtt = 0;
	double min_rtt = 100000000;
	for(int i = 0; i < received_packets; i++){
		if(rtt[i] > max_rtt){
			max_rtt = rtt[i];
			printf("%ls\n", max_rtt);
		}
		if(rtt[i] < min_rtt){
			min_rtt = rtt[i];
		}
		total_rtt += rtt[i];
	}
	double mean_rtt = total_rtt / received_packets;
	int percentage = (((double)(received_packets)) / total_packets) * 100;
	printf("%d packets transmitted, %d received, %d%% packet loss rtt min/avg/max = %lf %lf %lf ms\n", total_packets, received_packets, percentage, min_rtt, mean_rtt, max_rtt);

	return 0;
}

void error(const char *msg)
{
	perror(msg);
	exit(0);
}
