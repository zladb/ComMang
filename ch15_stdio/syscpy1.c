#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 3

int main(int argc, char *argv[])
{
    int fd1, fd2;
    int len;
    char buf[BUF_SIZE];
	const unsigned long long nano = 1000000000;
    unsigned long long t1, t2;
	struct timespec start, end;

    if(argc != 3) 
    {
        printf("Usage: %s <src_file> <dest_file>\n", argv[0]);
		return -1;
    }

    fd1 = open(argv[1], O_RDONLY);
    fd2 = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC);
	
	//clock_gettime(CLOCK_REALTIME, &start);
	clock_gettime(CLOCK_MONOTONIC, &start);
	t1 = start.tv_nsec + start.tv_sec * nano;

    while((len=read(fd1, buf, sizeof(buf))) > 0)
        write(fd2, buf, len);

	//clock_gettime(CLOCK_REALTIME, &end);
	clock_gettime(CLOCK_MONOTONIC, &end);
	t2 = end.tv_nsec + end.tv_sec * nano;

    printf("syscpy elapsed time: %lld milliseconds\n", 
				(t2-t1)/1000000);

    close(fd1);
    close(fd2);
    return 0;
}
