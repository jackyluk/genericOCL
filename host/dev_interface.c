/** implementation of dev_interface.h using sockets 
 * @author Marcin Bujar
 */

#include "debug.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "dev_interface.h"


/* device settings */
#define HOSTNAME "localhost"
#define PORT_CTRL "5000"
#define PORT_DATA "5001" 




int dev_connect(enum conn_type type)
{
    int fd;
    struct addrinfo hints, *ai, *ai0;
    int i;
    char *port;

    switch(type){
        case CONN_CTRL: port = PORT_CTRL; break;
        case CONN_DATA: port = PORT_DATA; break;
    }

    DEBUG("DEVICE: dev_connect(%s:%s)\n", HOSTNAME, port);

    /* create file descriptor using DNS lookup */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((i = getaddrinfo(HOSTNAME , port, &hints, &ai0)) != 0){
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



ssize_t dev_read(int fd, void* buffer, size_t len){
    ssize_t rcount = 0;
    size_t total = 0;

    while(total < len){
        rcount = read(fd, buffer, len);
        if(rcount < 0){
            perror("Unable to read data from device");
            return -1;
        }
        total += rcount;
    }
    DEBUG("DEVICE: dev_read(%p)\n", buffer);

    return total;
}



ssize_t dev_write(int fd, void* buffer, size_t len){
    ssize_t wcount = 0;
    size_t total = 0;


    DEBUG("DEVICE: dev_write(%p)\n", buffer);

    while(total < len){
        wcount = write(fd, buffer, len);
        if(wcount < 0){
            perror("Unable to writa data to device");
            return -1;
        }
        total += wcount;
    }

    return total;
}



int dev_set_kernel_args(char* arglist){
    FILE *file;
    int count;

    DEBUG("DEVICE: dev_set_kernel_args\n");
 
    file = fopen("kernelargs","w+");
    if(file == NULL)
        return -1;
    count = fprintf(file,"%s",arglist);
    fclose(file);
    if(count < 0)
        return -1;

    return 0;
}



int dev_disconnect(int fd){
    DEBUG("DEVICE: dev_disconnect\n");
    return close(fd);
}
