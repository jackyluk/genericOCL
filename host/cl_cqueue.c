/*!****************************************************************************
 * @file cl_queue.c OpenCL queue implementation
 * @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
 *****************************************************************************/

#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cl_defs.h"
#include "dev_interface.h"
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>

void *queue_worker(void *arg);


cl_command_queue clCreateCommandQueue(
cl_context context,
cl_device_id device,
cl_command_queue_properties properties,
cl_int *errcode_ret)
{
    cl_command_queue cqueue;
    int fd_ctrl;

    DEBUG("clCreateCommandQueue called\n");
    if( (cqueue = (cl_command_queue)malloc(sizeof(struct _cl_command_queue))) == NULL){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    
    if(device->connected == CL_FALSE){
        fd_ctrl = dev_connect(CONN_CTRL);
        if( (fd_ctrl  == -1) ){
            if(errcode_ret) *errcode_ret = CL_INVALID_DEVICE;
            return NULL;
        }
        device->connected = CL_TRUE;
        device->fd_ctrl = fd_ctrl;
    }
    else{ /* only one control and data connection to device allowed */
        if(errcode_ret) *errcode_ret = CL_OUT_OF_RESOURCES;
        return NULL;
    }
    
    cqueue->refcount = 1; /* implicit retain */
    cqueue->context = context;
    cqueue->device = device;
    cqueue->props = properties;
    cqueue->queue = NULL;
    cqueue->currentEvent = NULL;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    
    pthread_mutex_init(&(cqueue->queue_mutex), NULL);
    /*! Start queue thread*/
    pthread_create(&(cqueue->queue_thread), NULL, queue_worker, cqueue);
    
    return cqueue;
}



cl_int clRetainCommandQueue(
cl_command_queue command_queue)
{
    DEBUG("clRetainCommandQueue called\n");
    if(command_queue == NULL)
        return CL_INVALID_COMMAND_QUEUE;

    command_queue->refcount += 1;

    return CL_SUCCESS;
}



cl_int clReleaseCommandQueue(
cl_command_queue command_queue)
{
    DEBUG("clReleaseCommandQueue called\n");
    if(command_queue == NULL)
        return CL_INVALID_COMMAND_QUEUE;

    if(command_queue->refcount > 0)
        command_queue->refcount -= 1;
    if(command_queue->refcount == 0){
                
        /*! Wait for the queue thread to stop*/
        pthread_join(command_queue->queue_thread, NULL);
        
        dev_disconnect(command_queue->device->fd_ctrl);
        command_queue->device->connected = CL_FALSE;
        free(command_queue);
    }

    return CL_SUCCESS;
}



cl_int clGetCommandQueueInfo(
cl_command_queue command_queue,
cl_command_queue_info param_name,
size_t param_value_size,
void *param_value,
size_t *param_value_size_ret)
{
    void *param;
    size_t param_size;

    DEBUG("clGetCommandQueueInfo called\n");
    if(command_queue == NULL)
        return CL_INVALID_COMMAND_QUEUE;

    switch(param_name){
        case CL_QUEUE_REFERENCE_COUNT:
            param = &command_queue->refcount;
            param_size = sizeof(cl_uint);
            break;

        case CL_QUEUE_CONTEXT:
            param = command_queue->context;
            param_size = sizeof(command_queue->context);
            break;

        case CL_QUEUE_DEVICE:
            param = command_queue->device;
            param_size = sizeof(command_queue->device);
            break;

        case CL_QUEUE_PROPERTIES:
            param = &command_queue->props;
            param_size = sizeof(cl_command_queue_properties);
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if(param_value_size_ret)
        *param_value_size_ret = param_size;

    if(param_value){
        if(!param_value_size || param_value_size < param_size) return CL_INVALID_VALUE;
        memcpy(param_value, param, param_size);
    }

    return CL_SUCCESS;

}

cl_int clFlush(cl_command_queue command_queue){
    return CL_SUCCESS;
}

cl_int clFinish(cl_command_queue command_queue){
    int done;
    /*! Sanity check */
    if(command_queue == NULL)
        return CL_INVALID_COMMAND_QUEUE;
    
    
    while(1){
        pthread_mutex_lock(&(command_queue->queue_mutex));
        done = (command_queue->currentEvent == NULL);
        pthread_mutex_unlock(&(command_queue->queue_mutex));
        if(done){
            return CL_SUCCESS;
        }else{
            usleep(1000);
        }
    }
 
    return CL_SUCCESS;
}

cl_int clEnqueueBarrier(cl_command_queue command_queue){
    QueueCommand *newCmd;
    pthread_mutex_lock(&(command_queue->queue_mutex));
    DEBUG("%s called\n", __func__);
    newCmd = queue_add(command_queue);
    if(NULL == newCmd){
        pthread_mutex_unlock(&(command_queue->queue_mutex));
        return CL_OUT_OF_HOST_MEMORY;
    }
    
    newCmd->eventStatus = CL_QUEUED;
    newCmd->commandType = CL_CUSTOM_COMMAND_BARRIER;
    newCmd->payload = NULL;
    pthread_mutex_unlock(&(command_queue->queue_mutex));
    return CL_SUCCESS;
}

void *queue_worker(void *arg){
    cl_command_queue queue = (cl_command_queue)arg; 
    time_t now;
    QueueCommand *qpos;
        
    /*! Sanity check */
    if(queue == NULL) return NULL;
    
    DEBUG("%s %p\n", __func__, arg);
    /*!Keep queue alive if reference count is not zero*/
    while(queue->refcount){
        pthread_mutex_lock(&(queue->queue_mutex));
        if(queue->currentEvent != NULL){
            queue_dispatchCommand(queue, queue->currentEvent);
            queue->currentEvent = queue->currentEvent->next;
            DEBUG("%s Next event -> %p \n", __func__, queue->currentEvent);
        }
        time(&now);
        
        qpos = queue->queue;
        while(qpos && qpos->eventStatus == CL_COMPLETE){
            if((now - qpos->completionTime) >= EVENT_EXPIRY_TIME_S){
                //Completed and expired.
                queue_remove(queue, qpos);
            }
            qpos = qpos->next;
        }
        pthread_mutex_unlock(&(queue->queue_mutex));
        usleep(1);
    }
    DEBUG("%s %p Joining\n", __func__, arg);
    return NULL;
}

void queue_dispatchCommand(cl_command_queue command_queue, QueueCommand *command){
    int fd;
    char cmdRsp[64*1024];
    int reqLength;
    int rspLength;
    int retLength;
    
    fd = command_queue->device->fd_ctrl;
    CommPacket_t *payload = (CommPacket_t *)command->payload;
    CommPacket_t *readRspPacket = (CommPacket_t *)cmdRsp;
    switch(command->commandType){
        case CL_COMMAND_READ_BUFFER:
            DEBUG("%s: Submitting Read buffer.\n", __func__);
            reqLength = ntohs(payload->length);
            DEBUG("%s: Read PKG 1 %d\n", __func__, reqLength);
            retLength = ntohl(payload->payload.read.accessLength); /*! Expected return length */
            DEBUG("%s: Read 1 %d\n", __func__, retLength);
            rspLength = 4 + 8 + retLength;
            dev_write(fd, command->payload, reqLength);
            dev_read(fd, cmdRsp, rspLength);
            if(command->ret){
                DEBUG("%s: Read %d\n", __func__, retLength);
                memcpy(command->ret, readRspPacket->payload.read.data, retLength);
            }
            DEBUG("%s: Submitting Read buffer. Return\n", __func__);
            break;
            
        case CL_COMMAND_WRITE_BUFFER:
            DEBUG("%s: Submitting Write buffer.\n", __func__);
            reqLength = ntohs(payload->length);
            rspLength = 4; //Ack or Nak
            dev_write(fd, command->payload, reqLength);
            
            dev_read(fd, cmdRsp, rspLength);
            DEBUG("%s: Submitting Write buffer. Return\n", __func__);
            //TODO Set error based on ACK or NAK
            break;
        
        case CL_COMMAND_NDRANGE_KERNEL:
            DEBUG("%s: Submitting NDRange Kernel.\n", __func__);
            dispatchNDRangeKernel(fd, command);
            DEBUG("%s: Submitting NDRange Kernel. Return\n", __func__);
            break;
            
        case CL_COMMAND_MAP_BUFFER:
            DEBUG("%s: Submitting Map (Read) buffer.\n", __func__);
            reqLength = ntohs(payload->length);
            DEBUG("%s: Read PKG 1 %d\n", __func__, reqLength);
            retLength = ntohl(payload->payload.read.accessLength); /*! Expected return length */
            DEBUG("%s: Read 1 %d\n", __func__, retLength);
            rspLength = 4 + 8 + retLength;
            dev_write(fd, command->payload, reqLength);
            dev_read(fd, cmdRsp, rspLength);
            if(command->ret){
                DEBUG("%s: Read %d\n", __func__, retLength);
                memcpy(command->ret, readRspPacket->payload.read.data, retLength);
            }
            DEBUG("%s: Submitting Map buffer. Return\n", __func__);
            break;
            
        case CL_COMMAND_UNMAP_MEM_OBJECT:
            DEBUG("%s: UnMap Memory object.\n", __func__);
            if(command->ret){
                free(command->ret);
                command->ret = NULL;
            }
            break;
        case CL_CUSTOM_COMMAND_BARRIER:
             DEBUG("%s: Barrier command.\n", __func__);
             break;
        case CL_COMMAND_TASK:
        case CL_COMMAND_NATIVE_KERNEL:
        case CL_COMMAND_COPY_BUFFER:
        case CL_COMMAND_COPY_IMAGE:
        case CL_COMMAND_COPY_IMAGE_TO_BUFFER:
        case CL_COMMAND_COPY_BUFFER_TO_IMAGE:
        case CL_COMMAND_MAP_IMAGE:

        case CL_COMMAND_MARKER:
        case CL_COMMAND_ACQUIRE_GL_OBJECTS:
        case CL_COMMAND_RELEASE_GL_OBJECTS:
        case CL_COMMAND_READ_BUFFER_RECT:
        case CL_COMMAND_WRITE_BUFFER_RECT:
        case CL_COMMAND_COPY_BUFFER_RECT:
        case CL_COMMAND_USER:
            DEBUG("%s: Command 0x%04x not yet supported. \n", __func__, command->commandType);
            break;
        default:
            DEBUG("%s: Command 0x%04x unknown.\n", __func__, command->commandType);
            break;
    }
    command->eventStatus = CL_COMPLETE;
    time(&(command->completionTime));
}

void dispatchNDRangeKernel(int fd, QueueCommand *command){
    char buf[1024];
    char cmd[256];
    char* args;
    char* name;
    DEBUG("%s: Entered \n", __func__);
    ND_Kernel_Cmd_Params *params = command->payload;
    CommPacket_t *globalWorkSize = (CommPacket_t *)buf;
    CommPacket_t *rsp = (CommPacket_t *)buf;
    
    /* set kernel args on the device */
    args = get_kernel_args(params->kernel);
    dev_set_kernel_args(args);
    free(args);
    
    /*! Set Global size */
    globalWorkSize->version = MORACL_PROTOCOL_VERSION;
    globalWorkSize->cmdId = GLOBAL_WORK_SIZE;
    globalWorkSize->length = htons(4 + 12);
    globalWorkSize->payload.globalWorkSize.globalX = htonl(params->globalWorkSize.globalX);
    globalWorkSize->payload.globalWorkSize.globalY = htonl(params->globalWorkSize.globalY);
    globalWorkSize->payload.globalWorkSize.globalZ = htonl(params->globalWorkSize.globalZ);
    dev_write(fd, buf, (4+12));
    dev_read(fd, buf, 4);
    if(rsp->cmdId == CTRL_ACK){
        DEBUG("%s: Global work size set.\n", __func__);
    }else if(rsp->cmdId != CTRL_NAK){
        DEBUG("%s: Device response protocol error.\n", __func__);
        time(&(command->completionTime));
        command->eventStatus = CL_COMPLETE;
        return;
//         return CL_INVALID_GLOBAL_WORK_SIZE;
    }else{
        DEBUG("%s: Device error while setting global work size.\n", __func__);
        time(&(command->completionTime));
        command->eventStatus = CL_COMPLETE;
        return;
//         return CL_INVALID_GLOBAL_WORK_SIZE;
    }
    
    /*! Compile kernel*/
    name = params->kernel->func_name;

    DEBUG("clEnqueueNDRangeKernel: performing kernel compilation\n");
    /* create kernel wrapper from kernelargs and compile the kernel */
    snprintf(cmd, 256, "%s ./createwrapper.py %s %d %d && ./compilekernel.sh %s.cl", PYTHON, name, params->globalWorkSize.globalX, params->globalWorkSize.globalY, name); 

    if(system(cmd)){
        // TODO Set error
        DEBUG("clEnqueueNDRangeKernel: kernel compilation failed\n");
        time(&(command->completionTime));
        command->eventStatus = CL_COMPLETE;
        return;
    }

    DEBUG("clEnqueueNDRangeKernel: telling device to start processing\n");
    
    /* send kernel to device */
    FILE *kfd = fopen("kernel.so", "rb");
    int kernelLoaded = 0;
    
    if (kfd!=NULL){
        /*! Get kernel size */
        fseek (kfd , 0 , SEEK_END);
        int lSize = ftell (kfd);
        
        int transferred;
        int result;
        int dataSize;
        int commSize;
        CommPacket_t *loadKernel = (CommPacket_t *)buf;

        /*! Rewind kernel file) */
        fseek (kfd , 0 , SEEK_SET);

        transferred = 0;
        while(transferred < lSize){
            loadKernel->version = MORACL_PROTOCOL_VERSION;
            loadKernel->cmdId = LOAD_KERNEL_IMAGE;
            loadKernel->payload.loadkernel.totalSize = htonl(lSize);
            loadKernel->payload.loadkernel.offset = htonl(transferred);
            
            dataSize = (lSize - transferred > 512)?512:(lSize - transferred);
            
            
            result = fread (loadKernel->payload.loadkernel.data, 1, dataSize, kfd);
            if(result > 0){
                DEBUG("%s: Transferring kernel: %d of %d bytes.\n", __func__, transferred, lSize);
                loadKernel->payload.loadkernel.dataSize = htonl(result);
                
                commSize = 4 + 12 + result;
                loadKernel->length = htons(commSize);
                
                dev_write(fd, loadKernel, commSize);
                dev_read(fd, buf, 4);
                
                if(rsp->cmdId == CTRL_ACK){
                    transferred += result;
                    if(transferred >= lSize){
                        DEBUG("%s: Kernel transferred.\n", __func__);
                        kernelLoaded = 1;
                    }
                }else if(rsp->cmdId != CTRL_NAK){
                    DEBUG("%s: Device error while transferring kernel.\n", __func__);
                    transferred = 0;
                    break;
                }
            }
        }
          
        fclose (kfd);
    }else{
      DEBUG("%s: Host error while transferring kernel.\n", __func__);
    }
    
    if(!kernelLoaded){
        // TODO Set Error
        time(&(command->completionTime));
        command->eventStatus = CL_COMPLETE;
        return;
//       return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
    }
    
    CommPacket_t *processCmd = (CommPacket_t *)buf;
    /*! Prefill some parts of the start command message */
    processCmd->version = MORACL_PROTOCOL_VERSION;
    processCmd->cmdId = START_KERNEL;
    processCmd->length = htons(4);
    dev_write(fd, processCmd, 4);
    dev_read(fd, buf, 4);
    time(&(command->completionTime));
    command->eventStatus = CL_COMPLETE;
    return;
//     *event = clCreateUserEvent(0, &err);
//     if(err != CL_SUCCESS)
//         return CL_OUT_OF_HOST_MEMORY;
    
}


/*! 
 * @brief Add a command to a command queue
 * @param command_queue Command queue object
 * @return Pointer to the newly queued command object. NULL if unsuccessful.
 */
QueueCommand* queue_add(cl_command_queue command_queue){
    QueueCommand *newCmd;
    
    
    /*! Sanity check if command queue is NULL */
    if(NULL == command_queue) return NULL;
    
    
    /*! Attempt to allocate memory for a new command */
    newCmd = calloc(sizeof(QueueCommand), 1);
    if(NULL == newCmd) return NULL;
    
    /*! Put new element to the back */
    if(command_queue->queue == NULL){
        command_queue->queue = newCmd;
    }else{
        QueueCommand *tail = command_queue->queue;
        while(tail->next){
            tail = tail->next;
        }
        tail->next = newCmd;
    }
    
    /*! Schedule the new event if there is nothing going on*/
    if(command_queue->currentEvent == NULL){
        DEBUG("%s: Setting current command.\n", __func__);
        command_queue->currentEvent = newCmd;
        DEBUG("%s: Next event -> %p \n", __func__, command_queue->currentEvent);
    }
    return newCmd;
}

/*! 
 * @brief Get pointer to the command at the head of a command queue
 * @param command_queue Command queue object
 * @return Pointer to the newly queued command object. NULL if unsuccessful.
 */
QueueCommand* queue_head(cl_command_queue command_queue){
    QueueCommand *head;
    
    /*! Sanity check if command queue is NULL */
    if(NULL == command_queue) return NULL;
    
    pthread_mutex_lock(&(command_queue->queue_mutex));
    head = command_queue->queue;
    
    /*! Check if the command queue has any elements */
    if(NULL == head) return NULL;
    
    /*! Get to the head of the queue */
    while(head){
        if(head->next == NULL){
            return head;
        }else{
            head = head->next;
        }
    }
    pthread_mutex_unlock(&(command_queue->queue_mutex));
    return NULL;
}


/*! 
 * @brief Remove a command from a queue
 * @param command_queue Command queue object
* @param cmd Pointer to command
 * @return Pointer to the newly queued command object. NULL if unsuccessful.
 */
void queue_remove(cl_command_queue command_queue, QueueCommand *cmd){
    QueueCommand *tmp;
    QueueCommand **prev;
    
    /*! Sanity check if command queue is NULL */
    if(NULL == command_queue) return;
    
    prev = &(command_queue->queue);
    
    while(*prev){
        if(cmd == *prev){
            *prev = (*prev)->next;
            prev = &((*prev)->next);
            free(cmd->payload);
            free(cmd);
        }
    }

    return;
}






























