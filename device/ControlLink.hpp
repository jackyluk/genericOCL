/*!****************************************************************************
 * @file ControlLink.hpp Control connection manager definitions
 * @author Jacky H T Luk 2013
 *****************************************************************************/

#if !defined(CONTROL_LINK_HPP)
#define CONTROL_LINK_HPP

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>

#include "SocketConnector.hpp"
#include "Device.hpp"
#include "ComputeUnit.hpp"


#define PACKED_STRUCT __attribute__((packed))
#define MORACL_PROTOCOL_VERSION 1

#define RESET                   0x00
#define MEM_WRITE_CMD           0x01
#define MEM_READ_CMD            0x02
#define MEM_READ_RSP_CMD        0x03
#define LOAD_KERNEL_IMAGE       0x04
#define START_KERNEL            0x05
#define GLOBAL_WORK_SIZE        0x06

#define CTRL_ACK                     0xFE
#define CTRL_NAK                     0xFF

typedef struct {
  uint32_t globalX;
  uint32_t globalY;
  uint32_t globalZ;
} PACKED_STRUCT GlobalWorkSize_t;

typedef struct {
  uint32_t totalSize;
  uint32_t offset;
  uint32_t dataSize;
  uint8_t data[0];
} PACKED_STRUCT LoadKernel_t;

typedef struct {
  uint32_t offset;
  uint32_t accessLength;
  uint8_t data[0];
} PACKED_STRUCT MemReadWrite_t;

typedef struct {
  uint8_t version;
  uint16_t length;
  uint8_t cmdId;
  union {
    MemReadWrite_t write;
    MemReadWrite_t read;
    LoadKernel_t loadkernel;
    GlobalWorkSize_t globalWorkSize;
  } payload;
} PACKED_STRUCT CommPacket_t;

enum{
  CTRL_STATE_IDLE,
  CTRL_STATE_RECV_KERNEL,
  CTRL_STATE_PROCESSING,
};

class ControlLink : public SocketConnector{
  char ctrl;
  int connfd;
  pthread_mutex_t ctrl_mx;

  pthread_cond_t ctrl_cond;
  pthread_mutex_t ctrl_condmx;

  int ctrlState;
  int kernelValid;
  int kernelLength;
  int recievedKernelLength;
  
  FILE *kernelfd;
  
  Device *parent;
  
private:
    int processPacket(char *packet, size_t buflen);
    int handleReset();
    int handleMemoryRead(MemReadWrite_t *read);
    int handleMemoryWrite(MemReadWrite_t *write);
    int handleKernelLoad(LoadKernel_t *loadkernel);
    int handleStartProcessing();
    int handleGlobalWorkSize(GlobalWorkSize_t *globalWS);
public:
    ControlLink(Device *parent);
    virtual ~ControlLink(){};
    
    virtual void* act_func();
    
    int sendAck();
    int sendErr();
    int sendKD();
};

#endif //CONTROL_LINK_HPP