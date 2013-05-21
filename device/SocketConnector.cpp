/*!****************************************************************************
 * @file SocketConnector.cpp Socket Connector implementation
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#include "SocketConnector.hpp"

int SocketConnector::init(int port){
    int fd;
    struct sockaddr_in addr;
    
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    fprintf(stderr, "Initialising on port %d...", port);
    /* create fd: family = IPv4, type = TCP, protocol = unused */
    if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1){
        perror("Unable to create a socket");
        close(fd);
        return -1;
    }

    /* bind */
    if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1){
        perror("Unable to bind file descriptor to port");
        close(fd);
        return -1;
    }

    /* listen: backlog = 1 */
    if(listen(fd, 1) == -1){
        perror("Unable to initiate port listening");
        close(fd);
        return -1;
    }
    else{
        fprintf(stderr, "Started listening on port %d\n", port);
    }
    act_fd = fd;
    return fd;
}


void SocketConnector::start(){
    pthread_create(&thread, NULL, act_func_caller, this);
}

void SocketConnector::join(){
    pthread_join(thread, NULL);
}

void *SocketConnector::act_func_caller(void *arg){
  return ((SocketConnector *)arg)->act_func();
}