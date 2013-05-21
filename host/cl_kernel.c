/*!****************************************************************************
 * @file cl_kernel.c OpenCL kernel implementation
 * @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
 *****************************************************************************/
#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cl_defs.h"
#include "dev_interface.h"



cl_kernel clCreateKernel(
cl_program program,
const char *kernel_name,
cl_int *errcode_ret)
{
    cl_kernel kernel;
    char* name;
    size_t name_len;

    DEBUG("clCreateKernel called\n");
    if(program == NULL){
        if(errcode_ret) *errcode_ret = CL_INVALID_PROGRAM;
        return NULL;
    }
    if((name_len = strlen(kernel_name)) < 1){
        if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
        return NULL;
    }

    if((name = (char*)malloc(++name_len)) == NULL){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }

    if( (kernel = (cl_kernel)malloc(sizeof(struct _cl_kernel))) == NULL){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    kernel->refcount = 1; /* implicit retain */
    kernel->arg_count = 0;
    strncpy(name, kernel_name, name_len);
    kernel->func_name = name;

    if(errcode_ret) *errcode_ret = CL_SUCCESS;

    return kernel;
}



cl_int clRetainKernel(
cl_kernel kernel)
{
    DEBUG("clRetainKernel called\n");
    if(kernel == NULL)
        return CL_INVALID_KERNEL;

    kernel->refcount += 1;

    return CL_SUCCESS;
}



cl_int clReleaseKernel(
cl_kernel kernel)
{
    DEBUG("clReleaseKernel called\n");
    if(kernel == NULL)
        return CL_INVALID_KERNEL;

    if(kernel->refcount > 0)
        kernel->refcount -= 1;
    if(kernel->refcount == 0)
        free(kernel);

    return CL_SUCCESS;
}



cl_int clSetKernelArg(
cl_kernel kernel,
cl_uint arg_index,
size_t arg_size,
const void *arg_value)
{
    DEBUG("clSetKernelArg called (index: %u, size: %zu)\n",arg_index, arg_size);
    if(kernel == NULL)
        return CL_INVALID_KERNEL;

    kernel->args[arg_index] = arg_size;
    kernel->arg_count++;

    return CL_SUCCESS;
}



char* get_kernel_args(cl_kernel kernel){
    int i, count;
    int total = 0;
    char arg_line[16];
    char* arg_str;

    if( (arg_str = (char*)malloc(sizeof(char)*512)) == NULL){
        return NULL;
    }
    count = kernel->arg_count;
    arg_str[0] = '\0';
    for(i=0; i<count; i++){
        sprintf(arg_line, "%d %zu\n", total, kernel->args[i]);
        strcat(arg_str, arg_line);
        total += kernel->args[i];
    }

    return arg_str;
}



cl_int clEnqueueNDRangeKernel(
cl_command_queue command_queue,
cl_kernel kernel,
cl_uint work_dim,
const size_t *global_work_offset,
const size_t *global_work_size,
const size_t *local_work_size,
cl_uint num_events_in_wait_list,
const cl_event *event_wait_list,
cl_event *event)
{
    QueueCommand *newCmd;
    CommPacket_t *payload;
    const int cmdlen = sizeof(ND_Kernel_Cmd_Params);
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
    newCmd->commandType = CL_COMMAND_NDRANGE_KERNEL;
    newCmd->payload = payload;
    newCmd->ret = NULL;
    
    /*! Use the payload area of the command object for parameters*/
    ND_Kernel_Cmd_Params *params = payload;
    params->globalWorkSize.globalX = global_work_size[0];
    params->globalWorkSize.globalY = (work_dim >=2) ? global_work_size[1] : 1;
    params->globalWorkSize.globalZ = (work_dim >=3) ? global_work_size[2] : 1;
    params->kernel = kernel;
    pthread_mutex_unlock(&(command_queue->queue_mutex));
    return CL_SUCCESS;
}

cl_int clGetKernelWorkGroupInfo (
    cl_kernel kernel,
    cl_device_id device,
    cl_kernel_work_group_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret
){
    if(kernel == NULL){
        return CL_INVALID_KERNEL;
    }
    //TODO Associate kernel with device and do relevent checks on the value of device.
    
    if(param_value == NULL){
        return CL_INVALID_VALUE;
    }
    
    switch(param_name){
        
        case CL_KERNEL_WORK_GROUP_SIZE:
            if(param_value_size >= sizeof(size_t)){
                //Statically defined, but should have been related to the device hardware and kernel resource usage
                *(size_t *)param_value = MAX_WORK_GROUP_SIZE; 
                if(param_value_size_ret) *param_value_size_ret = sizeof(size_t);
            }else{
                return CL_INVALID_VALUE;
            }
            break;
            
        case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
            if(param_value_size >= sizeof(size_t) * 3){
                //Assume optional attribute of worksize not present in the kernel source.
                ((size_t *)param_value)[0] = 0;
                ((size_t *)param_value)[1] = 0;
                ((size_t *)param_value)[2] = 0;
                if(param_value_size_ret) *param_value_size_ret = sizeof(size_t) * 3;
            }else{
                return CL_INVALID_VALUE;
            }
            break;
        case CL_KERNEL_LOCAL_MEM_SIZE:
            if(param_value_size >= sizeof(cl_ulong)){
                //Assume not specified in the kernel source.
                ((cl_ulong *)param_value)[0] = 0;
                if(param_value_size_ret) *param_value_size_ret = sizeof(cl_ulong);
            }else{
                return CL_INVALID_VALUE;
            }
            break;
        case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
            if(param_value_size >= sizeof(size_t)){
                //Statically defined, but should have been related to the device hardware and kernel resource usage
                *(size_t *)param_value = PREFERRED_WORK_GROUP_SIZE; 
                if(param_value_size_ret) *param_value_size_ret = sizeof(size_t);
            }else{
                return CL_INVALID_VALUE;
            }
            break;
        case CL_KERNEL_PRIVATE_MEM_SIZE:
            if(param_value_size >= sizeof(cl_ulong)){
                //Assume not specified in the kernel source.
                ((cl_ulong *)param_value)[0] = 512; //Some number
                if(param_value_size_ret) *param_value_size_ret = sizeof(cl_ulong);
            }else{
                return CL_INVALID_VALUE;
            }
            break;
        default:
            return CL_INVALID_VALUE;
    }
    
    return CL_SUCCESS;
}
