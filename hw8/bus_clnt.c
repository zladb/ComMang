
// assign #8
// 2020112757 Yujin Kim

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
	
#define BUF_SIZE 100
#define NAME_SIZE 20

typedef struct {
	int command;
	int seatno;
	int seats[21];
	int result;
} BUS_INFO;
	
void * send_info(void * arg);
void * recv_info(void * arg);
void print_seat(BUS_INFO BUS);
void error_handling(char * msg);
	

BUS_INFO BUS;
pthread_mutex_t mutx;
static sem_t sem_one;
static sem_t sem_two;
	
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	sem_init(&sem_one, 0, 0);
    sem_init(&sem_two, 0, 1);

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	 }
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	pthread_create(&snd_thread, NULL, send_info, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_info, (void*)&sock);

	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);

	close(sock);  
	return 0;
}
	
void * send_info(void * arg)   // send thread main
{
	int sock=*((int*)arg);
	int num;
	while(1) 
	{	
		memset(&BUS, 0, sizeof(BUS_INFO));
		sem_wait(&sem_two); // sem_two의 값을 0으로 설정 
		printf("1: inquiry, 2: reservation, 3:cancellation, 4: quit: ");
		scanf("%d", &BUS.command);

		switch(BUS.command)
		{
			case 1:
				write(sock, &BUS, sizeof(BUS_INFO));
				break;

			case 2:
				printf("Input seat number: ");
				scanf("%d", &BUS.seatno);
				write(sock, &BUS, sizeof(BUS_INFO));
				break;

			case 3:
				printf("Input seat number for cancellation: ");
				scanf("%d", &BUS.seatno);
				write(sock, &BUS, sizeof(BUS_INFO));
				break;

			case 4:
				printf("Quit.\n");
				close(sock);
				exit(0);
				break;
			
			default:
				printf("wrong number!\n");
				sem_post(&sem_two); // sem_two의 값을 1으로 설정 
				continue;
				break;
		}
		sem_post(&sem_one); // sem_one의 값을 1로 설정 
	}
	return NULL;
}
	
void * recv_info(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	int str_len;
	while(1)
	{
		sem_wait(&sem_one); // sem_two의 값을 0으로 설정 
		str_len=read(sock, &BUS, sizeof(BUS_INFO));
		if(str_len==-1) 
			return (void*)-1;

		if (BUS.result==0)
		{
			print_seat(BUS);
			printf("Operation success.\n");
		}
		else if (BUS.result==-1)
		{
			print_seat(BUS);
			printf("Wrong seat number.\n");
		}
		else if (BUS.result==-2)
		{
			print_seat(BUS);
			printf("Reservation failed (The seat was already reserved.)\n");
		}
		else if (BUS.result==-3)
		{
			print_seat(BUS);
			printf("Cancellation failed (The seat was not reserved.)\n");
		}
		else if (BUS.result==-4)
		{
			print_seat(BUS);
			printf("Cancellation failed (The seat was reserved by another person.)\n");
		}

		sem_post(&sem_two); // sem_one의 값을 1로 설정 
	}
	return NULL;
}

void print_seat(BUS_INFO BUS)
{
	printf("\n----------------------------------------\n");
	for(int i=1; i<=10; i++) printf("%3d ", i);
	printf("\n");
	for(int i=1; i<=10; i++) printf("%3d ", BUS.seats[i]);
	printf("\n----------------------------------------\n");
	for(int i=11; i<=20; i++) printf("%3d ", i);
	printf("\n");
	for(int i=11; i<=20; i++) printf("%3d ", BUS.seats[i]);
	printf("\n----------------------------------------\n");
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
