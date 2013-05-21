/*!****************************************************************************
 * @file ControlLink.cpp Control connection manager
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#include "GlobalDef.hpp"
#include "ControlLink.hpp"
#include "IScheduler.hpp"
#include "debug.h"


/*!****************************************************************************
 * @brief handleKernelLoad Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
ControlLink::ControlLink(Device *parent){
  pthread_mutex_init(&ctrl_mx, NULL);
  pthread_cond_init(&ctrl_cond, NULL);
  pthread_mutex_init(&ctrl_condmx, NULL);
  
  ctrlState = CTRL_STATE_IDLE;
  kernelValid = 0;
  this->parent = parent;
  pthread_mutex_init(&(parent->data_mx), NULL);
  
}

/*!****************************************************************************
 * @brief handleKernelLoad Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
void* ControlLink::act_func(){
    int fd;
    struct sockaddr_in ctrl_addr;
    socklen_t ctrl_addr_len = sizeof(ctrl_addr);
    char buf[MAXBUF];
    ssize_t rcount, wcount, acklen;
    size_t buflen = 0;
    int consumed = 0;

    fprintf(stderr, "[CTRL] Thread started\n");
    /* accept new control connection */
    fd = this->act_fd;
    if((connfd = accept(fd, (struct sockaddr *) &ctrl_addr, &ctrl_addr_len)) == -1){
        perror("[CTRL] Unable to connect with interface");
        close(fd);
        pthread_exit(NULL);
    }

    perror("[CTRL] New connection"); 
    buflen = 0;
    while((rcount = recv(connfd, buf+buflen, MAXBUF - buflen, 0)) != 0){
        fprintf(stderr, "[CTRL] rcount %d\n", rcount);       
        if(rcount < 0){
            perror("[CTRL] Unable to read from host");
            close(fd);
            pthread_exit(NULL);
        }
        buflen += rcount;
        fprintf(stderr, "[CTRL] recv %zd bytes\n", rcount);
        if((consumed = processPacket(buf, buflen))){
            fprintf(stderr, "[CTRL] Processed 1 packet, consumed %d\n", consumed);
            if(buflen - consumed){
                printf("memmov from %d to %d size %d\n", consumed, 0, buflen - consumed);
                memcpy(buf, buf+consumed, buflen - consumed);
            }
            buflen -= consumed;

        }
        fprintf(stderr, "[CTRL] buflen %d\n", buflen);
        printf("MAXBUF - buflen = %d\n", MAXBUF - buflen);
    }
    close(connfd);
    close(fd);
    fprintf(stderr, "[CTRL] Thread stopped\n");
    
    return NULL;
}

/*!****************************************************************************
 * @brief processPacket Process a packet if enough bytes are in the buffer
 * @param loadkernel pointer to Load kernel command payload
 * @return Bytes consumed. 
 * ***************************************************************************/
int ControlLink::processPacket(char *packet, size_t buflen){
    CommPacket_t *cmdPkt = (CommPacket_t *)packet;
    fprintf(stderr, "[CTRL] processpacket len %zd\n", buflen);
    fprintf(stderr, "[CTRL] offsetof cmdID len %zd\n", offsetof(CommPacket_t, cmdId));
    
    if(buflen >= offsetof(CommPacket_t, cmdId)){
        fprintf(stderr, "[CTRL] cmd len %zd\n", ntohs(cmdPkt->length));
        if(ntohs(cmdPkt->length) > buflen) return 0;
          
        /*! We have enough data, process the packet*/
        switch(cmdPkt->cmdId){
            case RESET:
                handleReset();
                break;
            case MEM_WRITE_CMD:
                handleMemoryWrite(&(cmdPkt->payload.write));
                break;
            case MEM_READ_CMD:
                handleMemoryRead(&(cmdPkt->payload.read));
                break;
            case LOAD_KERNEL_IMAGE:
                handleKernelLoad(&(cmdPkt->payload.loadkernel));
                break;
            case START_KERNEL:
                handleStartProcessing();
                break;
            case GLOBAL_WORK_SIZE:
                handleGlobalWorkSize(&(cmdPkt->payload.globalWorkSize));
                break;
            default:
                fprintf(stderr, "[CTRL] Unrecognised command 0x%02X\n", cmdPkt->cmdId);
                sendErr();
                break;
            
        }
        return ntohs(cmdPkt->length);
        
    }
    return 0;
}

/*!****************************************************************************
 * @brief handleReset Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
int ControlLink::handleReset(){
    kernelValid = false;
    return 0;  
}

/*!****************************************************************************
 * @brief handleKernelLoad Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
int ControlLink::handleMemoryRead(MemReadWrite_t *read){
    int offset = ntohl(read->offset);
    int accessLength = ntohl(read->accessLength);
    
    ssize_t wcount, sentLen;
    uint8_t replyBuf[64*1024];
    int rspLength;
    CommPacket_t *readRsp = (CommPacket_t *)replyBuf;

    readRsp->version = MORACL_PROTOCOL_VERSION;
    readRsp->cmdId = MEM_READ_RSP_CMD;        
    readRsp->payload.read.offset = htonl(offset);
    readRsp->payload.read.accessLength = htonl(accessLength);
                    
    /* read from device memory */
    pthread_mutex_lock(&(parent->data_mx));
    memcpy(readRsp->payload.read.data, parent->data+offset, accessLength);
    pthread_mutex_unlock(&(parent->data_mx));
                    
    rspLength = 4 + 8 + accessLength;
    readRsp->length = htonl(rspLength);
          
    sentLen = 0;
    while(sentLen < rspLength){
        wcount = send(connfd, readRsp + sentLen, rspLength - sentLen, 0);
        if(wcount < 0){
            perror("[CTRL] Unable to send data read response.");
            close(connfd);
            pthread_exit(NULL);
        }
        sentLen += wcount;
    }
    fprintf(stderr, "[DATA] Sending requested data complete.\n");
    return 0;  
}

