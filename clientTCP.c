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
#include <netdb.h>

#define SERVER_PORT 21
#define BUF_SIZE 4096

char* extract_filename(char* path) {    
    const char* last = strrchr(path, '/');
    
    if (last) {
        return strdup(last + 1);
    } else {
        return strdup(path);
    }
}

char** link_parser(char* link){
    char** linkInfo = (char**) malloc(4 * sizeof(char*));
    int arroba = '@';
    int barra = '/';
    int pontos = ':';
    int end = '\0';
    char* arrobaptr = strchr(link, arroba);
    char* next = strchr(link, barra) + 2;

    if (arrobaptr!=NULL){
        char* userptr = next;
        char* passwordptr = strchr(userptr, pontos) + 1;
        
        int user_size = passwordptr-userptr;
        char* user = (char*)malloc(user_size);
        memcpy(user, userptr, user_size-1);

        int password_size = arrobaptr-passwordptr;
        char* password = (char*)malloc(password_size);
        memcpy(password, passwordptr, password_size);

        next = arrobaptr + 1;

        linkInfo[0]=user;
        linkInfo[1]=password;
    }

    char* hostptr = next;
    char* pathptr = strchr(next, barra);

    int host_size = pathptr - hostptr;
    char* host = (char*)malloc(host_size); 
    memcpy(host, hostptr, host_size);

    char* endptr = strchr(pathptr, end);

    int path_size = endptr-pathptr;
    char* path = (char*)malloc(path_size);
    memcpy(path, pathptr+1, path_size-1); 

    linkInfo[2]=host;
    linkInfo[3]=path;

    return linkInfo;
}

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
    if (argc < 2) {
        exit(-1);
    }
    
    char** linkInfo = link_parser(argv[1]);
    if (!linkInfo) {
        exit(-1);
    }
    
    printf("USER: %s\n", linkInfo[0] ? linkInfo[0] : "(anonymous)");
    printf("PASS: %s\n", linkInfo[1] ? linkInfo[1] : "(anonymous)");
    printf("HOST: %s\n", linkInfo[2] ? linkInfo[2] : "(error)");
    printf("PATH: %s\n", linkInfo[3] ? linkInfo[3] : "(error)");
    
    if (linkInfo[2]==NULL) {
        exit(-1);
    }
    
    char* serv = ipGetter(linkInfo[2]);
    printf("\nIP: %s\n", serv ? serv : "error");
    
    int sockfd;
    int port;
    struct sockaddr_in server_addr;
    char buf[BUF_SIZE] = {0};
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

    printf("Conexão estabelecida!\n");

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
    sleep(0.1);


    //////USER

    char user_cmd[BUF_SIZE] = {0};
    if (linkInfo[0] != NULL) {
        snprintf(user_cmd, sizeof(user_cmd), "USER %s\r\n", linkInfo[0]);
    } else {
        strcpy(user_cmd, "USER anonymous\r\n");
    }
    
    bytes = write(sockfd, user_cmd, strlen(user_cmd));
    if (bytes > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }
    
    while (1) {
        int n = read(sockfd, buf, BUF_SIZE - 1);
        if (n <= 0) {
            printf("erro de conexão!\r\n");
            break;
        }
        buf[n] = '\0';
        printf("%s", buf);

        if (strstr(buf, "331") != NULL) {
            break;
        }
    }
    sleep(0.1);


    //////PASS

    char pass_cmd[BUF_SIZE] = {0};
    if (linkInfo[1] != NULL) {
        snprintf(pass_cmd, sizeof(pass_cmd), "PASS %s\r\n", linkInfo[1]);
    } else {
        strcpy(pass_cmd, "PASS anonymous\r\n");
    }
    
    bytes = write(sockfd, pass_cmd, strlen(pass_cmd));
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

    bytes = write(sockfd, "PASV\r\n", strlen("PASV\r\n"));
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

    
    //////RETR
    if (!linkInfo[3]) {
        exit(-1);
    }
    
    char retr_cmd[BUF_SIZE];
    snprintf(retr_cmd, sizeof(retr_cmd), "RETR %s\r\n", linkInfo[3]);
    
    bytes = write(sockfd, retr_cmd, strlen(retr_cmd));
    if (bytes > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }
    
    sleep(0.1);
    
    while (1) {
        int n = read(sockfd, buf, BUF_SIZE - 1);
        if (n <= 0) {
            printf("erro de conexão!\n");
            break;
        }
        buf[n] = '\0';
        printf("Resposta FTP: %s", buf);

        if (strstr(buf, "150") != NULL || strstr(buf, "125") != NULL) {
            break;
        }
    }

    FILE* output = fopen(extract_filename(linkInfo[3]), "wb");
    
    int total_bytes = 0;
    while ((bytes = read(sockfd2, buf, BUF_SIZE)) > 0) {
        total_bytes += bytes;
        if (output) {
            fwrite(buf, 1, bytes, output);
        }
        printf("\rRecebidos: %d bytes", total_bytes);
        fflush(stdout);
    }
    
    fclose(output);
    
    printf("\nFIM DOS DADOS (total: %d)\n", total_bytes);
    
    bytes = read(sockfd, buf, BUF_SIZE - 1);
    if (bytes > 0) {
        buf[bytes] = '\0';
        printf("Resposta: %s", buf);
    }
    
    sleep(0.1);

    //////QUIT

    bytes = write(sockfd, "QUIT\r\n", strlen("QUIT\r\n"));
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

        if (strstr(buf, "221 ") != NULL) {
            port = extractPort(buf, BUF_SIZE);
            //CLOSES
            close(sockfd2);
            close(sockfd);
            for (int i = 0; i < 4; i++) {
                if (linkInfo[i]) free(linkInfo[i]);
            }
            free(linkInfo);
            return 0;
        }
    }
    //CLOSES
    close(sockfd2);
    close(sockfd);
    for (int i = 0; i < 4; i++) {
        if (linkInfo[i]) free(linkInfo[i]);
    }
    free(linkInfo);
    return -1;
}