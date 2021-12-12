#include <stdio.h>
#include <pigpio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

int main(){
	// 버튼에 연결할 gpio 번호 설정
	int up = 23;
	int down = 24;

	// led에 연결할 gpio 번호 설정
	int led0 = 5;
	int led1 = 6;
	int led2 = 13;
	int led3 = 19;
	
	int led[4] = {led0,led1,led2,led3};
	
	// gpio 초기화
	gpioInitialise();
	gpioSetMode(up, PI_INPUT);
	gpioSetMode(down, PI_INPUT);
	gpioSetMode(led[0], PI_OUTPUT);
	gpioSetMode(led[1], PI_OUTPUT);
	gpioSetMode(led[2], PI_OUTPUT);
	gpioSetMode(led[3], PI_OUTPUT);
	
	// select 사용을 위한 변수 초기화
	struct timeval timeout;
	fd_set readfds, readtemp;
	int max_fd = 0;
	int result, str_len;
	char message[10];

	// stdin, 0번 fd_set에 지정
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	max_fd = 0;
	printf("Press q or Q to QUIT\n");

	// 현재 표시할 숫자를 저장하는 변수
	int num = 0;
	while(1){ 
		// up 클릭 시 num 증가, down 클릭 시 num 감소
		if(gpioRead(up) == 0) {
			num++;
			// num이 16보다 클 때 처리
			if(num >= 16)
				num = num % 16;
			gpioDelay(500000);
			printf("binary number : %d\n", num);
		}
		else if(gpioRead(down) == 0) {
			num--;
			// num이 음수일 때 처리 
			if(num < 0)
				num = 0;
			gpioDelay(500000);
			printf("binary number : %d\n", num);
		}
			
		// 비트 연산을 통해 LED 표시
		for(int i = 3;i >= 0; i--){
			int result = num >> i & 1;
			gpioWrite(led[i], result);
		}

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
	// gpio 종료
	gpioWrite(led[0], 0);
	gpioWrite(led[1], 0);
	gpioWrite(led[2], 0);
	gpioWrite(led[3], 0);
	gpioTerminate();

	return 0;
}
