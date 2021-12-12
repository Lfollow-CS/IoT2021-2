#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE 1024

// 에러 처리 함수와 자식 프로세스 종료 확인 시그널 핸들러
void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[]){
    int serv_sock, clnt_sock, str_len;
	struct sockaddr_in serv_adr, clnt_adr;
	
    pid_t pid;
	struct sigaction act;

	socklen_t adr_sz;
	int state;
	char move[BUF_SIZE], how[BUF_SIZE];
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	// 자식 프로세스 종료 시그널 설정
    act.sa_handler=read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	state=sigaction(SIGCHLD, &act, 0);

	// 서버 소켓 정보(프로토콜, IP 주소, PORT 번호) 정의
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	// PORT 번호 반복 재사용을 위한 옵션 지정
	int option = 1;
	socklen_t optlen = sizeof(option);
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);

    // 앞서 등록한 정보로 소켓 bind
    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	// 접속 요청 대기열 5로하여 listen
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	// 별도의 종료 시그널이 없으면 무한 반복 하도록 함
    while(1){
		// 접속 요청 accept,
		// 접속 요청이 없으면 이후의 코드는 실행하지 않고 다시 while 처음부터 실행
        adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
        if(clnt_sock==-1)
			continue;
        else
			puts("new client connected...");

		//accept 성공 했으면 자식 프로세스 생성
        pid = fork();
		
		// 자식 프로세스의 내용
        if(pid == 0){
			// 사용하지 않는 서버 소켓은 종료
            close(serv_sock); 
			while(1){
				memset(move, 0, BUF_SIZE);
				memset(how, 0, BUF_SIZE);
                
				// 라즈베리파이에서 메세지 수신
		        str_len=read(clnt_sock, move, BUF_SIZE);
                if(str_len <= 0)
                    break;
		        move[str_len]=0;

				// 메세지 수신한 소켓의 IP와 move 출력, move에는 시간을 나타내는 메세지
				printf("IP: %s 동작감지 (%s)\n", inet_ntoa(clnt_adr.sin_addr), move);
			}
			close(clnt_sock);
			return 0;
		}
		else
			close(clnt_sock);
    }
	close(serv_sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
void read_childproc(int sig)
{
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG);
	printf("removed proc id: %d \n\n", pid);
}