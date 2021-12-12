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
	int led = 13;

	int delay = 500000;
	int freq = 10000;
	int duty_min = 100000;
	
	// gpio 초기화
	gpioInitialise();
	gpioSetMode(up, PI_INPUT);
	gpioSetMode(down, PI_INPUT);
	gpioSetMode(led, PI_OUTPUT);
	
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
	printf("Press q of Q to QUIT\n");

	// 밝기 단계 변수
	int num = 1;
	int duty;
	while(1){
		// up 클릭 시 num 증가, down 클릭 시 num 감소
		if(gpioRead(up) == 0) {
			num++;
			// num이 10보다 클 때 처리 
			if(num > 10)
				num = 10;
			gpioDelay(delay);		
		}
		else if(gpioRead(down) == 0) {
			num--;
			// num이 0보다 작을 때 처리 
			if(num < 1)
				num = 1;
			gpioDelay(delay);		
		}
		// duty 값을 100,000~1,000,000 사이를 10단계로 표시하므로
		// 최소값인 100,000 * (단계=num)으로 표현 가능
		duty = num * duty_min;

		// 위에서 구한 duty 값으로 밝기 표현
		gpioHardwarePWM(led, freq, duty);
		
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
	gpioWrite(led, 0);
	gpioTerminate();
	
	return 0;
}

