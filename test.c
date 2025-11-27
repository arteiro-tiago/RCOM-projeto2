#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
int extractPort(char *buf, int bufsize){
    int nums[6];
    int count = 0;

    char *token = strtok(buf, ",");
    while (token && count < 6) {
        nums[count++] = atoi(token);
        token = strtok(NULL, ",");
    }

    int p1 = nums[4];
    int p2 = nums[5];

    return p1 * 256 + p2;
}

int main(){
    char bufuser[] = "233 dawdawndkjwad (193,137,29,15,211,225)";
    int i = extractPort(bufuser, 500);
    printf("%d", i);
    return 0;
}

