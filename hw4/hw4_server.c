
// 컴퓨터망프로그래밍 
// Assign#4. Tic Tac Toe
// 2020112757 김유진

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BOARD_SIZE 3 
#define INIT_VALUE 0
#define S_VALUE 1
#define C_VALUE 2

typedef struct { 
    int board[BOARD_SIZE][BOARD_SIZE]; 
}GAMEBOARD;

void draw_board(GAMEBOARD *gboard);
int available_space(GAMEBOARD *gboard);
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock;
	int str_len;
	socklen_t clnt_adr_sz;
	
	struct sockaddr_in serv_adr, clnt_adr;
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock==-1)
		error_handling("UDP socket creation error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

    GAMEBOARD s;
    int row, col;

    // initialize board
    for(int i=0; i<BOARD_SIZE; i++){
        for(int j=0; j<BOARD_SIZE; j++){
            s.board[i][j]=0;
        }
    }

    printf("Tic-Tac-Toe Server\n");
    draw_board(&s);

    while(1)
    {
        // receive board from client
		clnt_adr_sz=sizeof(clnt_adr);
		recvfrom(serv_sock, (void*)s.board, sizeof(s.board), 0, 
								(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		draw_board(&s);

        // end game
        if(available_space(&s)==0){
            printf("No available space. Exit this program.\n");
            printf("Tic Tac Toe Server Close\n");
            close(serv_sock);
            return 0;
        }

        // input random integer to board
        do{
            srand((int)time(NULL));
            row = rand()%3;
            col = rand()%3;
        }while(s.board[row][col]!=0);
        s.board[row][col]=1;

        // print integer;
        printf("Server choose: [%d, %d]\n", row, col);
        draw_board(&s);

        // send board to clinet
		sendto(serv_sock, s.board, sizeof(s.board), 0, 
								(struct sockaddr*)&clnt_adr, clnt_adr_sz);
    }
}

void draw_board(GAMEBOARD *gboard) 
{ 
    char value = ' '; 
    int i, j; 
    printf("+-----------+\n"); 
    for (i=0; i<BOARD_SIZE; i++) 
    { 
        for (j=0; j<BOARD_SIZE; j++) 
        { 
            if(gboard->board[i][j] == INIT_VALUE)  // 초기값 0 
                value = ' '; 
            else if(gboard->board[i][j] == S_VALUE) // Server 표시 (1) 
                value = 'O'; 
            else if(gboard->board[i][j] == C_VALUE) // Client 표시 2 
                value = 'X'; 
        else 
            value = ' '; 

        printf("| %c ", value); 
    } 
    printf("|"); 
    printf("\n+-----------+\n"); 
    } 
}

int available_space(GAMEBOARD *gboard)
{
    int result=0;
    for(int i=0; i<BOARD_SIZE; i++){
        for(int j=0; j<BOARD_SIZE; j++){
            if(gboard->board[i][j]==0){
                result=1;
                goto out;
            }
        }
    }
    out:
    return result;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}