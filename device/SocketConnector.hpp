/*!****************************************************************************
 * @file SocketConnector.hpp Socket Connector Definitions
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#if !defined(SOCKET_CONNECTOR_HPP)
#define SOCKET_CONNECTOR_HPP
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <pthread.h>
#define MAXBUF 64*1024
class SocketConnector{
  pthread_t thread;
protected:
  int act_fd;
public:
  int init(int port);
  virtual ~SocketConnector(){};
  virtual void* act_func() = 0;
  static void* act_func_caller(void *arg);
  void start();
  void join();
};

#endif //SOCKET_CONNECTOR_HPP