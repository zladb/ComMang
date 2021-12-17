#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>  

#define BUF_SIZE 2048
void error_handling(char* message);

int main(int argc, char *argv[])
{
    if(argc!=3){
        error_handling("[Error] mymove Usage: ./mymove src_file dest_file");
    }

    int fd1, fd2;
    int move_size;
    char buf[BUF_SIZE];

    fd1=open(argv[1], O_RDONLY);
    if(fd1==-1)
        error_handling("src.txt open() error!");

    move_size=read(fd1, buf, sizeof(buf));
    if(move_size==-1)
        error_handling("read() error!");

    fd2=open(argv[2], O_CREAT|O_RDWR, 0644);
    if(fd2==-1)
        error_handling("open() error!");

    if(write(fd2, buf, move_size)==-1)
        error_handling("write() error!");

    printf("move from %s to %s (bytes: %d) finished.\n", argv[1], argv[2], move_size);
    remove(argv[1]);

    close(fd1);
    close(fd2);
    return 0;
}

void error_handling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
