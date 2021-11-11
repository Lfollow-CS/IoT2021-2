#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char* argv[]){
    int sock;
	char message[BUF_SIZE];
	struct sockaddr_in serv_addr;

	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	sock=socket(PF_INET, SOCK_STREAM, 0);   
	if(sock==-1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
		error_handling("connect() error!");
    }
    
    write(sock, "GET /webhp HTTP/1.1\r\nUser-Agent: Mozilla/4.0\r\ncontent-type:text/html\r\nConnection: close\r\n\r\n", strlen("GET /webhp HTTP/1.1\r\nUser-Agent: Mozilla/4.0\r\ncontent-type:text/html\r\nConnection: close\r\n\r\n"));
    
    FILE *fp = fopen("test.html", "w");
	while(1){
		int str_len;
		str_len = read(sock, message, BUF_SIZE-1);
		if(str_len == 0){
			break;
		}
		message[str_len]=0;
    	printf("%s\n", message);
    	fprintf(fp, "%s", message);
	}
	fclose(fp);
    close(sock);

    return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}