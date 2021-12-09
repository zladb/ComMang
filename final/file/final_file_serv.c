
// 컴망 기말고사 2020112757 김유진
// final_exam_serv.c

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

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

// 전역변수 선언
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
pthread_mutex_t file_mutx;
FILE * fp;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
    char buf[BUF_SIZE];
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

    // 파일을 엽니다. 
    if((fp = fopen("squidgame.txt", "rb")) == NULL)
        error_handling("fopen() error");

    // 뮤텍스 초기화
	pthread_mutex_init(&mutx, NULL);
    pthread_mutex_init(&file_mutx, NULL);

    // tcp로 소켓 생성
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
        // 연결 요청
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
        // 뮤텍스 잠금 및 해제
		pthread_mutex_lock(&file_mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&file_mutx);

	
        // 쓰레드 생성
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));

        // // 파일에 저장된 기존 채팅내용 전송
        // pthread_mutex_lock(&file_mutx);
		// sleep(1);
        // while(!feof(fp))
        // {
        //     fgets(buf, BUF_SIZE, fp);
		// 	printf("%s", buf);
        //     write(clnt_sock, buf, sizeof(buf));
        // }
        // pthread_mutex_unlock(&file_mutx);
	}
	close(serv_sock);
    fclose(fp);
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0, i;
	char msg[BUF_SIZE];
	
    // 클라이언트로부터 온 내용 읽어서 msg에 저장
	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0){
		msg[str_len]='\0';
		send_msg(msg, str_len);

        // 파일에 채팅 내용 저장
        //pthread_mutex_lock(&file_mutx);
        fwrite(msg, str_len, 1, fp);
		//fclose(fp);
        //pthread_mutex_unlock(&file_mutx);
    }
	
    // 클라이언트가 종료 되었을 때 
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++ < clnt_cnt)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
    // fclose(fp);
	return NULL;
}

void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
