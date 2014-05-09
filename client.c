#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "settings.h"
#define BUF_SIZE 256

int s;

void halt(int sig) {
    printf("\nClient shutdown.\n");
    shutdown(s, 2);
    close(s);
    exit(0);
}

int main(int argc, char** argv) {
    int file_ds, readed_bytes;
    char buf[BUF_SIZE];
    char *command, *src_path, *host_addr, *parse_buf;
    struct sockaddr_in sname, cname;

    signal(SIGINT, halt);

    if(argc < 3) {
        write(2, "First arg must be host\n", sizeof("First arg must be host\n"));
        write(2, "Second arg must be command\n", sizeof("Second arg must be command\n"));
        exit(1);
    }

    command = malloc(sizeof(argv[2])+BUF_SIZE);
    strcpy(command, argv[2]);

    /*parse input string*/
    host_addr = malloc(sizeof(argv[1]));
    strcpy(host_addr, argv[1]);

    /*Create socket for host_addr*/
    int optval = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1) {
        perror("Cannot create socket.");
        exit(1);
    }

    /*Set server addr*/
    sname.sin_family = AF_INET;
    sname.sin_port = htons(SERV_PORT);
    if(inet_aton(host_addr, &sname.sin_addr) == 0) {
        perror("Cannot convert host_addr");
        exit(1);
    }

    /*set client addr*/
    cname.sin_family = AF_INET;
    cname.sin_port = htons(CLI_PORT);
    if(inet_aton(CLI_ADDR, &cname.sin_addr) == 0) {
        perror("Cannot convert client_addr");
        exit(1);
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    /*bind and connect*/
    if(bind(s, &cname, sizeof(cname)) == -1) {
        perror("Cannot bind socket");
        exit(1);
    }

    if(connect(s, &sname, sizeof(sname)) == -1) {
        perror("Cannot connect to remote host");
        exit(1);
    }

    printf("Connected to server to execute \"%s\" command.\n", command);
    send(s, command, BUF_SIZE, 0);
    char msg[BUF_SIZE] = {""};
    char prev_msg[BUF_SIZE] = {"prev"};
    int i;
    while (strstr(msg, prev_msg) == NULL) {
        recv(s, msg, sizeof(msg), 0);
        write(1, msg, sizeof(msg));
	strcpy(prev_msg, msg);
	for(i = 0; i < BUF_SIZE; i++)
	    msg[i] = '\0';
    }
    shutdown(s, 2);
    close(s);
    exit(0);
}
