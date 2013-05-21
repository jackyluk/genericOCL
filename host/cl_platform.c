/*!****************************************************************************
 * @file cl_platform.c OpenCL platform implementation
 * @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
 *****************************************************************************/
#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cl_defs.h"

struct _cl_platform_id platformID_0 = {
        PF_NAME, 
        PF_VENDOR,
        PF_PROFILE,
        PF_VERSION,
        PF_EXTENSIONS,
        0,
        NULL
};


cl_int clGetPlatformIDs(
cl_uint num_entries,
cl_platform_id *platforms,
cl_uint *num_platforms)
{
    DEBUG("clGetPlatformIDs called\n");
    
    
    if( ((num_entries == 0) && (platforms != NULL)) || ((num_platforms == NULL) && (platforms == NULL)) )
        return CL_INVALID_VALUE;
    if(platforms){
        platforms[0] = &platformID_0; /* platform identifier */
    }
    if(num_platforms)
        *num_platforms = 1;


    
    
    return CL_SUCCESS;
}



cl_int clGetPlatformInfo(
cl_platform_id platform,
cl_platform_info param_name,
size_t param_value_size,
void *param_value,
size_t *param_value_size_ret)
{
    void *param;
    size_t param_size = 0;

    DEBUG("%s called\n", __func__);
    if(platform != PF_ID)
        return CL_INVALID_PLATFORM;
   
    switch(param_name){
        case CL_PLATFORM_PROFILE:
            DEBUG("%s CL_PLATFORM_PROFILE\n", __func__);
            param = PF_PROFILE;
            break;

        case CL_PLATFORM_NAME:
            DEBUG("%s CL_PLATFORM_NAME\n", __func__);
            param = PF_NAME;
            break;

        case CL_PLATFORM_VENDOR:
            DEBUG("%s CL_PLATFORM_VENDOR\n", __func__);
            param = PF_VENDOR;
            break;

        case CL_PLATFORM_VERSION:
            DEBUG("%s CL_PLATFORM_VERSION\n", __func__);
            param = PF_VERSION;
            break;

        case CL_PLATFORM_EXTENSIONS:
            DEBUG("%s CL_PLATFORM_EXTENSIONS\n", __func__);
            param = PF_EXTENSIONS;
            break;

        default:
            return CL_INVALID_VALUE;
    }

    param_size = strlen(param)+1;
    if(param_value_size_ret)
        *param_value_size_ret = param_size;

    if(param_value){
        if(!param_value_size || param_value_size < param_size) return CL_INVALID_VALUE;
        memcpy(param_value, param, param_size);
    }

    return CL_SUCCESS;
}
