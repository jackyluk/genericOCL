/*!****************************************************************************
 * @file cl_program.c OpenCL program implementation
 * @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
 *****************************************************************************/
#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cl_defs.h"
#include "dev_interface.h"

void cl_program_sanitizeBuildOptions(const char *options, char *safeOptions);

cl_program clCreateProgramWithSource(
cl_context context,
cl_uint count,
const char **strings,
const size_t *lengths,
cl_int *errcode_ret)
{
    cl_program prog;
    char cmd[256];
    int line;

    DEBUG("clCreateProgramWithSource called\n");
    if( (prog = (cl_program)malloc(sizeof(struct _cl_program))) == NULL){
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    prog->refcount = 1; /* implicit retain */
    prog->source.count = count;
    prog->source.strings = calloc(sizeof(char **), count);
    if(prog->source.strings == NULL){
        free(prog);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    
    prog->source.lengths = calloc(sizeof(size_t *), count);
    if(prog->source.lengths == NULL){
        free(prog->source.strings);
        free(prog);
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    
    if(lengths == NULL){
        if(count != 1 || strings[0] == NULL){
            free(prog->source.strings);
            free(prog);
            if(errcode_ret) *errcode_ret = CL_INVALID_VALUE;
            return NULL;
        }
        prog->source.lengths[0] = strlen(strings[0]);
    }
        
    DEBUG("clCreateProgramWithSource: Copying source, count %d\n", count);
    for(line = 0; line < count; line++){
        if(lengths != NULL) prog->source.lengths[line] = lengths[line];
        prog->source.strings[line] = calloc(prog->source.lengths[line], 1);
        if(prog->source.strings[line] == NULL){
            for(line = 0; line < count; line++){
                if(prog->source.strings[line]) free(prog->source.strings[line]);
            }
            free(prog->source.lengths);
            free(prog->source.strings);
            free(prog);
            if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        memcpy(prog->source.strings[line], strings[line], prog->source.lengths[line]);
        
        DEBUG("clCreateProgramWithSource: Copying source, Length %d\n", prog->source.lengths[line]);
    }
    prog->hasBinary = CL_FALSE;
    prog->createdWithBinary = CL_FALSE;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;

    return prog;
}

cl_program clCreateProgramWithBinary (  cl_context context,
        cl_uint num_devices,
        const cl_device_id *device_list,
        const size_t *lengths,
        const unsigned char **binaries,
        cl_int *binary_status,
        cl_int *errcode_ret){
            
    DEBUG("Entering %s\n", __func__);
    
//     prog->hasBinary = CL_TRUE;
//     prog->createdWithBinary = CL_TRUE;
    return CL_OUT_OF_HOST_MEMORY;
    
}


cl_int clRetainProgram(
cl_program program)
{
    DEBUG("clRetainProgram called\n");
    if(program == NULL)
        return CL_INVALID_PROGRAM;

    program->refcount += 1;

    return CL_SUCCESS;
}



cl_int clReleaseProgram(
cl_program program)
{
    DEBUG("clReleaseProgram called\n");
    if(program == NULL)
        return CL_INVALID_PROGRAM;

    if(program->refcount > 0)
        program->refcount -= 1;
    if(program->refcount == 0){
        if(program->createdWithBinary == CL_FALSE){
            int line;
        
            for(line = 0; line < program->source.count; line++){
                if(program->source.strings[line]) free(program->source.strings[line]);
            }
            free(program->source.lengths);
            free(program->source.strings);
        }
        free(program);
    }
    return CL_SUCCESS;
}

cl_int clBuildProgram(
cl_program program,
cl_uint num_devices,
const cl_device_id *device_list,
const char *options,
void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
void *user_data)
{
    char cmd[256];
    DEBUG("clBuildProgram called\n");
    if(program == NULL)
        return CL_INVALID_PROGRAM;
    
    if(program->createdWithBinary == CL_TRUE){
        return CL_SUCCESS;
    }
    char safeOptions[512];
    
    DEBUG("clBuildProgram: buildoptions %s\n", options);
    cl_program_sanitizeBuildOptions(options, safeOptions);
    DEBUG("clBuildProgram: safe buildoptions %s\n", safeOptions);
    FILE *optionsFile = fopen("tmp.options", "wb+");
    if (optionsFile != NULL){
        fwrite("CLFLAGS=\"", 9, 1, optionsFile);
        fwrite(safeOptions, strlen(safeOptions), 1, optionsFile);
        fwrite("\"", 1, 1, optionsFile);
        fclose(optionsFile);
        optionsFile = NULL;
    }else{
        DEBUG("%s: build options store failed.\n", __func__);
        return CL_BUILD_PROGRAM_FAILURE;
    }
    
    FILE *srcFile = fopen("tmp.cl", "wb+");
    
    if (srcFile != NULL){
        int line;
        for(line = 0; line < program->source.count; line++){
            fwrite(program->source.strings[line], program->source.lengths[line], 1, srcFile);
        }
        fclose(srcFile);
        
        DEBUG("%s: performing program build\n", __func__);
        snprintf(cmd, 256, " ./buildprogram.sh tmp.cl "); 

        if(system(cmd)){
            DEBUG("%s: program build error\n", __func__);
            return CL_BUILD_PROGRAM_FAILURE;
        }
        DEBUG("%s: program build success\n", __func__);
        program->hasBinary = CL_TRUE;
        return CL_SUCCESS;

    }else{
        DEBUG("%s: Failed\n", __func__);
        return CL_BUILD_PROGRAM_FAILURE;
    }
}

cl_int clGetProgramInfo (cl_program program,
        cl_program_info param_name,
        size_t param_value_size,
        void *param_value,
        size_t *param_value_size_ret){
    DEBUG("Entering %s\n", __func__);
    if(program == NULL)
        return CL_INVALID_PROGRAM;
  
    switch(param_name){
        case CL_PROGRAM_REFERENCE_COUNT :
            param_value = (void *)program->refcount;
            return CL_SUCCESS;
            
        case CL_PROGRAM_CONTEXT:
            param_value = (void *)program->context;
            return CL_SUCCESS;
            
        case CL_PROGRAM_NUM_DEVICES:
            param_value = (void *)program->context->num_devices;
            return CL_SUCCESS;
            
        case CL_PROGRAM_DEVICES:
            param_value = (void *)program->context->devices;
            return CL_SUCCESS;
            
        case CL_PROGRAM_SOURCE:
            return CL_OUT_OF_RESOURCES;
            param_value = (void *)program->context->devices;
            return CL_SUCCESS;
            
        default:
            return CL_INVALID_VALUE;
    }
  
  return CL_OUT_OF_RESOURCES;
}

//TODO
cl_int clGetProgramBuildInfo (  cl_program  program,
                                cl_device_id  device,
                                cl_program_build_info  param_name,
                                size_t  param_value_size,
                                void  *param_value,
                                size_t  *param_value_size_ret){
    DEBUG("Entering %s\n", __func__);
    DEBUG("%s: NOT YET IMPLEMENTED\n", __func__);
    
    return CL_OUT_OF_HOST_MEMORY;
}

void cl_program_sanitizeBuildOptions(const char *options, char *safeOptions){
    char *pos;
    char *spos;
    int first = 1;
    pos = options;
    spos = safeOptions;
    
    while(*pos){
        if(*pos == ' ' || first){
            first = 0;
            while(*pos && *pos == ' ') pos++; //skip spaces
            *spos++ = ' ';
            if(0 == memcmp(pos, "-D", 2)){
                continue;
            }
            while(*pos && *pos != ' ') pos++; //skip the option
        }else{
            *spos = *pos;
            spos++;
            pos++;
        }
    }
    *spos = 0;
}


