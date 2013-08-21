/*!****************************************************************************
* @file cl_context.c OpenCL context implementation
* @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
*****************************************************************************/

#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cl_defs.h"

/*!
* @brief Creates an OpenCL context.
* @param properties Specifies a list of context property names and their corresponding values. Each property name is immediately followed by the corresponding desired value. The list is terminated with 0. properties can be NULL in which case the platform that is selected is implementation-defined. 
* @param num_devices The number of devices specified in the devices argument.
* @param devices A pointer to a list of unique devices returned by clGetDeviceIDs for a platform.
* @param pfn_notify A callback function that can be registered by the application. This callback function will be used by the OpenCL implementation to report information on errors that occur in this context. 
* @param user_data Passed as the user_data argument when pfn_notify is called. user_data can be NULL.
* @param errcode_ret Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
*/

cl_context clCreateContext(
const cl_context_properties *properties,
cl_uint num_devices,
const cl_device_id *devices,
void (CL_CALLBACK *pfn_notify)(const char *errinfo, const void *private_info, size_t cb, void *user_data),
void *user_data,
cl_int *errcode_ret)
{
    cl_platform_id pf;
    cl_context context;

    DEBUG("%s called\n", __func__);
    if(properties){
        if(properties[0] == (cl_context_properties)CL_CONTEXT_PLATFORM){
            pf = (cl_platform_id)properties[1];
        }
        else{
            if(errcode_ret) *errcode_ret = CL_INVALID_PROPERTY;
            return NULL;
        }
    }

    if(devices == NULL || num_devices == 0 || (pfn_notify == NULL && user_data != NULL)){
        if(errcode_ret != NULL) *errcode_ret = CL_INVALID_VALUE;
        return NULL;
    }

    if(*devices == NULL){
        if(errcode_ret != NULL) *errcode_ret = CL_INVALID_DEVICE;
        return NULL;
    }

    if(properties){
        if(pf != PF_ID){
            if(errcode_ret) *errcode_ret = CL_INVALID_PLATFORM;
            return NULL;
        }
    }

    if( (context = (cl_context)malloc(sizeof(struct _cl_context))) == NULL){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }

    context->refcount = 1; /* implicit retain */
    context->devices = calloc(num_devices, sizeof(cl_device_id));
    if(NULL == context->devices){
        free(context);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    
    /*! copy the list of device pointers to context memory */
    memcpy(context->devices, devices, num_devices * sizeof(cl_device_id));
    
    context->num_devices = num_devices;
    if(properties) memcpy(context->props, properties, sizeof(context->props));
    context->mem_alloc_offset = 0;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    
    return context;
}

/*!
* @brief Create an OpenCL context from a device type that identifies the specific device(s) to use.
* @param properties Specifies a list of context property names and their corresponding values. Each property name is immediately followed by the corresponding desired value. The list is terminated with 0. properties can be NULL in which case the platform that is selected is implementation-defined. 
* @param device_type The number of devices specified in the devices argument.
* @param pfn_notify A callback function that can be registered by the application. This callback function will be used by the OpenCL implementation to report information on errors that occur in this context. 
* @param user_data Passed as the user_data argument when pfn_notify is called. user_data can be NULL.
* @param errcode_ret Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
*/
cl_context clCreateContextFromType(
const cl_context_properties *properties,
cl_device_type device_type,
void (CL_CALLBACK *pfn_notify)(const char *errinfo, const void *private_info, size_t cb, void *user_data),
void *user_data,
cl_int *errcode_ret)
{
    cl_device_id newDevices[1];
    cl_context newContext;
    DEBUG("clCreateContextFromType called\n");
    *errcode_ret = clGetDeviceIDs(PF_ID, device_type, 1, newDevices, 0);
    if(*errcode_ret != CL_SUCCESS)
        return NULL;

    newContext = clCreateContext(properties, 1, newDevices, pfn_notify, user_data, errcode_ret);
    if(*errcode_ret != CL_SUCCESS){
        free(newDevices[0]);
        return NULL;
    }
    return newContext;
}



cl_int clRetainContext(
cl_context context)
{
    DEBUG("clRetainContext called\n");
    if(context == NULL)
        return CL_INVALID_CONTEXT;

    context->refcount += 1;

    return CL_SUCCESS;
}



cl_int clReleaseContext(
cl_context context)
{
    DEBUG("clReleaseContext called\n");
    if(context == NULL)
        return CL_INVALID_CONTEXT;

    if(context->refcount > 0)
        context->refcount -= 1;
    if(context->refcount == 0){
        cl_uint counter;
        
        for(counter = 0; counter < context->num_devices; counter++){
            free(context->devices[counter]);
        }
        free(context->devices);
        free(context);
    }

    return CL_SUCCESS;
}



cl_int clGetContextInfo(
cl_context context,
cl_context_info param_name,
size_t param_value_size,
void *param_value,
size_t *param_value_size_ret)
{
    void *param;
    size_t param_size;

    DEBUG("clGetContextInfo called\n");
    if(context == NULL)
        return CL_INVALID_CONTEXT;

    switch(param_name){
        case CL_CONTEXT_REFERENCE_COUNT:
            param = &context->refcount;
            param_size = sizeof(cl_uint);
            break;

        case CL_CONTEXT_NUM_DEVICES:
            param = &context->num_devices;
            param_size = sizeof(cl_uint);
            break;

        case CL_CONTEXT_DEVICES:
            param = context->devices;
            param_size = sizeof(context->devices);
            break;

        case CL_CONTEXT_PROPERTIES:
            param = context->props;
            param_size = sizeof(context->props);
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



