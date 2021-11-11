#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netdb.h>

int main(int argc, char* argv[]){
    struct hostent *host;

    host = gethostbyname(argv[1]);
    if(host ==NULL){
        fprintf(stderr, "Cannot find IP address from %s\n",argv[1]);
        return 0;
    }
    printf("Official name: %s\n", host->h_name);
    
    for(int i=0;host->h_aliases[i];i++){
        printf("Alias %d: %s\n", i+1, host->h_aliases[i]);
    }

    printf("Address type: %s\n", (host->h_addrtype == PF_INET) ? "PF_INET" : "Unknown");

    for(int i=0;host->h_addr_list[i];i++){
        printf("IP addr %d: %s\n", i+1, inet_ntoa(*(struct in_addr *)host->h_addr_list[i]));
    }

    return 0;
}