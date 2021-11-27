
// assign #8
// 2020112757 Yujin Kim

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

typedef struct {
	int command;
	int seatno;
	int seats[21];
	int result;
} BUS_INFO;

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

BUS_INFO BUS;
int bus_seats[21]={0,};

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	printf("Bus Reservation System\n");

	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s, clnt_sock=%d \n", inet_ntoa(clnt_adr.sin_addr), clnt_sock);
	}
	close(serv_sock);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0, i;
	char msg[BUF_SIZE];
	
	while((str_len=read(clnt_sock, &BUS, sizeof(BUS_INFO)))!=0)
		{	
			memcpy(BUS.seats, bus_seats, sizeof(bus_seats));
			switch(BUS.command)
			{
				//inquiry
				case 1:
					for(int i=1; i<=20; i++){
							BUS.seats[i]= bus_seats[i];
					}
					BUS.result=0;
					write(clnt_sock, &BUS, sizeof(BUS_INFO));
				break;

				// reservation
				case 2:
					pthread_mutex_lock(&mutx);

					// Wrong seat numbder
					if (BUS.seatno<1 || BUS.seatno>20)
					{
						BUS.result = -1; 
						write(clnt_sock, &BUS, sizeof(BUS_INFO));
					}
					else 
					{
						// Reservation successed
						if(BUS.seats[BUS.seatno]==0)
						{
							bus_seats[BUS.seatno]=clnt_sock;
							memcpy(BUS.seats, bus_seats, sizeof(bus_seats));
							BUS.result = 0;
							write(clnt_sock, &BUS, sizeof(BUS_INFO));
						}

						// Reservation failed
						else{
							BUS.result = -2; 
							write(clnt_sock, &BUS, sizeof(BUS_INFO));
						}
					}
					pthread_mutex_unlock(&mutx);
					break;
				
				// cancellation
				case 3:
					pthread_mutex_lock(&mutx);

					// Cancellation failed. (The seat was not reserved.)		
					if(BUS.seats[BUS.seatno]==0){
						BUS.result = -3;
						write(clnt_sock, &BUS, sizeof(BUS_INFO));
					}

					// Cancellation failed. (The seat was reserved by another person.)
					else if(BUS.seats[BUS.seatno]!=clnt_sock){
						BUS.result = -4;
						write(clnt_sock, &BUS, sizeof(BUS_INFO));
					}

					// Cancel seccessed.
					else{
						bus_seats[BUS.seatno]=0; 	
						memcpy(BUS.seats, bus_seats, sizeof(bus_seats));					
						BUS.result = 0;
						write(clnt_sock, &BUS, sizeof(BUS_INFO));
					}
					pthread_mutex_unlock(&mutx);
					break;

				default:
					break;
			}
		}
	
	//printf("exit %d\n", clnt_sock);
	printf("clnt_sock=%d closed\n", clnt_sock);

	close(clnt_sock);
	return NULL;
}

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
