/*!****************************************************************************
 * @file Device.cpp Device class
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#include "Device.hpp"
#include "ControlLink.hpp"
#include "TPScheduler.hpp"

/*!****************************************************************************
 * @brief Constructor
 * @param port The port the device shall listen on.
 * ***************************************************************************/
Device::Device(int port){
    this->data = new char[GLOBAL_MEMORY_SIZE]();
    this->controller = new ControlLink(this);
    this->scheduler = new TPScheduler(this->data);
    this->port = port;
}

/*!****************************************************************************
 * @brief Destructor
 * ***************************************************************************/
Device::~Device(){
    delete this->controller;
    delete this->scheduler;
    delete this->data;
}

/*!****************************************************************************
 * @brief Start device. This must be called for the device to start 
 *        functioning.
 * ***************************************************************************/
void Device::start(){
    if(this->controller->init(this->port) < 0){
      throw -1;
      return;
    }
    printf("pPort is %d\n", this->port);
    this->controller->start();
}

/*!****************************************************************************
 * @brief Wait for device thread to complete. This call blocks until the 
 *        device has completed serving one opencl execution.
 * ***************************************************************************/
void Device::join(){
    this->controller->join();
}

/*!****************************************************************************
 * @brief Device memory dump.
 * ***************************************************************************/
int Device::memdump(){
    unsigned int len, i;

    pthread_mutex_lock(&data_mx);
    len = GLOBAL_MEMORY_SIZE;
    for(i=0; i<len; i++){
        if(i%16==0) fprintf(stderr, "\n");
        if(data[i] == '\n')
            fprintf(stderr, "\\N ");
        else if(data[i] == '0')
            fprintf(stderr, ".. ");
        else
            fprintf(stderr, "%2x ", data[i]);
    }
    fprintf(stderr, "\n");
    pthread_mutex_unlock(&data_mx);
}