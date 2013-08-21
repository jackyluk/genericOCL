/*!****************************************************************************
 * @file ComputeUnit.hpp ComputeUnit Definitions
 * @author Jacky H T Luk 2013
 *****************************************************************************/

#if !defined(COMPUTEUNIT_HPP)
#define COMPUTEUNIT_HPP
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <dlfcn.h>
#include "IScheduler.hpp"

typedef void (*pfnKernelWrapper_t)(int x, int y, int z, void* mem);

class ComputeUnit{
    char *data;
    pthread_mutex_t cuState_mx;
    int globalX, globalY, globalZ;
    void* dlHandle;
    IScheduler *parent; 
    pthread_t thread;
    pfnKernelWrapper_t pfnKernelWrapper;
    
private:
    static void* cu_thread_start(void *arg); 
    void* cu_thread(); 
public:
    int designation;
    ComputeUnit(IScheduler *parent, int designation, char *dataPtr);
    ~ComputeUnit();
    void set_kernel(char *lib_name);
    void run_kernel(int x, int y, int z);
    void join();
};

#endif //COMPUTEUNIT_HPP