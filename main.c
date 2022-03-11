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

unsigned long long getTotalCpuTime( unsigned long long* idl) {
    FILE* file = fopen("/proc/stat", "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open stat file!\n");
        return 0;
    }

    char buffer[1024];
    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
        unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

    char* ret = fgets(buffer, sizeof(buffer) - 1, file);
    if (ret == NULL) {
        fprintf(stderr, "Could not open stat file!\n");
        fclose(file);
        return -1;
    }
    fclose(file);

    sscanf(buffer,
           "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);


    *idl = idle + iowait;
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

int cpuUsage(){
    unsigned long long totald, total, prevTotal ;
    unsigned long long idle, prevIdle, idled;
    double load;

    //prvni cteni
    prevTotal = getTotalCpuTime(&prevIdle);
    if(prevTotal == 0){
        return -1;
    }
    sleep(1);

    //druhy cteni
    total = getTotalCpuTime(&idle);
    if(total == 0){
        return -1;
    }
    totald = total - prevTotal;
    idled = idle - prevIdle;

    load = ((double)(totald-idled)/(double)totald)*100;

    return (int)load;
}

int getTerminalOutput(char* command, char* buff){

    FILE *p;
    char ch;

    p = popen(command,"r"); //ziskani streamu s odpovedi
    if( p == NULL)
    {
        return -1;
    }
    int i = 0;
    while( (ch=fgetc(p)) != EOF){
        if(ch == '\n'){
            break;
        } else{
            buff[i++] = ch;
        }
    }
    buff[i] = '\0';
    return 1;
}

int main(int argc, char *argv[] ) {

    //kontrola poctu paramatru
    if (argc != 2){
        fprintf(stderr, "Wrong number of parameters!\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    //kontrola rozsahu portu
    if(port < 1024 || port > 65535){
        fprintf(stderr, "Wrong port!\n");
        exit(EXIT_FAILURE);
    }



    int sockfd, connfd;
    unsigned int len;
    struct sockaddr_in servaddr, cli;
    char buff[MAX_RESPONSE];

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        fprintf(stderr, "Socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    bzero(&servaddr, sizeof(servaddr));

    // nastaveni ip adresy a portu
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT | SO_REUSEADDR,&(int){1}, sizeof(int));

    // Nastaveni socketu
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        fprintf(stderr, "Socket bind failed!\n");
        exit(EXIT_FAILURE);
    }

    // naslouchani
    if ((listen(sockfd, 5)) != 0) {
        fprintf(stderr, "Listen failed!\n");
        exit(EXIT_FAILURE);
    }
    len = sizeof(cli);

    while (1){
        connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
        if (connfd < 0) {
            fprintf(stderr, "Server accept failed!\n");
            exit(EXIT_FAILURE);
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
                char* command = "lscpu | grep 'Model name' | cut -f 2 -d \":\" | awk '{$1=$1}1'";
                if(getTerminalOutput(command,cpuname) == -1){
                    fprintf(stderr, "Eror in cpu-name command!\n");
                    close(connfd);
                    exit(EXIT_FAILURE);
                }
                sprintf(buff,"HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n%s",cpuname);
            } else if (strstr(buff,"GET /hostname ")) {
                bzero(buff, MAX_RESPONSE);
                char hostname[MAX_HOSTNAME];
                char* command = "hostname";
                if(getTerminalOutput(command,hostname) == -1){
                    fprintf(stderr, "Eror in hostname command!\n");
                    close(connfd);
                    exit(EXIT_FAILURE);
                }
                sprintf(buff,"HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n%s",hostname);
            } else if (strstr(buff,"GET /load ")) {
                int load = cpuUsage();
                if(load == -1){
                    fprintf(stderr, "Eror in hostname command!\n");
                    close(connfd);
                    exit(EXIT_FAILURE);
                }
                sprintf(buff,"HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n%d%c",load,'%');
            } else {
                sprintf(buff,"HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain;\r\n\r\n%s","400 Bad request");
            }
            send(connfd, buff, strlen(buff), 0);
            close(connfd);
        }
    }
}
