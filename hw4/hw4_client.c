
// 컴퓨터망프로그래밍 
// Assign#4. Tic Tac Toe
// 2020112757 김유진


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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
    int sock;
	int str_len;
	socklen_t adr_sz;
	
	struct sockaddr_in serv_adr, from_adr;
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

    GAMEBOARD c;
    int row, col;

    // initialize board
    for(int i=0; i<BOARD_SIZE; i++){
        for(int j=0; j<BOARD_SIZE; j++){
            c.board[i][j]=0;
        }
    }

    printf("Tic-Tac-Toe Client\n");
    draw_board(&c);
    
    // while(1)
    while(1)
    {
        // input row, col
        do {
        printf("Input row, column: ");
        scanf("%d %d", &row, &col);
        }while((row<0 || row >2) || (col<0 || col>2) || c.board[row][col]!=0);

        // update board
        c.board[row][col]=2;
        draw_board(&c);

        // send board to server
        sendto(sock, c.board, sizeof(c.board), 0, 
                        (struct sockaddr*)&serv_adr, sizeof(serv_adr));

        // end game
        if(available_space(&c)==0){
            printf("No available space. Exit Client\n");
            printf("Tic Tac Toe Client Close\n");
            close(sock);
            return 0;
        }

        // receive board from server
        adr_sz=sizeof(from_adr);
        recvfrom(sock, (void*)c.board, sizeof(c.board), 0, 
                        (struct sockaddr*)&from_adr, &adr_sz);

        draw_board(&c);
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