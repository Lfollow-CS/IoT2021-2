#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pigpio.h>
#include <time.h>
#include <sys/time.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
	int str_len;
	char date[BUF_SIZE];
	struct sockaddr_in serv_adr;

    if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	// 현재시간을 출력하기위한 변수
    time_t t;
    struct tm tm;
	// 적외선 센서에 연결할 gpio 번호 설정
	int motion = 17;
	// 5초 타이머를 위한 시간 변수
	unsigned int start=0, end=0;
	// gpio 초기화
	gpioInitialise();
	gpioSetMode(motion, PI_INPUT);


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

	// select 사용을 위한 변수 초기화
	struct timeval timeout;
	fd_set readfds, readtemp;
	int max_fd = 0;
	int result;
	char message[10];

	// stdin, 0번 fd_set에 지정
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	max_fd = 0;
	printf("Press q of Q to QUIT\n");
    while(1) {
        memset(date, 0, BUF_SIZE);
		// 적외선 센서가 동작을 감지한 경우
		int n = gpioRead(motion);
        if(n == 1) {
			// 현재 타이머 측정
			end = gpioTick();
			// 현재 타이머와 기준 타이머의 차이가 5초 이상인 경우
			if((end - start) > 5000000){
				// 기준시간 재측정
				start = gpioTick();
				
				// 현재 시간을 측정하여 '2021/01/01 00 00'형태로 문자열 저장 
				t = time(NULL);
				tm = *localtime(&t);
				sprintf(date, "%04d/%02d/%02d %dh %dm %ds", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,tm.tm_hour, tm.tm_min, tm.tm_sec);
				
				// 저장한 문자열 서버 소켓으로 전송
				write(sock, date, sizeof(date));
				printf("동작감지 %s\n", date);
			}
		}
		gpioDelay(1000);
		
		readtemp = readfds;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		result = select(max_fd+1, &readtemp, NULL, NULL, &timeout);
		// select 1 리턴, stdin 이벤트 발생
		if(result == 1) {
			str_len = read(0, message, 10);
			//입력값이 q, Q인 경우 반복문 종료
			if(!strncmp(message,"q",1) || !strncmp(message,"Q",1)){
				break;
			}
		}
	}
	close(sock);
	gpioTerminate();
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}