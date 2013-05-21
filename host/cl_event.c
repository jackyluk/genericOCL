/*!****************************************************************************
 * @file cl_event.c OpenCL event implementation
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cl_defs.h"




cl_event clCreateUserEvent(
cl_context context,
cl_int *errcode_ret)
{
    cl_event event;

    DEBUG("clCreateUserEvent called\n");
    if( (event = (cl_event)malloc(sizeof(struct _cl_event))) == NULL){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    event->refcount = 1; /* implicit retain */
    if(errcode_ret) *errcode_ret = CL_SUCCESS;

    return event;

}



cl_int clRetainEvent(
cl_event event)
{
    DEBUG("clRetainEvent called\n");
    if(event == NULL)
        return CL_INVALID_EVENT;

    event->refcount += 1;

    return CL_SUCCESS;

}



cl_int clReleaseEvent(
cl_event event)
{
    DEBUG("clReleaseEvent called\n");
    if(event == NULL)
        return CL_INVALID_EVENT;

    if(event->refcount > 0)
        event->refcount -= 1;
    if(event->refcount == 0)
        free(event);

    return CL_SUCCESS;

}



cl_int clWaitForEvents(
cl_uint num_events,
const cl_event *event_list)
{
    DEBUG("clWaitForEvents called\n");
    return CL_SUCCESS;
}
