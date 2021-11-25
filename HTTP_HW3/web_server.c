#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

char imgheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: image/jpg\r\n\r\n";
char pngheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: image/png\r\n\r\n";
char htmlheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/html\r\n\r\n";
char notfound[] = "HTTP/1.1 404 Not Found Ok\r\nContent-Type: text/html\r\nContent-Length: 137\r\n";

void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[]){
    int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int fds[2];
	
    pid_t pid;
	struct sigaction act;
	socklen_t adr_sz;
	int str_len, state;
	char buf[BUF_SIZE];
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
    act.sa_handler=read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	state=sigaction(SIGCHLD, &act, 0);

    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

    while(1){
        adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
        if(clnt_sock==-1)
			continue;
        else
			puts("new client connected...");
        
        pid = fork();
        if(pid == 0){
            close(serv_sock);
			char filename[BUF_SIZE];
			char pwd[BUF_SIZE];
			char* cmpstr;
			memset(filename,0,BUF_SIZE);
			//memset(pwd,0,BUF_SIZE);
            str_len=read(clnt_sock, buf, BUF_SIZE);
            if(str_len == 0 || str_len == -1)
				error_handling("read() error");
            else
    		    buf[str_len]=0;
			printf("a");
			cmpstr = strstr(buf,"GET /");
			cmpstr += 5;
			int i = 0;
			printf("b");
			while(1){
				if(cmpstr[i]==' '){
					filename[i] = '\0';
					break;
				}
				filename[i] = cmpstr[i];
				i++;
				printf("%c",filename[i]);
			}
		}
		else
			close(clnt_sock);
    }
}

void read_childproc(int sig)
{
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG);
	printf("removed proc id: %d \n", pid);
}
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}