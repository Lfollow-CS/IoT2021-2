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

char imgheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: image/jpg\r\n\r\n";
char pngheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: image/png\r\n\r\n";
char htmlheader[] = "HTTP/1.1 200 Ok\r\nContent-Type: text/html\r\n\r\n";
char notfound[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n";

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
			char pwd[100];
			char* cmpstr;
			memset(filename,0,BUF_SIZE);
            str_len=read(clnt_sock, buf, BUF_SIZE);
            if(str_len == 0 || str_len == -1)
				error_handling("read() error");
            else
    		    buf[str_len]=0;
			cmpstr = strstr(buf,"GET /");
			cmpstr += 5;
			int i = 0;
			while(1){
				if(cmpstr[i]==' '){
					filename[i] = '\0';
					break;
				}
				filename[i] = cmpstr[i];
				i++;
			}
			getcwd(pwd,BUF_SIZE);
			strcat(pwd,"/");
			strcat(pwd,filename);
			printf("%s\n",pwd);
			char fbuf[BUF_SIZE];
			int n;
			if((fd=open(pwd,O_RDONLY))==-1){
				char notfound_content[100];
				notfound_content = "<h1>404 Not Found</h1>";
				write(clnt_sock, notfound, sizeof(notfound)-1);
				write(clnt_sock, notfound_content, sizeof(notfound_content)-1);
				printf("Not Found\n");
			}
			else{
				if(strstr(filename,".jpg")!=NULL){
					write(clnt_sock, imgheader, sizeof(imgheader)-1);
					printf("Found jpg\n");
				}
				else if(strstr(filename,".png")!=NULL){
					write(clnt_sock, pngheader, sizeof(pngheader)-1);
					printf("Found png\n");
				}
				else if(strstr(filename,".html")!=NULL){
					write(clnt_sock, htmlheader, sizeof(htmlheader)-1);
					printf("Found html\n");
				}
				while((n= read(fd,fbuf,BUF_SIZE))>0){
					write(clnt_sock,fbuf,n);
				}
			}
			close(clnt_sock);
			return 0;
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