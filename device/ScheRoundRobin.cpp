#include "IScheduler.hpp"
#include "ScheRoundRobin.hpp"
#include "ComputeUnit.hpp"

IScheduler::IScheduler(){}

ScheRoundRobin::ScheRoundRobin(char *dataPtr){
    this->data = dataPtr;
}
    
void ScheRoundRobin::addWork(int globalWS[3]){
    int x; 
    int y;
    
    for(y = 0; y < globalWS[1]; y++){
        for(x = 0; x < globalWS[0]; x++){
            ComputeUnit cu(this->data, "./kernel.so");
            cu.run_kernel(0, y, x);
        }
    }
}
