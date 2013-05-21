/*!****************************************************************************
 * @file Device.hpp Device class Definitions
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#if !defined(DEVICE_HPP)
#define DEVICE_HPP
#include "IScheduler.hpp"
#include <pthread.h>
#include <dlfcn.h>
#include "GlobalDef.hpp"

class ControlLink;
class ComputeUnit;



class Device{
  
public:
  pthread_mutex_t data_mx;
  char *data;
  IScheduler *scheduler;
  ControlLink *controller;
  int port;
  
  int groupSize[3];

  Device(int port);
  ~Device();
  void start();
  void join();
  int memdump();
  
};

#endif //DEVICE_HPP