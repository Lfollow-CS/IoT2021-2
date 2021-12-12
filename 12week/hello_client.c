#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char message1[30], message2[30], message3[30];
	int str_len;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
		
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
		error_handling("connect() error!");
	
	struct iovec vec[3];

	memset(&vec,0,sizeof(vec));
	vec[0].iov_base=message1;
	vec[0].iov_len=2;
	vec[1].iov_base=message2;
	vec[1].iov_len=2;
	// vec[2].iov_base=message3;
	// vec[2].iov_len=2;

	// str_len=readv(sock, vec, 2);
	str_len=read(sock, message1, sizeof(message1));
	if(str_len==-1)
		error_handling("read() error!");
	
	printf("Message from server: %s \n", message1);  
	printf("Message from server: %s \n", message2);  
	// printf("Message from server: %s \n", message3);  
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
