/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#define SERVER_PORT 21
#define SERVER_ADDR "193.137.29.15"
#define BUF_SIZE 4096

int main(int argc, char **argv) {

    if (argc > 1)
        printf("**** No arguments needed. They will be ignored. Carrying ON.\n");
    int sockfd;
    struct sockaddr_in server_addr;
    char buf[BUF_SIZE] = {0};
    char bufuser[] = "USER anonymous";
    char bufpass[] = "PASS anonymous";
    size_t bytes;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(SERVER_PORT);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    int rbytes = read(sockfd, buf, BUF_SIZE);
    for (int i = 0; i < rbytes; i++){
        printf("%c",buf[i]);
    }
    /*send a string to the server*/

    bytes = write(sockfd, bufuser, strlen(bufuser));
    if (bytes > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }


    if (close(sockfd)<0) {
        perror("close()");
        exit(-1);
    }
    return 0;
}


