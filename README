-----------------------------------------------

 A Generic OpenCL Implementation to Support Novel Architectures

 Author: Hau Tat Luk
 Date: 13 May 2012

 Supervisor: Wim Vanderbauwhede

-----------------------------------------------

 CONTENTS:

 host/             | OpenCL API implementation
 device/           | device application
 intel/MonteCarlo  | Intel Monte Carlo simulation for the European stock option pricing

-----------------------------------------------

 INSTRUCTIONS

 The included Makefile in the top level folder
 should be used to build all the necessary
 executables:

 $ make
 
 To install the OpenCL library to /usr/lib:
 $ cd host
 $ sudo make install

 Before running any applications, please start the device application, on port 5000.

 $ cd device
 $ ./device 5000
 
 Then run application by:
 $ cd ..
 $ ./run_host.sh helloworld

 Replace helloworld with sample application of your choice, the others beint addition and matrix
 
 DEPENDENCIES:

 GNU Toolchain
 OpenCL 1.1 headers

 NOTE: Please make sure that the PYTHON macro
       in host/cl_defs.h is set correctly to
       the python 2.x binary present on your
       machine.

 Intel Monte Carlo simulation for the European stock option pricing
 
 The Makefile has been modified to compile with this OpenCL host implementation instead of Intel's.
 To compile:
 $ make -C intel/MonteCarlo

 To run: 
 Make sure device is running, then run:
 $ intel/MonteCarlo/montecarlo -o 128
 
 128 is the number of options to. 


 Tested on a Linux machine.

-----------------------------------------------
