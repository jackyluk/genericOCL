/*!****************************************************************************
 * @file cl_defs.c OpenCL definitions
 * @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
 *****************************************************************************/
#ifndef CL_DEFS_H
#define CL_DEFS_H


/* python 2.x */
#define PYTHON "python2.7"




/* Platform */
extern struct _cl_platform_id platformID_0;
#define PF_ID &platformID_0
#define PF_NAME "Prototype Intel"
#define PF_VENDOR "University of Glasgow"
#define PF_PROFILE "EMBEDDED_PROFILE"
#define PF_VERSION "OpenCL 1.1 FPGA"
#define PF_EXTENSIONS "None"

/* Device */
#define DV_TYPE CL_DEVICE_TYPE_ACCELERATOR
#define DV_VENDOR "None"
#define DV_NAME "Mora array"
#define PREFERRED_WORK_GROUP_SIZE 64 //Some number.

#define MAX_MEM_ALLOC_SIZE 4*1024 //Some number.

#define MAX_WORK_ITEM_SIZES 256 //Some number.
#define MAX_WORK_GROUP_SIZE (MAX_WORK_ITEM_SIZES * MAX_WORK_ITEM_SIZES * MAX_WORK_ITEM_SIZES)

#include <pthread.h>
#include "dev_interface.h"
#include <time.h>


#define EVENT_EXPIRY_TIME_S 60
#define CL_CUSTOM_COMMAND_BARRIER 0x1300


/** Platform ID format */
struct _cl_platform_id{
    char *name;
    char *vendor;
    char *profile;
    char *version;
    char *extensions;
    int num_devices;
    cl_device_id devices[1];
};


/** Implementation of cl_device_id */
struct _cl_device_id{
    cl_device_type type;
    char *vendor;
    char *name; 
    cl_bool connected;
    int fd_ctrl;
};


/** Implementation of cl_context */
struct _cl_context{
    cl_uint refcount;
    cl_uint num_devices;
    const cl_device_id *devices;
    cl_context_properties props[3];
    unsigned int mem_alloc_offset;
};

/** Internal Implementation of Command queue Linked list */
typedef struct QueueCommand_t{
    struct QueueCommand_t *next;        /*! Pointer to the next command */
    cl_int eventStatus;                 /*! Status of the event */
    cl_command_type commandType;        /*! Command Type */
    void *payload;                      /*! Pointer to memory containing payload for the command.*/
    void *ret;                          /*! Return pointer */
    time_t completionTime;              /*! The time the command was completed */
} QueueCommand;

/** Implementation of cl_command_queue */
struct _cl_command_queue{
    cl_uint refcount;
    cl_context context;
    cl_device_id device;
    cl_command_queue_properties props;
    pthread_t queue_thread;
    pthread_mutex_t queue_mutex;
    QueueCommand *queue;
    QueueCommand *currentEvent;
};


/** Implementation of cl_mem */
struct _cl_mem{
    cl_uint refcount;
    size_t offset;
    size_t size;
};


/** Implementation of cl_kernel */
struct _cl_kernel{
    cl_uint refcount;
    char* lib_name;
    char* func_name;
    size_t args[32];
    void* argv[32];
    unsigned int arg_count;
};

struct _cl_source{
    cl_uint count;
    char **strings;
    size_t *lengths;
};

/** Implementation of cl_program */
struct _cl_program{
    cl_uint refcount;
    cl_context context;
    struct _cl_source source;
    cl_bool createdWithBinary;
    cl_bool hasBinary;
};



/** Implementation of cl_event */
struct _cl_event{
    cl_uint refcount;
};

typedef struct ND_Kernel_Cmd_Params_t {
    GlobalWorkSize_t globalWorkSize;
    cl_kernel kernel;
} ND_Kernel_Cmd_Params;

void queue_dispatchCommand(cl_command_queue command_queue, QueueCommand *command);
void dispatchNDRangeKernel(int fd, QueueCommand *command);
char* get_kernel_args(cl_kernel kernel);
QueueCommand* queue_add(cl_command_queue command_queue);
QueueCommand* queue_head(cl_command_queue command_queue);

#endif /* CL_DEFS_H */
