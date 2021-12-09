// 2020112757 Yujin Kim
// mid_exam2_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUF_SIZE 1024
#define SEQ_START 1000

typedef struct{
	int type;  // 0:SEQ, 1:ACK, 2:FIN, 3:FIN_ACK, 4:ETC
	int seq; // SEQ number
	int ack; // ACK number
	int buf_len; // File read/write bytes
	char buf[BUF_SIZE];
}Packet;

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock;
	FILE *fp;
	char fname[BUF_SIZE];

	Packet recv_packet, send_packet;
	int fread_cnt = 0;
	int first = 0; // First sending
	int total_cnt = 0;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	char error_message[20]="File Not Found";

	if(argc!=2){
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	memset(fname, 0, sizeof(fname));
	memset(&send_packet, 0, sizeof(Packet));
	memset(&recv_packet, 0, sizeof(Packet));

	//소켓 생성
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	//bind
	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("bind() error");

	//listen
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	clnt_addr_size = sizeof(clnt_addr);
	//accept
	clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
	if(clnt_sock==-1)
		error_handling("accept() error");

	printf("------------------------------------\n");
    printf("    File Transmission Server\n");
    printf("------------------------------------\n");

	/*------------------------------
	  Receive a file name from a client
	  ------------------------------*/

	read(clnt_sock, (void*)&recv_packet, sizeof(Packet));
	
	strncpy(fname, recv_packet.buf, recv_packet.buf_len);
	fname[recv_packet.buf_len]=0; // NULL 넣기

	fp=fopen(fname, "rb");
	if(fp != NULL)
	{
		printf("[Server] sending %s\n\n", fname);
		while(!feof(fp))
		{
			/*---------
			  File read
			  ---------*/
			fread_cnt = fread((void*)send_packet.buf, 1, BUF_SIZE, fp);

			//printf("fread_cnt = %d\n", fread_cnt);
			if(first == 0)
			{
				// SEQ number is the start number.(1000)
				send_packet.seq = SEQ_START;
				send_packet.ack = 0;
				send_packet.buf_len = fread_cnt;
				first = 1;
			}
			else
			{
				send_packet.seq = recv_packet.ack;
				send_packet.buf_len = fread_cnt;
			}
			/*------------------------------------------------
			  	Tx(write): Server --> Client
				- Send file contents and SEQ
				----------------------------------------------*/
			write(clnt_sock, (void*)&send_packet, sizeof(Packet));
			printf("[Server] Tx: SEQ: %d,  %d byte data\n", send_packet.seq, fread_cnt);
			total_cnt += fread_cnt;
			memset(&send_packet, 0, sizeof(Packet));
			/*-----------------------------------------
			  Rx(read): Server <-- Client
			  				Packet(ack)
			-------------------------------------------*/
			
			// if(fread_cnt >= BUF_SIZE)
			// {
				read(clnt_sock, (void*)&recv_packet, sizeof(Packet));
				printf("[Server] Rx ACK:%d\n\n", recv_packet.ack);
			//}
		}
		printf("%s sent (%d Bytes)\n", fname, total_cnt);
		fclose(fp);

			
	}
	else
	{
		printf("%s File Not Found\n", fname);
		send_packet.seq = 0;
		send_packet.type=1;
		strncpy(send_packet.buf, error_message, strlen(error_message));
		write(clnt_sock, (void*)&send_packet, sizeof(Packet));
	}

	//step 1
	memset(&send_packet, 0, sizeof(Packet));
	send_packet.seq=recv_packet.ack;
	send_packet.type=2;
	printf("[Server] Tx FIN (seq: %d)\n",send_packet.seq);
	
	if(write(clnt_sock, (void*)&send_packet, sizeof(Packet))==-1)
		error_handling("fdsfhdslfjlds\n");


	//step 4
	read(clnt_sock, (void*)&recv_packet, sizeof(Packet));
	printf("[Server] Rx FIN_ACK (seq: %d, ack: %d)\n", recv_packet.seq, recv_packet.ack);

		
	//step 6 
	read(clnt_sock, (void*)&recv_packet, sizeof(Packet));
	printf("[Server] Rx FIN (seq: %d, ack: %d)\n", recv_packet.seq, recv_packet.ack);

	//step 7
	//memset(&send_packet, 0, sizeof(Packet));
	send_packet.seq=recv_packet.ack;
	send_packet.ack=recv_packet.seq+1;
	send_packet.type=3;
	printf("[Server] Tx FIN_ACK (seq: %d, ack: %d)\n",send_packet.seq, send_packet.ack);
	
	if(write(clnt_sock, (void*)&send_packet, sizeof(Packet))==-1)
		error_handling("fdsfhdslfjlds\n");


	close(clnt_sock);
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
