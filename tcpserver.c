#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define MAXLINE	256

void *client_request(void *arg);
void read_n_reply(int socket_fd);

const int backlog = 4;


int main(int argc, char *argv[])
{

    int	    listenfd, connfd;
    pid_t   childpid;
    pthread_t tid;
    int     clilen;
    struct  sockaddr_in cliaddr, servaddr;

    if (argc != 3) {
	printf("Usage: tcpserver <address> <port> \n");
	return EXIT_FAILURE;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
	perror("socket error");
	return EXIT_FAILURE;
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family        = AF_INET;
    servaddr.sin_addr.s_addr   = inet_addr(argv[1]);
    servaddr.sin_port          = htons(atoi(argv[2]));

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
	perror("bind error");
        return EXIT_FAILURE;

    }
	
    if (listen(listenfd, backlog) == -1) {
	perror("listen error");
	return EXIT_FAILURE;
    }

	

    while (1) {
	clilen = sizeof(cliaddr);
	if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0 ) {
		if (errno == EINTR)
			continue;
		else {
			perror("aceppt error");
			return EXIT_FAILURE;
		}
	}
	pthread_create(&tid,NULL,client_request,(void*)&connfd);
    }

}

void *client_request(void *arg)
{
	int connfd;
	connfd = *(int*)arg;
	FILE *fp;
	int	n;
	int i = 0;
	int t = 0;
	char	buf[256];
	char command[4];
	char data[500];
	char version[9];
	char *file;
	long fsize;
	struct stat mod;
	time_t modtime;
	char ltime[36];

	while (1) {
	    i = 0;
	    t = 0;
	    if ((n = read(connfd, buf, MAXLINE)) == 0)
		return;
	
	    //printf("%s\n", buf);	
	    while(buf[i] != ' ')
	    {
		command[t] = buf[i];
		i++;
		t++;
	    }
	    command[t] = '\0';
	    i++;    
	    t = 0;
	    if(strcmp(command,"GET") == 0 || strcmp(command,"HEAD") == 0 || strcmp(command,"POST") == 0);
	    else
	    {
		write(connfd, "HTTP/1.0 400 Bad Request\n", 26);
		continue;
	    }

	    if(buf[i] == '/')
	    {
		i++;
	    }
	    while(buf[i] != ' ')
	    {
		data[t] = buf[i];
		i++;
		t++;
	    }
	    data[t] = '\0';
	    //printf("%s\n", data);
	    i++;
	    t = 0;

	    if(strcmp(command, "GET") == 0)
	    {
	    	if(fp = fopen(data, "r"));
	    	else
	    	{
		    write(connfd, "HTTP/1.0 404 Not Found\n", 24);
		    //fclose(fp);
		    continue;
		}
	    }
	    else if(strcmp(command, "POST") == 0)
	    {
		fp = fopen(data, "w");
	    }

	    while(t < 8)//strlen(buf) - 2)
	    {
		//printf("%d", i);
		version[t] = buf[i];
		i++;
		t++;
	    }
	    version[t] = '\0';

	    if(strcmp(version,"HTTP/1.0") != 0)
	    {
		write(connfd, "HTTP/1.0 505 HTTP Version Not Supported\n", 41);
		continue;
	    }

	   if(strcmp(command,"GET") == 0)
	   {
		write(connfd, "HTTP/1.0 200 OK\n", 17);
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		file = malloc(fsize + 1);
		fread(file, fsize, 1, fp);
		file[fsize] = 0;
                write(connfd, file, fsize);
		fclose(fp);
	   }

	   char space[2];
	   t = 0;
	   if(strcmp(command, "POST") == 0)
	   {
		while(strlen(buf) > 2)
		{
		    memset(buf, 0, sizeof(buf));
		    read(connfd, buf, MAXLINE);
		    //printf("%d\n", strlen(buf));
		}
		t = 0;
		i = 0;

		while(strlen(buf) <= 2)
		{
		   read(connfd, buf, MAXLINE);
		}
		while(i < strlen(buf))
		{
		   data[t] = buf[i];
		   t++;
		   i++;
		}
		fwrite(data, t, 1, fp);
		write(connfd, "HTTP/1.0 200 OK\n", 17);
		fclose(fp);
	   }

	   if(strcmp(command, "HEAD") == 0)
	   {
		write(connfd, "HTTP/1.0 200 OK\n", 17);
		stat(data, &mod);
		strftime(ltime, 36, "%d.%m.%Y %H:%M:%S", localtime(&mod.st_mtime));
		write(connfd, ltime, strlen(ltime));
		write(connfd, "\n", 1);
		//modtime = mod.st_mtime;
		//write(connfd, asctime(gmtime(&modtime)), 26);
	   }
	}

}
