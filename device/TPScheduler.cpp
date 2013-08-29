/*!****************************************************************************
 * @file TPScheduler.cpp Thread pool scheduler Implementation
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#include "IScheduler.hpp"
#include "TPScheduler.hpp"
#include "ComputeUnit.hpp"
#include "debug.h"

IScheduler::IScheduler(){}
IScheduler::~IScheduler(){}

TPScheduler::TPScheduler(char *dataPtr){
    int counter;
    
    pthread_mutex_init(&(this->queue_mx), NULL);
    this->data = dataPtr;
    for(counter = 0; counter < COMPUTE_UNIT_ARRAY_SIZE; counter++){
        this->free_cu_array.push(new ComputeUnit(this, counter, this->data));
    }
    memset(this->data, 0, GLOBAL_MEMORY_SIZE);
}

TPScheduler::~TPScheduler(){
    int counter;
    for(counter = 0; counter < COMPUTE_UNIT_ARRAY_SIZE; counter++){
        delete(this->free_cu_array.front());
        this->free_cu_array.pop();
    }
}
    
void TPScheduler::addWork(int globalWS[3]){
    int x; 
    int y;
    int z;
    int counter;
    ComputeUnit *tmp;
    
    for(counter = 0; counter < COMPUTE_UNIT_ARRAY_SIZE; counter++){
        tmp = this->free_cu_array.front();
        this->free_cu_array.pop();
        tmp->set_kernel((char *)"./kernel.so");
        this->free_cu_array.push(tmp);
    }
    
    for(z = 0; z < globalWS[2]; z++){
        for(y = 0; y < globalWS[1]; y++){
            for(x = 0; x < globalWS[0]; x++){
                while(1){
                    pthread_mutex_lock(&(this->queue_mx));
                    if(false == this->free_cu_array.empty()){
                        ComputeUnit *free_cu = this->free_cu_array.front();
                        this->free_cu_array.pop();
                        pthread_mutex_unlock(&(this->queue_mx));
                        free_cu->run_kernel(z, y, x);
                        break;
                    }else{
#if !defined(ENABLE_THREAD_POOL)
                        while(false == this->done_cu_array.empty()){
                            tmp = this->done_cu_array.front();
                            this->done_cu_array.pop();
                            tmp->join();
                            this->free_cu_array.push(tmp);
                        }
#endif
                        pthread_mutex_unlock(&(this->queue_mx));
                    }
                }
            }
        }
    }
    
    //Finally ensure all threads are joined.
    while(1){
        pthread_mutex_lock(&(this->queue_mx));
#if defined(ENABLE_THREAD_POOL)
        if(this->free_cu_array.size() != COMPUTE_UNIT_ARRAY_SIZE){
#else
        if(this->free_cu_array.size() + this->done_cu_array.size() != COMPUTE_UNIT_ARRAY_SIZE){
#endif
            usleep(10);
            pthread_mutex_unlock(&(this->queue_mx));
        }else{
#if !defined(ENABLE_THREAD_POOL)
            while(false == this->done_cu_array.empty()){
                tmp = this->done_cu_array.front();
                this->done_cu_array.pop();
                tmp->join();
                this->free_cu_array.push(tmp);
            }
#endif
            pthread_mutex_unlock(&(this->queue_mx));
            break;
        }
    }
    
    for(counter = 0; counter < COMPUTE_UNIT_ARRAY_SIZE; counter++){
        tmp = this->free_cu_array.front();
        this->free_cu_array.pop();
        tmp->unset_kernel();
        this->free_cu_array.push(tmp);
    }

    for(x = 0; x < 128; x++){
        DEBUG("%d ", ((int *)(this->data))[x]);
    }
    DEBUG("\n");
}

void TPScheduler::CUDone(ComputeUnit *free_cu){
    pthread_mutex_lock(&(queue_mx));
#if defined(ENABLE_THREAD_POOL)
    this->free_cu_array.push(free_cu);
#else
    this->done_cu_array.push(free_cu);
#endif
    pthread_mutex_unlock(&(this->queue_mx));
}

