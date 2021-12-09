// assign#7 multicast
// 2020112757 Yujin Kim

// Multicast 데이터 수신: recv_sock (SO_REUSEADDR 적용)
// Multicast 데이터 전송: send_sock (IP_MULTICAST_TTL=1 설정)
// fork()를 이용한 멀티프로세스 생성.
// 자식 프로세스: 데이터 수신
// 부모 프로세스: 데이터 전송
// $ hw7 <멀티캐스트주소> <포트번호> <이름>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define TTL 1

typedef struct
{
    char name[15];
    char message[BUF_SIZE];
}chat;

void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
	int send_sock, recv_sock;
	struct sockaddr_in mul_adr, recv_adr;
    struct ip_mreq join_adr;
    int opt;
	int time_live = TTL;
	pid_t pid;
    chat send, recv;

	if(argc != 4) 
    	{
		printf("Usage: %s <GroupIP> <Port> <Name>\n", argv[0]);
		exit(1);
	}

    strcpy(send.name, argv[3]);

    // parent socket (send messages)
	send_sock = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&mul_adr, 0, sizeof(mul_adr));
    mul_adr.sin_family=AF_INET;
	mul_adr.sin_addr.s_addr = inet_addr(argv[1]);
	mul_adr.sin_port=htons(atoi(argv[2]));

    opt = 1;
    setsockopt(send_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));

    // clild socket (receive messages)
    recv_sock=socket(PF_INET, SOCK_DGRAM, 0);

	memset(&recv_adr, 0, sizeof(recv_adr));
	recv_adr.sin_family=AF_INET;
	recv_adr.sin_addr.s_addr=htonl(INADDR_ANY);	
	recv_adr.sin_port=htons(atoi(argv[2]));
	
	setsockopt(recv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
    if(bind(recv_sock, (struct sockaddr*)&recv_adr, sizeof(recv_adr))==-1)
		error_handling("bind() error");
    
    join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]);
    join_adr.imr_interface.s_addr = htonl(INADDR_ANY);

    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));

    // start chat!
	pid = fork();
    if(pid == -1)
    {
        close(recv_sock);
        close(send_sock);
        printf("fork() error!\n");
    }
    if(pid == 0)
    {  
        close(send_sock);
        while(1)
        {
            recvfrom(recv_sock, &recv, sizeof(recv), 0, NULL, 0);
            printf("Received Message : [%s]  %s", recv.name, recv.message);
            memset(&recv, 0, sizeof(chat));
        }
    }
    else
    {  
        close(recv_sock);
        while(1)
        {
            fgets(send.message, BUF_SIZE, stdin);
            sendto(send_sock, &send, sizeof(chat), 0, (struct sockaddr*)&mul_adr, sizeof(mul_adr));
            memset(&send.message, 0, sizeof(send.message));
            sleep(1);
        }
    }
	close(send_sock);
    close(recv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
