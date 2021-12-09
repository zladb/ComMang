
// 컴퓨터망프로그래밍 assign#6 - select (client)
// 2020112757 김유진

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 2048

void error_handling(char *message);

int main(int argc, char *argv[])
{
	char text[BUF_SIZE], buf[BUF_SIZE];
    int text_len, str_len;
	struct sockaddr_in serv_adr;
    fd_set reads, cpy_reads;
    struct timeval timeout;
	// fd1 : file open descripter, fd2: sever connect descripter
    int fd1, fd2, fd_max, fd_num;  

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

    // open file
    fd1=open("rfc1180.txt", O_RDONLY, 0644);
    if(fd1==-1)
        error_handling("open() error!");
	
	fd2=socket(PF_INET, SOCK_STREAM, 0);   
	if(fd2==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if(connect(fd2, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");
	

    FD_ZERO(&reads);
    FD_SET(fd1, &reads);
    FD_SET(fd2, &reads);
    fd_max = fd2; // 4

	while(1)
	{
        cpy_reads = reads;  // copy original
        timeout.tv_sec = 3;


		text_len=read(fd1, text, BUF_SIZE);
		if (text_len <= 0) 
			FD_CLR(fd1, &reads);

		// fd_num == -1 :  error
		if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1)
			break;  

		// there is no change
		if(fd_num == 0)
			continue;


		for (int i=0; i<fd_max+1 ; i++) // 0~4
		{
			if(FD_ISSET(i, &cpy_reads))
			{
				if(i == fd1)  // (i==3) when file is read
				{
					// send text
					write(fd2, text, text_len);
					sleep(1);
				}

				if(i == fd2)  // (i==4)when server writes messages
				{
					str_len=read(fd2, buf, BUF_SIZE);
					if(str_len==0){
						FD_CLR(i, &reads);
					}
					buf[str_len]=0;
					printf("%s", buf);
				}
			}
		}
		memset(buf, 0, sizeof(buf));
		memset(text, 0, sizeof(text));
	}
	close(fd1);
	close(fd2);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
