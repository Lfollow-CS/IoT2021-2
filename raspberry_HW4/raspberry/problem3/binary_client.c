#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pigpio.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
	char move[BUF_SIZE], how[BUF_SIZE];
	int str_len;
	struct sockaddr_in serv_adr;

    if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	// led에 연결할 gpio 번호 설정
	int led0 = 5;
	int led1 = 6;
	int led2 = 13;
	int led3 = 19;
	int led[4] = {led0,led1,led2,led3};

	// gpio 초기화
	gpioInitialise();
	gpioSetMode(led[0], PI_OUTPUT);
	gpioSetMode(led[1], PI_OUTPUT);
	gpioSetMode(led[2], PI_OUTPUT);
	gpioSetMode(led[3], PI_OUTPUT);
	
	// 소켓 생성
    sock=socket(PF_INET, SOCK_STREAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	// 서버 소켓 정보 등록
    memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	// 연결 요청
	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");

    
	// 현재 표시할 숫자를 저장하는 변수
	int num = 0;
    while(1) {
        memset(move, 0, BUF_SIZE);
		memset(how, 0, BUF_SIZE);
		// 클라이언트로부터 메세지 수신
		str_len=read(sock, move, BUF_SIZE);
		move[str_len]=0;
		printf("Message from server: %s", move);
		// 메세지 내에서 이동할 양을 추출
		int i = 2;
		int j = 0;
		while(1){
			if(move[i]=='\0'){
				how[j] = '\0';
				break;
			}
			how[j] = move[i];
			i++;
			j++;
		}
		// 메세지에서 이동 방향(+, -) 추출
		if(move[0] == 'u') {
			num += atoi(how);
			// num이 16보다 클 때 처리
			if(num >= 16)
				num = num % 16;
			gpioDelay(10000);
			printf("binary number : %d\n", num);
		}
		else if(move[0] == 'd') {
			num -= atoi(how);
			// num이 음수일 때 처리 
			if(num < 0)
				num = 0;
			gpioDelay(10000);
			printf("binary number : %d\n", num);
		}
		// 서버로부터 q, Q 수신 시 종료
		else if(move[0] == 'q' || move[0] == 'Q' || str_len == 0){
			printf("\nServer Send End Signal\n");
			printf("End Connection\n");
			break;
		}

		// 비트 연산을 통해 LED 표시
		for(i = 3;i >= 0; i--){
			int result = num >> i & 1;
			gpioWrite(led[i], result);
		}
	}
	close(sock);
	// gpio 종료
	gpioWrite(led[0], 0);
	gpioWrite(led[1], 0);
	gpioWrite(led[2], 0);
	gpioWrite(led[3], 0);
	gpioTerminate();
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}