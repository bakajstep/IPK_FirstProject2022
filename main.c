#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define MAX_HOSTNAME 256
#define MAX_CPUINFO 256
#define MAX_RESPONSE 1024

int cpuUsage(){

    
    return 15;
}


//vracim 1 pro uspech a -1 pro neuspech
int hostName(char* hostname){

    FILE *p;
    char ch;

    p = popen("hostname","r"); //precteni hostnamu
    if( p == NULL)
    {
        return -1;
    }
    int i = 0;
    while( (ch=fgetc(p)) != EOF){
        if(ch == '\n'){
            break;
        } else{
            hostname[i++] = ch;
        }
    }
    hostname[i] = '\0';
    return 1;
}


//vracim 1 pro uspech a -1 pro neuspech
int cpuName(char* cpuname){

    FILE *p;
    char ch;

    p = popen("lscpu | grep 'Model name' | cut -f 2 -d \":\" | awk '{$1=$1}1'","r"); //precteni model namu cpu
    if( p == NULL)
    {
        return -1;
    }
    int i = 0;
    while( (ch=fgetc(p)) != EOF){
        if(ch == '\n'){
            break;
        } else{
            cpuname[i++] = ch;
        }
    }
    cpuname[i] = '\0';
    return 1;
}


int main(int argc, char *argv[] ) {

    int port = atoi(argv[1]);
    int sockfd, connfd, se;
    unsigned int len;
    struct sockaddr_in servaddr, cli;
    char buff[MAX_RESPONSE];

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        exit(0);
    }
    bzero(&servaddr, sizeof(servaddr));

    // nastaveni ip adresy a portu
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&(int){1}, sizeof(int));
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&(int){1}, sizeof(int));

    // Nastaveni socketu
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        exit(0);
    }

    // naslouchani
    if ((listen(sockfd, 5)) != 0) {
        exit(0);
    }
    len = sizeof(cli);

    while (1){
        connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
        if (connfd < 0) {
            exit(0);
        }
        for(;;){
            int res = 0;
            res = recv(connfd, buff, sizeof(buff),0);
            if(res <= 0){
                break;
            }
            if (strstr(buff,"GET /cpu-name ")) {
                bzero(buff, MAX_RESPONSE);
                char cpuname[MAX_CPUINFO];
                cpuName(cpuname);
                sprintf(buff,"HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n%s",cpuname);
            } else if (strstr(buff,"GET /hostname ")) {
                bzero(buff, MAX_RESPONSE);
                char hostname[MAX_HOSTNAME];
                hostName(hostname);
                sprintf(buff,"HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n%s",hostname);
            } else if (strstr(buff,"GET /load ")) {
                sprintf(buff,"HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n%d%c",cpuUsage(),'%');
            } else {
                sprintf(buff,"HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain;\r\n\r\n%s","400 Bad request");
            }
            send(connfd, buff, strlen(buff), 0);
            close(connfd);
        }
    }
}
