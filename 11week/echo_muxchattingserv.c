#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;

	struct timeval timeout;
	fd_set readfds, readtemp;
	int max_fd = 0;
	int result;
	int target;

	socklen_t adr_sz;
	int str_len;
	char message[BUF_SIZE];
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	FD_ZERO(&readfds);
	FD_SET(serv_sock, &readfds);
	FD_SET(0, &readfds);
	max_fd = serv_sock;

	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

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
		for(int i = 0; i <= max_fd+1; i++) {
			if(FD_ISSET(i, &readtemp)) {
				if(i == serv_sock) {
					memset(&message,0,sizeof(message));
					adr_sz=sizeof(clnt_adr);
					clnt_sock = accept(i, (struct sockaddr *)&clnt_adr, &adr_sz);
					if(clnt_sock == -1) {
						perror("accept");
						exit(0);
					}
					FD_SET(clnt_sock, &readfds);
					if(max_fd < clnt_sock)
						max_fd = clnt_sock;
					printf("Connected client: %d\n", clnt_sock);
				}
				else if(i == 0){
					memset(&message,0,sizeof(message));
					str_len = read(i, message, BUF_SIZE);
					if(!strncmp(message,"exit",4)){
						FD_ZERO(&readfds);
						close(serv_sock);
						return 0;
					}
					else if(str_len > 0){
						message[str_len] = 0;
						target = atoi(&message[0]);
						if(FD_ISSET(target, &readfds))
							write(target, message + 2, str_len);
						else
							printf("No Such Client\n");
					}
				}
				else {
					memset(&message,0,sizeof(message));
					str_len = read(i, message, BUF_SIZE);
					if(str_len == 0) {
						FD_CLR(i, &readfds);
						close(i);
						printf("Closed client: %d\n", i);
						break;
					}
					else{
						printf("Message from client %d %s", i, message);
					}
				}
			}
		}
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
