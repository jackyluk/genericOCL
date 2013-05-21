/*!****************************************************************************
 * @file cl_mem.c OpenCL memory object implementation
 * @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
 *****************************************************************************/
#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cl_defs.h"
#include "dev_interface.h"
#include <arpa/inet.h>




cl_mem clCreateBuffer(
cl_context context,
cl_mem_flags flags,
size_t size,
void *host_ptr,
cl_int *errcode_ret)
{
    cl_mem mem;

    DEBUG("clCreateBuffer called\n");
    if( ((mem = (cl_mem)malloc(sizeof(struct _cl_mem))) == NULL) ){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    mem->refcount = 1; /* implicit retain */
    mem->offset = context->mem_alloc_offset;
    mem->size = size;
    context->mem_alloc_offset += size;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    
    return mem;
}



cl_int clRetainMemObject(
cl_mem memobj)
{
    DEBUG("clRetainMemObject called\n");
    if(memobj == NULL)
        return CL_INVALID_MEM_OBJECT;

    memobj->refcount += 1;

    return CL_SUCCESS;
}



cl_int clReleaseMemObject(
cl_mem memobj)
{
    DEBUG("clReleaseMemObject called\n");
    if(memobj == NULL)
        return CL_INVALID_MEM_OBJECT;

    if(memobj->refcount > 0)
        memobj->refcount -= 1;
    if(memobj->refcount == 0){
        free(memobj);
    }
    return CL_SUCCESS;
}



cl_int clEnqueueReadBuffer(
cl_command_queue command_queue,
cl_mem buffer,
cl_bool blocking_read,
size_t offset,
size_t cb,
void *ptr,
cl_uint num_events_in_wait_list,
const cl_event *event_wait_list,
cl_event *event)
{
    QueueCommand *newCmd;
    CommPacket_t *payload;
    const int cmdlen = 4 + 8;
    pthread_mutex_lock(&(command_queue->queue_mutex));
    DEBUG("%s called\n", __func__);
    newCmd = queue_add(command_queue);
    
    payload = calloc(cmdlen, 1);
    if(NULL == payload){
        pthread_mutex_unlock(&(command_queue->queue_mutex));
        return CL_OUT_OF_HOST_MEMORY;
    }
    
    if(NULL == newCmd){
        free(payload);
        pthread_mutex_unlock(&(command_queue->queue_mutex));
        return CL_OUT_OF_HOST_MEMORY;
    }
    
    newCmd->eventStatus = CL_QUEUED;
    newCmd->commandType = CL_COMMAND_READ_BUFFER;
    newCmd->payload = payload;
    newCmd->ret = ptr;
    
    payload->version = MORACL_PROTOCOL_VERSION;  
    payload->cmdId = MEM_READ_CMD;
    payload->payload.read.offset = htonl(buffer->offset);
    payload->payload.read.accessLength = htonl(cb);
    DEBUG("%s: Queue Read %d.\n", __func__, cb);
    payload->length = htons(cmdlen);
    pthread_mutex_unlock(&(command_queue->queue_mutex));
    return CL_SUCCESS;
}



cl_int clEnqueueWriteBuffer
(cl_command_queue command_queue,
cl_mem buffer,
cl_bool blocking_write,
size_t offset,
size_t cb,
const void *ptr,
cl_uint num_events_in_wait_list,
const cl_event *event_wait_list,
cl_event *event)
{
    QueueCommand *newCmd;
    CommPacket_t *payload;
    int cmdlen = 4 + 8 + cb;
    pthread_mutex_lock(&(command_queue->queue_mutex));
    DEBUG("%s called\n", __func__);
    newCmd = queue_add(command_queue);
    DEBUG("%s Added\n", __func__);
    payload = calloc(cmdlen, 1);
    if(NULL == payload){
        pthread_mutex_unlock(&(command_queue->queue_mutex));
        return CL_OUT_OF_HOST_MEMORY;
    }
    
    if(NULL == newCmd){
        free(payload);
        pthread_mutex_unlock(&(command_queue->queue_mutex));
        return CL_OUT_OF_HOST_MEMORY;
    }
    
    
    newCmd->eventStatus = CL_QUEUED;
    newCmd->commandType = CL_COMMAND_WRITE_BUFFER;
    newCmd->payload = payload;
    
    newCmd->ret = (void *)ptr;
    
    payload->version = MORACL_PROTOCOL_VERSION;  
    payload->cmdId = MEM_WRITE_CMD;
    payload->payload.write.offset = htonl(buffer->offset);
    payload->payload.write.accessLength = htonl(cb);
    memcpy(payload->payload.write.data, ptr, cb);
    payload->length = htons(cmdlen);
    
    pthread_mutex_unlock(&(command_queue->queue_mutex));
    return CL_SUCCESS;
}

void * clEnqueueMapBuffer 
(cl_command_queue command_queue,
cl_mem buffer,
cl_bool blocking_map,
cl_map_flags map_flags,
size_t offset,
size_t cb,
cl_uint num_events_in_wait_list,
const cl_event *event_wait_list,
cl_event *event,
cl_int *errcode_ret)
{
    QueueCommand *newCmd;
    CommPacket_t *payload;
    void *mappedMemory;
    const int cmdlen = 4 + 8;
    pthread_mutex_lock(&(command_queue->queue_mutex));
    DEBUG("%s called\n", __func__);
    newCmd = queue_add(command_queue);
    
    mappedMemory = calloc(cb, 1);
    if(NULL == mappedMemory){
        pthread_mutex_unlock(&(command_queue->queue_mutex));
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    
    payload = calloc(cmdlen, 1);
    if(NULL == payload){
        free(mappedMemory);
        pthread_mutex_unlock(&(command_queue->queue_mutex));
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    
    if(NULL == newCmd){
        free(mappedMemory);
        free(payload);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    
    DEBUG("Command: %p\n", newCmd);
    newCmd->eventStatus = CL_QUEUED;
    newCmd->commandType = CL_COMMAND_MAP_BUFFER;
    newCmd->payload = payload;
    newCmd->ret = mappedMemory;
    
    payload->version = MORACL_PROTOCOL_VERSION;  
    payload->cmdId = MEM_READ_CMD;
    payload->payload.read.offset = htonl(buffer->offset);
    payload->payload.read.accessLength = htonl(cb);
    DEBUG("%s: Queue Read %d.\n", __func__, cb);
    payload->length = htons(cmdlen);
    pthread_mutex_unlock(&(command_queue->queue_mutex));

    if(blocking_map == CL_TRUE){
        clFinish(command_queue);
    }
    
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    return mappedMemory;
}

cl_int clEnqueueUnmapMemObject
(cl_command_queue command_queue,
cl_mem memobj,
void *mapped_ptr,
cl_uint num_events_in_wait_list,
const cl_event *event_wait_list,
cl_event *event)
{
    QueueCommand *newCmd;
    const int cmdlen = 4 + 8;
    pthread_mutex_lock(&(command_queue->queue_mutex));
    DEBUG("%s called\n", __func__);
    newCmd = queue_add(command_queue);
    
    if(NULL == newCmd){
        return CL_OUT_OF_HOST_MEMORY;
    }
    
    DEBUG("Command: %p\n", newCmd);
    newCmd->eventStatus = CL_QUEUED;
    newCmd->commandType = CL_COMMAND_UNMAP_MEM_OBJECT;
    newCmd->payload = NULL;
    newCmd->ret = mapped_ptr;
    
    pthread_mutex_unlock(&(command_queue->queue_mutex));
    
    return CL_SUCCESS;
}
