#include "IScheduler.hpp"
#include "ComputeUnit.hpp"

class ScheRoundRobin: public IScheduler {
    int globalWS[3];
    char *data;
public:
    ScheRoundRobin(char *dataPtr);
    
    void addWork(int globalWS[3]);
    
    virtual ~ScheRoundRobin(){}
};