/*!****************************************************************************
 * @brief handleMemoryWrite Write to device memory
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
int ControlLink::handleMemoryWrite(MemReadWrite_t *write){
    int offset = ntohl(write->offset);
    int accessLength = ntohl(write->accessLength);
    
    fprintf(stderr, "[CTRL] handleMemoryWrite. Off %x, access %x\n", offset, accessLength);

    pthread_mutex_lock(&(parent->data_mx));
    memcpy(parent->data+offset, write->data, accessLength);
    pthread_mutex_unlock(&(parent->data_mx));
    fprintf(stderr, "[CTRL] handleMemoryWrite.\n");
    
    sendAck();
    fprintf(stderr, "[DATA] Sending ack complete.\n");
    fprintf(stderr, "offset: %d\n", offset);
    fprintf(stderr, "memsize: %d, recv end: %d\n", GLOBAL_MEMORY_SIZE, offset + accessLength);
    return 0;  
}

/*!****************************************************************************
 * @brief handleKernelLoad Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
int ControlLink::handleKernelLoad(LoadKernel_t *loadkernel){
    
    int dataSize = ntohl(loadkernel->dataSize);
    int kernelSize = ntohl(loadkernel->totalSize);
    int offset = ntohl(loadkernel->offset);
    
    DEBUG("%s: Load Kernel %d bytes at %d of %d.\n", __func__, 
                                                    dataSize, 
                                                    offset, 
                                                    kernelSize);
    
    //Any Kernel loading will invalidate current kernel
    kernelValid = false;
    
    if(offset == 0){
        //Start of file
        if(kernelfd != NULL){
            fclose(kernelfd);
            kernelfd = NULL;
        }
        kernelfd = fopen("kernel.so", "wb+");
    }
    
    if(kernelfd == NULL){
        fprintf(stderr, "[CTRL] Cannot open kernel file.\n");
        sendErr();
    }
    
    if(dataSize != fwrite(loadkernel->data, 1, dataSize, kernelfd)){
        fprintf(stderr, "[CTRL] Kernel write error.\n");
        fclose(kernelfd);
        kernelfd = NULL;
        sendErr();
    }
    
    if(offset + dataSize > kernelSize){
        fprintf(stderr, "[CTRL] Received too many kernel bytes.\n");
        fclose(kernelfd);
        kernelfd = NULL;
        sendErr();
    }
    
    if(offset + dataSize == kernelSize){
        fprintf(stderr, "[CTRL] Full kernel received.\n");
        fclose(kernelfd);
        kernelfd = NULL;
        kernelValid = true;
    }
    
    sendAck();
    
    return 0;  
}

/*!****************************************************************************
 * @brief handleKernelLoad Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
int ControlLink::handleStartProcessing(){
    
     if(kernelValid){
        DEBUG("%s: Run kernel!\n", __func__);
        parent->scheduler->addWork(parent->groupSize);
        if(sendAck() < 0){
            perror("[CTRL] Unable to ack");
            close(connfd);
            pthread_exit(NULL);
        }
    }
    
    return 0;  
}

/*!****************************************************************************
 * @brief handleKernelLoad Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
int ControlLink::handleGlobalWorkSize(GlobalWorkSize_t *globalWS){
    int globalX = ntohl(globalWS->globalX);
    int globalY = ntohl(globalWS->globalY);
    int globalZ = ntohl(globalWS->globalZ);
    DEBUG("%s: Global ws X:%d, Y %d, Z %d\n", __func__, globalX, globalY, globalZ);
    parent->groupSize[0] = globalX;
    parent->groupSize[1] = globalY;
    parent->groupSize[2] = globalZ;
    sendAck();
    return 0;  
}

/*!****************************************************************************
 * @brief handleKernelLoad Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
int ControlLink::sendAck(){
  /* send ack */
  char ackBuf[4];
  CommPacket_t *ack = (CommPacket_t *)ackBuf;
  int len = 4;
  ack->version = MORACL_PROTOCOL_VERSION;
  ack->cmdId = CTRL_ACK;
  ack->length = htons(len);
  
  if(len != send(connfd, ackBuf, len, 0)){
        perror("[CTRL] Unable to send ack");
  }
}

/*!****************************************************************************
 * @brief handleKernelLoad Load parts of the kernel
 * @param loadkernel pointer to Load kernel command payload
 * ***************************************************************************/
int ControlLink::sendErr(){
  /* send nak */
  char nakBuf[4];
  CommPacket_t *nak = (CommPacket_t *)nakBuf;
  int len = 4;
  nak->version = MORACL_PROTOCOL_VERSION;
  nak->cmdId = CTRL_NAK;
  nak->length = htons(len);
  
  if(len != send(connfd, nakBuf, len, 0)){
        perror("[CTRL] Unable to send nak");
  }
}



