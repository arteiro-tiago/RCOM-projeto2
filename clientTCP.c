/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include "getip.h"
#include <string.h>

#define SERVER_PORT 21
#define BUF_SIZE 4096
char* ipGetter(char *argv) {
    struct hostent *h;
    if ((h = gethostbyname(argv)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    return inet_ntoa(*((struct in_addr *) h->h_addr));
}



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


int main(int argc, char **argv) {
    char const *serv = ipGetter(argv[1]);  
    int sockfd;
    int port;
    struct sockaddr_in server_addr;
    char buf[BUF_SIZE] = {0};
    /* Use CRLF as required by the FTP specification */
    char bufuser[] = "USER rcom\n";
    char bufpass[] = "PASS rcom\n";
    size_t bytes;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(serv);    /*32 bit Internet address network byte ordered*/
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

    while (1) {
        int n = read(sockfd, buf, BUF_SIZE - 1);
        if (n <= 0) {
            printf("erro de conexão!\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);

        if (strstr(buf, "220 ") != NULL) {
            break;
        }
    }
        sleep(0.1); /* 100 ms */


    //////USER

    bytes = write(sockfd, bufuser, strlen(bufuser));
    if (bytes > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }
    while (1) {
        int n = read(sockfd, buf, BUF_SIZE - 1);
        if (n <= 0) {
            printf("erro de conexão!\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);

        if (strstr(buf, "331") != NULL) {
            break;
        }
    }
    sleep(0.1);

    //////PASSWORD

    bytes = write(sockfd, bufpass, strlen(bufpass));
    if (bytes > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }
    while (1) {
        int n = read(sockfd, buf, BUF_SIZE - 1);
        if (n <= 0) {
            printf("erro de conexão!\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);

        if (strstr(buf, "230") != NULL) {
            break;
        }
    }
    sleep(0.1);

    //////PASV

    /* send PASV with CRLF and correct length */
    bytes = write(sockfd, "PASV\n", strlen("PASV\n"));
    if (bytes > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }
    while (1) {
        int n = read(sockfd, buf, BUF_SIZE - 1);
        if (n <= 0) {
            printf("erro de conexão!\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);

        if (strstr(buf, "227 ") != NULL) {
            port = extractPort(buf, BUF_SIZE);
            break;
        }
    }
    ////////////////////
    //////NEW TERM//////
    ////////////////////

    int sockfd2;
    struct sockaddr_in server_addr2;

    /*server address handling*/
    bzero((char *) &server_addr2, sizeof(server_addr2));
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_addr.s_addr = inet_addr(serv);    /*32 bit Internet address network byte ordered*/
    server_addr2.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(sockfd2,
                (struct sockaddr *) &server_addr2,
                sizeof(server_addr2)) < 0) {
        perror("connect()");
        exit(-1);
    }
    
    bytes = write(sockfd, "RETR debian/README.html\n", 25);

    if (bytes > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }
    sleep(0.1); /* 100 ms */
    // Ler resposta do servidor FTP (canal de controle)
    while (1) {
        int n = read(sockfd, buf, BUF_SIZE - 1);
        if (n <= 0) {
            printf("erro de conexão!\n");
            break;
        }
        buf[n] = '\0';
        printf("Resposta FTP: %s", buf);

        if (strstr(buf, "226") != NULL) {
            break;
        }
    }

    while (1) {
        int n = read(sockfd2, buf, BUF_SIZE - 1);
        if (n <= 0) {
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);
    }
    sleep(0.1); /* 100 ms */
     /* 100 ms */
    

    return 0;
}
