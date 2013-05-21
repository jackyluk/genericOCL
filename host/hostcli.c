#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define HOST "127.0.0.1"
#define MAXLINE 128
#define MAXBUF 1024

/* WARNING: This interface is deprecated. Use dev_inteface.h instead */


int dev_connect(const char* port)
{
    int fd;
    struct addrinfo hints, *ai, *ai0;
    int i;

    /* create file descriptor using DNS lookup */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((i = getaddrinfo(HOST , port, &hints, &ai0)) != 0){
        fprintf(stderr, "Unable to look up IP address: %s\n", gai_strerror(i));
        return -1;
    }

    for(ai = ai0; ai != NULL; ai = ai->ai_next){
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if(fd == -1){
            continue;
        }

      if(connect(fd, ai->ai_addr, ai->ai_addrlen) == -1){
          close(fd);
          continue;
      }
      /* else success! connection found */
      break;
    }

    if(ai == NULL){
        perror("Unable to connect to device");
        return -1;
    }
    
    return fd;
}



int readline(char s[], int lim){
	int c, i;

	for (i=0; i < lim-1 && (c = getchar()) != EOF && c != '\n'; ++i)
		s[i] = c;
	s[i] = '\0';
	return i;
}



int handle_ctrl(int fd){
    char line[MAXLINE];
    int linelen = 0;
    int wcount, rcount;
    char buf[MAXBUF];
    int buflen = 0;

    while((linelen = readline(line, MAXLINE))){
        if(strcmp(line, "q") == 0) break;
        /* send data from device */
        while((wcount = send(fd, line, linelen, 0)) != linelen){
            if(wcount < 0){
                perror("Unable to send data");
                close(fd);
                return -1;
            }
        }
        printf("Sending data complete.\n");
        /* read acknowledgement */
        while(buflen < 4){
            rcount = recv(fd, buf, MAXBUF, 0);
            if(rcount < 0){
                perror("Unable to read data from device");
                close(fd);
                return -1;
            }
            buflen += rcount;
        }
        buf[buflen>MAXBUF ? MAXBUF : buflen] = '\0';
        printf("recv %d bytes: %s\n", buflen, buf);
    }
    close(fd);
    
    return 0;
}



int handle_data(int fd){
    char line[MAXLINE];
    int linelen = 0;
    int wcount, rcount;
    char buf[MAXBUF];
    int buflen = 0;
    int readlen = 0;

    while((linelen = readline(line, MAXLINE))){
        if(strcmp(line, "q") == 0) break;
        /* send data from device */
        while((wcount = send(fd, line, linelen, 0)) != linelen){
            if(wcount < 0){
                perror("Unable to send data");
                close(fd);
                return -1;
            }
        }
        printf("Sending data complete.\n");
        /* read answer */
        if(line[0] == 'w') readlen = 4; /* just an ack */
        else if(line[0] == 'r') readlen = 10; /* temporary length */
        while(buflen < readlen){
            rcount = recv(fd, buf, MAXBUF, 0);
            if(rcount < 0){
                perror("Unable to read data from device");
                close(fd);
                return -1;
            }
            buflen += rcount;
        }
        buf[buflen>MAXBUF ? MAXBUF : buflen] = '\0';
        printf("recv %d bytes: %s\n", buflen, buf);
        memset(buf, 0, MAXBUF); /* clear the buffer */
        buflen = 0;
    }
    close(fd);
    
    return 0;
}



int main(int argc, char** argv)
{
    int fd;

    
    if(argc != 2){
        printf("usage: %s (-c|-d)\n", argv[0]);
        return -1;
    }

    if(strcmp(argv[1], "-c") == 0){
        if((fd = dev_connect("5000")) == -1)
            return -1;
        printf("Control connection:\n");
        return handle_ctrl(fd);
    }
    else if(strcmp(argv[1], "-d") == 0){
    if((fd = dev_connect("5001")) == -1)
        return -1;
    printf("Data connection:\n");
        return handle_data(fd);
    }
    else{
        printf("Incorrect argument. Use either -c for control or -d for data connection.\n");
        return -1;
    }
}
