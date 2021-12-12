#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock;
	char message[BUF_SIZE];
	int str_len;
	struct sockaddr_in serv_adr;

	struct timeval timeout;
	fd_set readfds, readtemp;
	int max_fd = 0;
	int result;

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);   
	if(sock==-1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);
	FD_SET(0, &readfds);
	max_fd = sock;

	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected...........");
	
	while(1) 
	{
		readtemp = readfds;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		result = select(max_fd+1, &readtemp, NULL, NULL, &timeout);
		if(result == -1) {
			perror("select");
			break;
		}
		else if(result == 0) {
			continue;
		}
		for(int i = 0; i <= max_fd; i++) {
			if(FD_ISSET(i, &readtemp)) {
				if(i == sock) {
					memset(&message,0,sizeof(message));
					str_len = read(i, message, BUF_SIZE);
					if(str_len == 0 || !strncmp(message,"exit",4)) {
						FD_CLR(i, &readfds);
						close(i);
						printf("End Connection\n");;
						return 0;
					}
					else{
						printf("Message from Server: %s", message);
					}
				}
				else if(i == 0){
					// memset(&message,0,sizeof(message));
					str_len = read(i, message, BUF_SIZE);
					if(!strncmp(message,"exit",4)){
						printf("End Connection\n");
						close(sock);
						return 0;
					}
					else if(str_len > 0){
						// message[str_len] = 0;
						write(sock, message, str_len);
					}
				}
			}
		}
	}
	
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}