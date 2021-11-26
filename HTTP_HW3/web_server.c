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

// jpg, png, html에 대한 HTTP response 헤더 정의
// 파일이 존재하지 않는 경우는 404 response 헤더 정의
char imgheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: image/jpg\r\n\r\n";
char pngheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: image/png\r\n\r\n";
char htmlheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/html\r\n\r\n";
char notfound[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";

// 에러 처리 함수와 자식 프로세스 종료 확인 시그널 핸들러
void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[]){
    int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int fd;
	
    pid_t pid;
	struct sigaction act;
	socklen_t adr_sz;
	int str_len, state;
	char buf[BUF_SIZE];
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

			char filename[BUF_SIZE];
			char pwd[100];
			char errpwd[100];
			char* cmpstr;
			memset(filename, 0, BUF_SIZE);

			// 클라이언트에서 보낸 request message를 buf에 저장
            str_len=read(clnt_sock, buf, BUF_SIZE);
            if(str_len == 0 || str_len == -1)
				error_handling("read() error");
            else
    		    buf[str_len]=0;

			// buf에서 "GET "로 시작하는 부분을 찾아서 포인터로 설정
			// +4하는 이유는 포인터가 가리키는 지점을 
			// "/파일이름"이라면 /를 가리키도록 하기 위함
			cmpstr = strstr(buf, "GET ");
			cmpstr += 4;
			int i = 0;
			// "/파일이름"형태를 filename 에 저장
			while(1){
				if(cmpstr[i]==' '){
					filename[i] = '\0';
					break;
				}
				filename[i] = cmpstr[i];
				i++;
			}
			//현재 작업 경로 뒤에 filename을 붙여서 파일의 위치를 찾음
			getcwd(pwd, BUF_SIZE);
			strcat(pwd, filename);

			// 찾은 파일 위치의 파일을 open
			// 각 파일의 경식에 맞춰 앞서 설정한 response message header를 
			// 클라이언트에 전송
			// 파일이 없는 경우 앞서 설정한 파일의 위치를 open하는 것이아니라
			// notfound.html을 다시 open하여 fd에 설정

			// 파일이 없는 경우
			if((fd = open(pwd, O_RDONLY))==-1){
				getcwd(errpwd, BUF_SIZE);
				strcat(errpwd, "/notfound.html");
				fd = open(errpwd, O_RDONLY);

				write(clnt_sock, notfound, sizeof(notfound)-1);
				printf("* Not Found *\n");
			}
			// 파일이 .jpg
			else if(strstr(filename, ".jpg")!=NULL){
				write(clnt_sock, imgheader, sizeof(imgheader)-1);
				printf("* Found jpg *\n");
			}
			// 파일이 .png
			else if(strstr(filename, ".png")!=NULL){
				write(clnt_sock, pngheader, sizeof(pngheader)-1);
				printf("* Found png *\n");
			}
			// 파일이 .html
			else if(strstr(filename, ".html")!=NULL){
				write(clnt_sock, htmlheader, sizeof(htmlheader)-1);
				printf("* Found html *\n");
			}

			// fd가 가리키는 파일의 내용을 클라이언트로 송신
			while((str_len = read(fd, buf, BUF_SIZE))>0){
				write(clnt_sock, buf, str_len);
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

void read_childproc(int sig)
{
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG);
	printf("removed proc id: %d \n\n", pid);
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}