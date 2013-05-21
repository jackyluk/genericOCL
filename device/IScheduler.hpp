/*!****************************************************************************
 * @file IScheduler.hpp Scheduler Interface definition
 * @author Jacky H T Luk 2013
 *****************************************************************************/

#if !defined(ISCHEDULER_HPP)
#define ISCHEDULER_HPP
class ComputeUnit;

class IScheduler{
    
public:
    IScheduler();
    
    virtual void addWork(int globalWS[3]) = 0;
    
    virtual void CUDone(ComputeUnit *free_cu) = 0;
    
    virtual ~IScheduler() = 0;
};
#endif //IScheduler
