
// 컴퓨터망프로그래밍 assign#5 - client
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
    int level;
    int option;
    char name[30];
}opt;

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
	opt OPT[10]={{0,0,0}, 
			{SOL_SOCKET, SO_SNDBUF, "SO_SNDBUF\0"}, 
			{SOL_SOCKET, SO_RCVBUF, "SO_RCVBUF\0"},
			{SOL_SOCKET, SO_REUSEADDR, "SO_REUSEADDR\0"},
			{SOL_SOCKET, SO_KEEPALIVE, "SO_KEEPALIVE\0"},
			{SOL_SOCKET, SO_BROADCAST, "SO_BROADCAST\0"},
			{IPPROTO_IP, IP_TOS, "IP_TOS\0"},
			{IPPROTO_IP, IP_TTL, "IP_TTL\0"},
			{IPPROTO_TCP, TCP_NODELAY, "TCP_NODELAY\0"},
			{IPPROTO_TCP, TCP_MAXSEG, "TCP_MAXSEG\0"}};

	int sock;
    int num;
	char message[BUF_SIZE];
	int str_len;
	SO_PACKET packet;
	socklen_t adr_sz;
	
	struct sockaddr_in serv_adr, from_adr;
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	while(1)
	{
		printf("-------------------------------\n");
        printf(" 1: SO_SNDBUF\n");
        printf(" 2: SO_RCVBUF\n");
        printf(" 3: SO_REUSEADDR\n");
        printf(" 4: SO_KEEPALIVE\n");
        printf(" 5: SO_BROADCAST\n");
        printf(" 6: IP_TOS\n");
        printf(" 7: IP_TTL\n");
        printf(" 8: TCP_NODELAY\n");
        printf(" 9: TCP_MAXSEG\n");
        printf("10: Quit\n");
        printf("-------------------------------\n");
        
		// select option number
        while(1)
        {
            printf("Input option number: ");
            scanf("%d", &num);
            if(num==10)
            {
                printf("Client quit.\n");
                close(sock);
                return 0;
            }
            if(num<=0 || num>=11)
            {
                printf("Wrong number. type again!\n");
            }
            else break;
        }
        
		// set option to packet
        strcpy(packet.option_name, OPT[num].name);
        packet.level=OPT[num].level;
        packet.option=OPT[num].option;
		packet.optval=0;
		packet.result=1;

		// send packet to server
		sendto(sock, (const void*)&packet, sizeof(packet), 0, 
					(struct sockaddr*)&serv_adr, sizeof(serv_adr));

		// receive result packet from client
		adr_sz=sizeof(from_adr);
		str_len=recvfrom(sock, (void *)&packet, sizeof(SO_PACKET), 0, 
					(struct sockaddr*)&from_adr, &adr_sz);

		// print result
		printf(">>> Server result: %s: value: %d, result: %d\n\n", packet.option_name, packet.optval, packet.result);
	}	
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
