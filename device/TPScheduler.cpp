/*!****************************************************************************
 * @file TPScheduler.cpp Thread pool scheduler Implementation
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#include "IScheduler.hpp"
#include "TPScheduler.hpp"
#include "ComputeUnit.hpp"

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
    
    for(z = 0; z < globalWS[2]; z++){
        for(y = 0; y < globalWS[1]; y++){
            for(x = 0; x < globalWS[0]; x++){
                while(1){
                    pthread_mutex_lock(&(this->queue_mx));
                    if(false == this->free_cu_array.empty()){
                        ComputeUnit *free_cu = this->free_cu_array.front();
                        this->free_cu_array.pop();
                        pthread_mutex_unlock(&(this->queue_mx));
                        free_cu->set_kernel("./kernel.so");
                        free_cu->run_kernel(z, y, x);
                        break;
                    }else{
                        pthread_mutex_unlock(&(this->queue_mx));
                    }
                }
            }
        }
    }
    while(1){
      pthread_mutex_lock(&(this->queue_mx));
      if(COMPUTE_UNIT_ARRAY_SIZE == this->free_cu_array.size()){
        pthread_mutex_unlock(&(this->queue_mx));
        break;
      }
      pthread_mutex_unlock(&(this->queue_mx));
    }
}

void TPScheduler::CUDone(ComputeUnit *free_cu){
    pthread_mutex_lock(&(queue_mx));
    this->free_cu_array.push(free_cu);
    pthread_mutex_unlock(&(this->queue_mx));
}

