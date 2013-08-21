/*!****************************************************************************
 * @file TPScheduler.hpp Threadpool scheduler Definitions
 * @author Jacky H T Luk 2013
 *****************************************************************************/

#include "IScheduler.hpp"
#include "ComputeUnit.hpp"
#include "GlobalDef.hpp"
#include <pthread.h>

#include <queue>
#include <deque>

#define COMPUTE_UNIT_ARRAY_SIZE 128

class TPScheduler: public IScheduler{
    int globalWS[3];
    std::queue<ComputeUnit *> free_cu_array;
    std::queue<ComputeUnit *> done_cu_array;
    pthread_mutex_t queue_mx;
    char *data;
public:
    
    TPScheduler(char *dataPtr);
    
    void addWork(int globalWS[3]);
    
     void CUDone(ComputeUnit *free_cu);
    
    virtual ~TPScheduler();
};
