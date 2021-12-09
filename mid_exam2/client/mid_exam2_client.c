#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

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

int main(int argc, char* argv[])
{
	int sock;
	int file_len;
	char fname[20];
	char file_name[BUF_SIZE];
	FILE *fp;
	struct sockaddr_in serv_addr;
	Packet send_packet, recv_packet;

	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	//소켓 생성
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");

	memset(fname, 0, sizeof(fname));
	memset(&send_packet, 0, sizeof(Packet));
	memset(&recv_packet, 0, sizeof(Packet));

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	//연결 요청
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error!");

	printf("Input file name: ");
	fgets(file_name, 100, stdin);
	strncpy(fname, file_name, strlen(file_name)-1);
	/*---------------------------------------------
	  Send the filename and remove a carriage return('\n)
	  ---------------------------------------------*/
	send_packet.ack = 0;
	send_packet.buf_len = strlen(fname);
	strncpy(send_packet.buf, fname, send_packet.buf_len);
	write(sock, (void*)&send_packet, sizeof(Packet));
	printf("[Client] request %s\n", fname);
	/*------------------------------------------------
	  Rx(read): Server --> Client
	  - read(packet) -> fwrite()
	  - read_cnt: total packet size
	  ------------------------------------------------*/
	int found = 0;
	int first = 1;
	int total_cnt = 0;
	int fread_cnt = 0;
	while(1)
	{
		read(sock, (void*)&recv_packet, sizeof(Packet));
		if(recv_packet.type==2) break;
		//if(strncmp(recv_packet.buf, "File Not Found", recv_packet.buf_len)==0)
		if(recv_packet.type==1)
		{
			found = 0;
			printf("File Not Found\n");
			break;
		}
		else
		{
			printf("[Client] Rx SEQ: %d, len: %d bytes\n", recv_packet.seq, recv_packet.buf_len);
			// File open for saving a file (write mode)
			if(first == 1)
			{
				fp = fopen(fname, "wb");
				first = 0;
				found = 1;
			}

			fread_cnt = recv_packet.buf_len;
			total_cnt += fread_cnt;
			/*--------------------------------------------------
			  File write: fwrite
			  - only write a file using the actual size sent
			  --------------------------------------------------*/
			fwrite((void*)recv_packet.buf, 1, fread_cnt, fp);
			/*---------------------------------------------------
			  Tx(write): Server <-- Client
			  -Send ACK to server
			  - ACK = recv_packet.seq+recv_packet.buf_len+1
			  ---------------------------------------------------*/
			send_packet.ack = recv_packet.seq + fread_cnt + 1;
			// if(fread_cnt >= BUF_SIZE)
			// {
				write(sock, (void*)&send_packet, sizeof(Packet));
				printf("[Client] Tx ACK: %d\n\n", send_packet.ack);
			//}
			memset(&send_packet, 0, sizeof(Packet));
			memset(&recv_packet, 0, sizeof(Packet));
		}
	}

	if(found == 1)
	{
		printf("%s received (%d Bytes)\n", fname, total_cnt);
		fclose(fp);
	}

	//step 2
	// if(read(sock, (void*)&recv_packet, sizeof(Packet))==-1)
	// 	error_handling("fdskljfldsk");
	//read(sock, (void*)&recv_packet, sizeof(Packet));
	printf("[Client] Rx FIN (seq: %d)\n", recv_packet.seq);

	// 3
	send_packet.seq=50000;
	send_packet.type=3;
	send_packet.ack=recv_packet.seq;
	printf("Tx FIN_AKC (seq: %d, ack: %d)\n", send_packet.seq, send_packet.ack);
	write(sock, (void*)&send_packet, sizeof(Packet));

	//step 5
	send_packet.seq=50000 + 1;
	send_packet.type=2; 
	printf("[Client] Tx FIN (seq: %d, ack:%d)\n", recv_packet.seq, recv_packet.ack);
	write(sock, (void*)&send_packet, sizeof(Packet));

	// 8
	read(sock, (void*)&recv_packet, sizeof(Packet));
	printf("[Client] Tx FIN_ACK (seq: %d, ack:%d)\n", recv_packet.seq, recv_packet.ack);


	printf("[Client] Tx FIN_ACK (seq: %d, ack:%d)\n", recv_packet.seq, recv_packet.ack);
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
