/*!****************************************************************************
 * @file core.cpp Entry file
 * @author Jacky H T Luk 2013
 *****************************************************************************/
#include "Device.hpp"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

/*! Prototypes */
void usage(char *name);

int main(int argc, char *argv[])
{

    if(argc < 2){
      usage(argv[0]);
      return -1;
    }
      
    int port = atoi(argv[1]);
    Device device(port);
    
    

    /* start processing thread */
    try{
      while(1){
        
        device.start();

        //pthread_join(proc_thread, NULL);
        device.join();
      }
    }
    catch(...){
      printf("Error starting device.\n");
    }

    return 0;
}

void usage(char *name){
    printf("%s <port>\n", name);
}