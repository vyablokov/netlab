#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "settings.h"
#define BUF_SIZE 256

int s;

void halt(int sig) {
    printf("Server shutdown.\n");
    shutdown(s, 2);
    close(s);
    exit(0);
}

int main(int argc, char** argv) {
    int s_new, from_len, child_pid, file_ds;
    char buf[BUF_SIZE];
    struct sockaddr_in sname, cname;
    struct stat file_stat;

    signal(SIGINT, halt);

    printf("Starting server...\n");

    int optval = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    s = socket(AF_INET, SOCK_STREAM, 0);
    sname.sin_family = AF_INET;
    sname.sin_addr.s_addr = INADDR_ANY;
    sname.sin_port = htons(SERV_PORT);

    if(inet_aton(SERV_ADDR, &sname.sin_addr) == 0) {
        perror("Cannot convert server address");
        exit(1);
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    if(bind(s, &sname, sizeof(sname)) == -1) {
        perror("Cannot bind socket");
        exit(1);
    }

    listen(s, 5);
    printf("Server is running.\n");

    for(;;) {
        from_len = sizeof(cname);
        setsockopt(s_new, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
		s_new = accept(s, &cname, &from_len);
		recv(s_new, buf, BUF_SIZE, 0);
		printf("Executing command \"%s\".\n", buf);
	        
		child_pid = fork();
		if(child_pid == 0) {
		    dup2(s_new, fileno(stdout));
		    dup2(s_new, fileno(stderr));
		    system(buf);
		    close(stdout);
		    close(stderr);
		    shutdown(s_new, 2);
	        close(s_new);
		    exit(0);
		}
    }
}
