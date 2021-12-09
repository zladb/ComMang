#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 3

int main(int argc, char *argv[])
{
    FILE *f1;
    FILE *f2;
    char buf[BUF_SIZE];
    unsigned long long t1, t2;
	unsigned long long nano = 1000000000;
	struct timespec start, end;

    if(argc != 3) 
    {
        printf("Usage: %s <src_file> <dest_file>\n", argv[0]);
		return -1;
    }
    f1 = fopen(argv[1], "r");
    f2 = fopen(argv[2], "w");

	clock_gettime(CLOCK_REALTIME, &start);
	t1 = start.tv_nsec + start.tv_sec * nano;

    while(fgets(buf, BUF_SIZE, f1) != NULL)
    {
        fputs(buf, f2);
    }
	clock_gettime(CLOCK_REALTIME, &end);
	t2 = end.tv_nsec + end.tv_sec * nano;

    printf("Stdcpy elapsed time: %lld milliseconds\n", 
					(t2-t1)/1000000);
    fclose(f1);
    fclose(f2);
    return 0;
}
