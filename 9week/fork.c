#include <unistd.h>
#include <stdio.h>

int g = 10;

int main()
{
    int l = 20;
    l += 5;
    g += 1;
    pid_t pid = fork();
    if(pid == 0){
        g += 1;
        printf("im child %d %d\n", g, l);
    }
    else{
        l += 1;
        printf("im parent %d %d\n", g, l);
    }
    return 0;
}
