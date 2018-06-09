#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 80

void str_client(FILE *fp, int socket_fd);

int main(int argc, char *argv[])
{

    int	socket_fd;
    struct  sockaddr_in servaddr;

    int   ntimes;
    char  ptime[MAXLINE];
    char  buf[1];

    if (argc != 4) {
	printf("Usage: tcpclient <address> <port> <ntimes> \n");
	exit(-1);
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
	perror("socket error");
	exit(-1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    ntimes = atoi(argv[3]);

    if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
	perror("connect error");
	exit(-1);
    }

    while (ntimes--) {
        write(socket_fd, buf, sizeof(buf));            

	memset(ptime, 0, MAXLINE);
        if (read(socket_fd, ptime, MAXLINE) == 0) {
            printf("ERROR: server terminated \n");
            break;
        }

        fputs(ptime, stdout);

        sleep(1);
    }

    close(socket_fd);

    return EXIT_SUCCESS; 

}
