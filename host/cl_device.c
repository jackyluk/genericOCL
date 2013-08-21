/*!****************************************************************************
* @file cl_device.c OpenCL device implementation
* @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
*****************************************************************************/
#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cl_defs.h"

static char *clDeviceExtensions = "";


cl_int clGetDeviceIDs(
cl_platform_id platform,
cl_device_type device_type,
cl_uint num_entries,
cl_device_id *devices,
cl_uint *num_devices)
{
    cl_device_id fpga;

    DEBUG("%s called\n", __func__);
    
    if(platform != PF_ID){
        DEBUG("%s: Invalid Platform ID (%d)\n", __func__, platform);
        return CL_INVALID_PLATFORM;
    }

    if( ((num_entries == 0) && (devices != NULL)) || ((num_devices == NULL) && (devices == NULL)) ){
        DEBUG("%s: Invalid Device numbers, num entry %d\n", __func__, num_entries);
        return CL_INVALID_VALUE;
    }

    if( (device_type != CL_DEVICE_TYPE_ACCELERATOR) &&
        (device_type != CL_DEVICE_TYPE_DEFAULT) && 
        (device_type != CL_DEVICE_TYPE_GPU) &&
        (device_type != CL_DEVICE_TYPE_ALL)){
        DEBUG("%s: Device Not found (Device Type %d)\n", __func__, device_type);
        return CL_DEVICE_NOT_FOUND;
    }

    DEBUG("%s: Warning: Not fully implemented\n", __func__);
    if(devices){
        if((fpga = (cl_device_id)malloc(sizeof(struct _cl_device_id))) == NULL)
            return CL_OUT_OF_HOST_MEMORY;
        fpga->type = DV_TYPE;
        fpga->vendor = strdup(DV_VENDOR);
        fpga->name = strdup(DV_NAME);
        fpga->version = strdup(DV_VERSION);
        fpga->connected = CL_FALSE;
        fpga->preferred_vector_width_char = PREFERRED_VECTOR_WIDTH_CHAR;
        devices[0] = fpga;
    }
    
    if(num_devices){
        *num_devices = 1;
    }
    return CL_SUCCESS;
}



cl_int clGetDeviceInfo(
cl_device_id device,
cl_device_info param_name,
size_t param_value_size,
void *param_value,
size_t *param_value_size_ret)
{
    void *param;
    size_t param_size;

    DEBUG("%s called\n", __func__);
    if(device == NULL)
        return CL_INVALID_DEVICE;

    switch(param_name){
        case CL_DEVICE_TYPE:
            DEBUG("%s: Device Type \n", __func__);
            param = &device->type;
            param_size = sizeof(param);
            break;

        case CL_DEVICE_VENDOR:
            DEBUG("%s: Device vendor \n", __func__);
            param = device->vendor;
            param_size = strlen(param)+1;
            break;
            
        case CL_DEVICE_VERSION:
            DEBUG("%s: Device Version \n", __func__);
            param = device->vendor;
            param_size = strlen(param)+1;
            break;
        
        
        case CL_DEVICE_NAME:
            DEBUG("%s: Device name \n", __func__);
            param = device->name;
            param_size = strlen(param)+1;
            break;
            
        case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
            DEBUG("%s: Device Base Addr Align \n", __func__);
            *(cl_uint *)param_value = sizeof(void *) << 3;
            if(param_value_size_ret) *param_value_size_ret = sizeof(cl_uint);
            return CL_SUCCESS;
            break;
        case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
            DEBUG("%s: Device Max Mem Alloc size \n", __func__);
            *(cl_ulong *)param_value = MAX_MEM_ALLOC_SIZE >> 2;
            if(param_value_size_ret) *param_value_size_ret = sizeof(cl_ulong);
            return CL_SUCCESS;
            break;
        case CL_DEVICE_GLOBAL_MEM_SIZE:
            DEBUG("%s: Device Global mem size \n", __func__);
            *(cl_ulong *)param_value = MAX_MEM_ALLOC_SIZE;
            if(param_value_size_ret) *param_value_size_ret = sizeof(cl_ulong);
            return CL_SUCCESS;
            break;
            
        case CL_DEVICE_MAX_WORK_ITEM_SIZES:
            DEBUG("%s: Device Max work item sizes \n", __func__);
            ((size_t *)param_value)[0] = MAX_WORK_ITEM_SIZES;
            ((size_t *)param_value)[1] = MAX_WORK_ITEM_SIZES;
            ((size_t *)param_value)[2] = MAX_WORK_ITEM_SIZES;
            if(param_value_size_ret) *param_value_size_ret = sizeof(size_t[3]);
            return CL_SUCCESS;
            break;
            
        case CL_DEVICE_MAX_WORK_GROUP_SIZE:
            DEBUG("%s: Device Max Work Group size \n", __func__);
            *(size_t *)param_value = MAX_WORK_GROUP_SIZE;
            if(param_value_size_ret) *param_value_size_ret = sizeof(size_t);
            return CL_SUCCESS;
            break;
            
        case CL_DEVICE_EXTENSIONS:
            DEBUG("%s: Device Extensions \n", __func__);
            *(size_t *)param_value = sizeof(clDeviceExtensions);
            if(param_value_size_ret) *param_value_size_ret = clDeviceExtensions;
            return CL_SUCCESS;
            break;
            
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
            DEBUG("%s: Device Preferred Vector Width Int\n", __func__);
            param_size = sizeof(cl_uint);
            *(cl_uint *)param = device->preferred_vector_width_char;
            break;
            
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
            DEBUG("%s: Device Preferred Vector Width Int\n", __func__);
            param_size = sizeof(cl_uint);
            *(cl_uint *)param = device->preferred_vector_width_char/sizeof(cl_short);
            break;
            
        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
            DEBUG("%s: Device Preferred Vector Width Int\n", __func__);
            param_size = sizeof(cl_uint);
            *(cl_uint *)param = device->preferred_vector_width_char/sizeof(cl_int);
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
