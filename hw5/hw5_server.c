
// 컴퓨터망프로그래밍 assign#5 - server
// 2020112757 김유진

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#define BUF_SIZE 1024

typedef struct{
    char option_name[30];
    int level;
    int option;
    int optval;
    int result;
}SO_PACKET;

void error_handling(char *message);

int main(int argc, char *argv[])
{
    SO_PACKET packet;
	int serv_sock, tcp_sock;
    int option, buf;
	int str_len;
	socklen_t clnt_adr_sz, len;

	
	struct sockaddr_in serv_adr, clnt_adr;
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
    // UDP
	serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock==-1)
		error_handling("UDP socket creation error");

	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
    
    len = sizeof(option);
    option = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, len);


    // TCP
    tcp_sock=socket(PF_INET, SOCK_STREAM, 0);
	if(tcp_sock==-1)
		error_handling("TCP socket creation error");


    printf("Socket Option Server Start\n");
	while(1) 
	{
        // receive packet from clinet
		clnt_adr_sz=sizeof(clnt_adr);
		str_len=recvfrom(serv_sock, (void *)&packet, sizeof(SO_PACKET), 0, 
								(struct sockaddr*)&clnt_adr, &clnt_adr_sz);

        // get socket option
        len=sizeof(buf);
        memset(&buf, 0, len);
        packet.result = getsockopt(tcp_sock, packet.level, packet.option, (void *)&buf, &len);

        if(packet.result)
        error_handling("getsockopt() error");

        packet.optval = buf;

        // print result
		printf(">>> Received Socket option: %s\n", packet.option_name);
        printf("<<< Send option: %s: %d, result: %d\n\n", packet.option_name, packet.optval, packet.result);

        // send result packet to client
		sendto(serv_sock, (const void*)&packet, sizeof(packet), 0, 
								(struct sockaddr*)&clnt_adr, clnt_adr_sz);
	}	
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
