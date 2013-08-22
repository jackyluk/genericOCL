/*!****************************************************************************
* @file cl_program.c OpenCL program implementation
* @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
*****************************************************************************/
#include "debug.h"
#include <CL/opencl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cl_defs.h"
#include "dev_interface.h"

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
    prog->buildStatus = CL_BUILD_NONE;
    prog->buildOptions = NULL;
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
    cl_program prog;        
    DEBUG("Entering %s\n", __func__);
    if( (prog = (cl_program)malloc(sizeof(struct _cl_program))) == NULL){
        if(binary_status) *binary_status = CL_OUT_OF_HOST_MEMORY;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    prog->refcount = 1; /* implicit retain */
    prog->buildStatus = CL_BUILD_NONE;
    prog->buildOptions = NULL;
    prog->source.count = 0;
    prog->source.strings = NULL;
    
    FILE *binaryFile = fopen("program.o", "wb+");
    if(binaryFile != NULL){
        fwrite(binaries[0], lengths[0], 1, binaryFile);
        prog->binarySize = lengths[0];
        fclose(binaryFile);
    }else{
        free(prog);
        if(binary_status) *binary_status = CL_OUT_OF_HOST_MEMORY;
        if(errcode_ret) *errcode_ret = CL_OUT_OF_HOST_MEMORY;
        return NULL;
    }
    if(binary_status) *binary_status = CL_SUCCESS;
    if(errcode_ret) *errcode_ret = CL_SUCCESS;
    prog->hasBinary = CL_TRUE;
    prog->createdWithBinary = CL_TRUE;
    return prog;
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
        if(program->buildOptions){
            free(program->buildOptions);
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
    if(program == NULL){
        DEBUG("Warning: Program is NULL");
        return CL_INVALID_PROGRAM;
    }
    program->buildStatus = CL_BUILD_IN_PROGRESS;
    
    if(program->createdWithBinary == CL_TRUE){
        program->buildStatus = CL_BUILD_SUCCESS;
        return CL_SUCCESS;
    }
    if(program->buildOptions){
        free(program->buildOptions);
    }
    program->buildOptions = strdup(options);
    program->buildOptionsLen = strlen(options);
    DEBUG("clBuildProgram: buildoptions %s\n", options);
//     cl_program_sanitizeBuildOptions(options, safeOptions);
    DEBUG("clBuildProgram: safe buildoptions %s\n", options);
    FILE *optionsFile = fopen("tmp.options", "wb+");
    if (optionsFile != NULL){
        fwrite("CLFLAGS=\"", 9, 1, optionsFile);
        fwrite(options, strlen(options), 1, optionsFile);
        fwrite("\"", 1, 1, optionsFile);
        fclose(optionsFile);
        optionsFile = NULL;
    }else{
        DEBUG("%s: build options store failed.\n", __func__);
        program->buildStatus = CL_BUILD_ERROR;
        return CL_BUILD_PROGRAM_FAILURE;
    }
    
    FILE *srcFile = fopen("tmp.cl", "wb+");
    
    if (srcFile != NULL){
        int line;
        for(line = 0; line < program->source.count; line++){
            DEBUG("%s: Line length %d\n", __func__, program->source.lengths[line]);
            fwrite(program->source.strings[line], program->source.lengths[line] - 1, 1, srcFile);
        }
        fclose(srcFile);
        
        DEBUG("%s: performing program build\n", __func__);
        snprintf(cmd, 256, "$NOVELCLSDKROOT/scripts/buildprogram.sh tmp.cl "); 
        DEBUG("RUN: %s", cmd);
        if(system(cmd)){
            DEBUG("%s: program build error\n", __func__);
            program->buildStatus = CL_BUILD_ERROR;
            return CL_BUILD_PROGRAM_FAILURE;
        }
        DEBUG("%s: program build success\n", __func__);
        FILE * binaryFile = fopen("program.o", "rb");
        if(binaryFile != NULL){
            fseek(binaryFile, 0L, SEEK_END);
            program->binarySize = ftell(binaryFile);
            fclose(binaryFile);
        }else{
            program->buildStatus = CL_BUILD_ERROR;
            return CL_BUILD_PROGRAM_FAILURE;
        }
        program->hasBinary = CL_TRUE;
        program->buildStatus = CL_BUILD_SUCCESS;
        return CL_SUCCESS;

    }else{
        DEBUG("%s: Failed\n", __func__);
        program->buildStatus = CL_BUILD_ERROR;
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
            
        case CL_PROGRAM_BINARY_SIZES:
            if(param_value != NULL){
                if(program->buildStatus == CL_BUILD_SUCCESS){
                    *(size_t *)param_value = program->binarySize;
                    if(param_value_size_ret) *param_value_size_ret = sizeof(size_t);
                    return CL_SUCCESS;
                }else{
                    return CL_BUILD_PROGRAM_FAILURE;
                }
            }
            break;
        case CL_PROGRAM_BINARIES:
            if(param_value != NULL){
                if(program->buildStatus == CL_BUILD_SUCCESS){
                    FILE * binaryFile = fopen("program.o", "rb");
                    if(binaryFile != NULL){
                        int byteRead;
                        byteRead = fread(((void **)param_value)[0], 1, program->binarySize, binaryFile);
                        fclose(binaryFile);
                        if(byteRead == program->binarySize){
                            if(param_value_size_ret) *param_value_size_ret = program->binarySize;
                            return CL_SUCCESS;
                        }else{
                            return CL_OUT_OF_RESOURCES;
                        }
                    }
                }
            }
            break;
            
        default:
            return CL_INVALID_VALUE;
    }

return CL_OUT_OF_RESOURCES;
}

cl_int clGetProgramBuildInfo (  cl_program  program,
                                cl_device_id  device,
                                cl_program_build_info  param_name,
                                size_t  param_value_size,
                                void  *param_value,
                                size_t  *param_value_size_ret){
    DEBUG("Entering %s\n", __func__);
    
    switch(param_name){
        case CL_PROGRAM_BUILD_STATUS:
            DEBUG("%s: CL_PROGRAM_BUILD_STATUS\n", __func__);
            if(param_value != NULL){
                *(cl_build_status *)param_value = program->buildStatus;
                *param_value_size_ret = sizeof(cl_build_status);
                return CL_SUCCESS;
            }else{
                DEBUG("%s: Warning: param_value is NULL!\n", __func__);
            }
            break;
            
        case CL_PROGRAM_BUILD_OPTIONS:
            DEBUG("%s: CL_PROGRAM_BUILD_OPTIONS\n", __func__);
            if(param_value != NULL){
                if(program->buildOptions != NULL){
                    memcpy((char *)param_value, program->buildOptions, program->buildOptionsLen);
                    if(param_value_size_ret)*param_value_size_ret = program->buildOptionsLen;
                }else{
                    if(param_value_size_ret)*param_value_size_ret = 0;
                }
                return CL_SUCCESS;
            }else{
                DEBUG("%s: Warning: param_value is NULL!\n", __func__);
            }
            break;
        case CL_PROGRAM_BUILD_LOG:
            if(param_value_size_ret)*param_value_size_ret = 0;
            return CL_SUCCESS;
        default:
            return CL_INVALID_VALUE;
    }
    
    
    DEBUG("%s: NOT YET IMPLEMENTED\n", __func__);
    
    return CL_OUT_OF_HOST_MEMORY;
}


