-----------------------------------------------

 A Generic OpenCL Implementation to Support Novel Architectures

 Author: Hau Tat Luk
 Date: 30 August 2013

 Supervisor: Wim Vanderbauwhede

-----------------------------------------------

 CONTENTS:

 host/             | OpenCL API implementation

 device/           | device application

 deviceProxy/      | TCP proxy program (optional)

 demos/addition    | Addition example

 demos/helloworld  | Hello world example

 demos/matrix      | Matrix multiplication example

 demos/cgminer     | Cgminer bitcoin miner

-----------------------------------------------

 INSTRUCTIONS

 The included Makefile in the top level folder
 should be used to build the device simulator, the host runtime, 
 the demo programs (except for cgminer):

 $ make
 
 At the end of the host runtime build, the build system will suggest
 the correct value to set environment variable NOVELCLSDKROOT to.
 The host is built into host/build. This is where NOVELCLSDKROOT needs
 to point, in absolute path.
 Export the variable before building Cgminer.

 To build cgminer, first go into the demos/cgminer.

 $ cd demos/cgminer

 Then run autogen.sh to configure the build system. 

 $ ./autogen.sh

 Once the configuration is complete, run make to build cgminer

 $ make


 Before running any applications, please start the device application, on port 5000.

 $ cd device
 $ ./device 5000
 
 The examples helloworld, addition, and matrix, can be run by just executing the corresponding binaries.


 To run the cgminer bitcoin miner example, you will need an account on a bitcoin mining pool,
 I have used 50btc.com here with an anonymous bitcoin address creditial. 

 $ cd demos/cgminer 
 $ ./cgminer -o http://pool.50btc.com:8332 -O1ALKUbzt6n4LdTANtqXwVeQeUbZk3akXUF:x -v1 -I0


 
 DEPENDENCIES:

 GNU Toolchain
 OpenCL 1.1 headers

 NOTE: Please make sure that the PYTHON macro
       in host/cl_defs.h is set correctly to
       the python 2.x binary present on your
       machine. This is used by the host runtime 
       when invoking kernel build scripts


 Tested on a Linux machine.

-----------------------------------------------
