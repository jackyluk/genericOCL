/*!****************************************************************************
 * @file ComputeUnit.cpp ComputeUnit implementation
 * @author Jacky H T Luk 2013
 *****************************************************************************/

#include "ComputeUnit.hpp"
#include "debug.h"

/*!****************************************************************************
 * @brief Compute Unit constructor
 * @param parent Reference to an IScheduler
 * @param designation ComputeUnit Unique ID
 * @param dataPtr Pointer to data memory.
 *****************************************************************************/
ComputeUnit::ComputeUnit(IScheduler *parent, int designation, 
                         char *dataPtr){
    this->data = dataPtr;
    this->parent = parent;
    this->thread = 0;
    this->threadAllocated = false;
    this->designation = designation;
    this->dlHandle = NULL;
#if defined(ENABLE_THREAD_POOL)
    pthread_mutex_init(&(this->cuState_mx), NULL);
    pthread_cond_init(&(this->cuState_cond), NULL);
    pthread_create(&(this->thread), NULL, cu_thread_start, this);
#endif
}

/*!****************************************************************************
 * @brief Set kernel to be executed
 * @param lib_name Kernel file name
 *****************************************************************************/
void ComputeUnit::set_kernel(char *lib_name){
    char* error;
    
    if(this->dlHandle){
      dlclose(this->dlHandle);
        error = dlerror();
        if (error) {
            DEBUG("%s\n", error);
            exit(EXIT_FAILURE);
        }
      DEBUG("Closing handle for %d\n", this->designation);
      this->pfnKernelWrapper = NULL;
      this->dlHandle = NULL;
    }
    
    /* link with kernel compiled as shared library */
    //fprintf(stderr, "Opening %s\n", lib_name);
    this->dlHandle = dlopen( lib_name, RTLD_NOW);
    error = dlerror();
    if (error) {
        DEBUG("%s\n", error);
        exit(EXIT_FAILURE);
    }
    this->pfnKernelWrapper = (void (*)(int, int, int, void *))dlsym(this->dlHandle, "kernel_wrapper");
    dlerror(); /* clear previous error */
    if ((error = dlerror()) != NULL)  {
        DEBUG("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
}

/*!****************************************************************************
 * @brief Unset kernel 
 *****************************************************************************/
void ComputeUnit::unset_kernel(){
    char* error;
    
    if(this->dlHandle){
      dlclose(this->dlHandle);
        error = dlerror();
        if (error) {
            DEBUG("%s\n", error);
            exit(EXIT_FAILURE);
        }
      DEBUG("Closing handle for %d\n", this->designation);
      this->pfnKernelWrapper = NULL;
      this->dlHandle = NULL;
    }
}


/*!****************************************************************************
 * @brief Start kernel execution
 * @param z 3rd dimension of execution
 * @param y 2nd dimension of execution
 * @param x 1st dimension of execution
 *****************************************************************************/
void ComputeUnit::run_kernel(int z, int y, int x){
    char* error;
    this->globalX = x;
    this->globalY = y;
    this->globalZ = z;

    //Start the thread
    DEBUG("Calling Designation %d, Instance %d:%d:%d\n", this->designation, z, y, x);
#if !defined(ENABLE_THREAD_POOL)
    this->join();
    this->threadAllocated = true;
    pthread_create(&(this->thread), NULL, cu_thread_start, this);
#else
    pthread_mutex_lock(&(this->cuState_mx));
    this->threadAllocated = true;
    pthread_cond_signal(&(this->cuState_cond));
    pthread_mutex_unlock(&(this->cuState_mx));
#endif
}

/*!****************************************************************************
 * @brief Start of thread function, required for pthread
 * @param arg Pointer to an instance of ComputeUnit
 *****************************************************************************/
void *ComputeUnit::cu_thread_start(void *arg){
  return ((ComputeUnit *)arg)->cu_thread();
}

/*!****************************************************************************
 * @brief CU Thread
 *****************************************************************************/
void* ComputeUnit::cu_thread(){
#if defined(ENABLE_THREAD_POOL)
  while(1){
    pthread_mutex_lock(&(this->cuState_mx));
    while(this->threadAllocated == false){
        pthread_cond_wait(&(this->cuState_cond), &(this->cuState_mx));
    }
    pthread_mutex_unlock(&(this->cuState_mx));
    this->pfnKernelWrapper(this->globalX, this->globalY, this->globalZ, this->data);
    this->threadAllocated = false;
    DEBUG("%d %d:%d:%d EXD\n", this->designation, this->globalZ, this->globalY, this->globalX);
    this->parent->CUDone(this);
  }
#else
  this->pfnKernelWrapper(this->globalX, this->globalY, this->globalZ, this->data);
  DEBUG("%d %d:%d:%d EXD\n", this->designation, this->globalZ, this->globalY, this->globalX);
  this->parent->CUDone(this);
#endif
}

/*!****************************************************************************
 * @brief Compute Unit Join
 *****************************************************************************/
void ComputeUnit::join(){
#if !defined(ENABLE_THREAD_POOL)
    if(this->threadAllocated == true){
        pthread_join(this->thread, NULL);
        this->threadAllocated = false;
    }
#endif
}

/*!****************************************************************************
 * @brief Compute Unit Destructor
 *****************************************************************************/
ComputeUnit::~ComputeUnit(){
  if(this->dlHandle){
    dlclose(this->dlHandle);
    this->dlHandle = NULL;
  }
}
