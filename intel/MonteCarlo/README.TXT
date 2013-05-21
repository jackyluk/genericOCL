******************************************************************************
**          Intel(R) SDK for OpenCL* Applications XE 2013 Beta              **                                      
**          Monte Carlo method for stock options pricing Sample             **
**                            README file                                   **
******************************************************************************

*****  Table of Contents  *****

  - Overview
  - Software Requirements
  - Sample Directory Content
  - How to Build
  - How to Run
  - Choosing Sample Parameters
  - Understanding the Sample Output
  - Disclaimer and Legal Information


*****  Overview  *****

This sample demonstrates implementation of the Monte Carlo simulation for the
European stock option pricing. Underlying algorithm is an OpenCL* kernel that
unifies three major algorithm components:

  - Mersenne twister - generation of uniformly distributed pseudorandom numbers
  - Box-Muller transform - generation of normally distributed random numbers
  - Option price calculation using Black-Scholes stock pricing model.
The exact Black-Scholes model is implemented as native code on the host for
comparison with the results, generated with Monte Carlo.


*****  Software Requirements  *****

To correctly build and run this sample, the following is required:

  - GNU Make utility (make)
  - GNU C++ compiler (g++)
  - Intel(R) SDK for OpenCL Applications XE 2013 Beta or higher
  - g++ compiler should know location of libOpenCL.so and CL/cl.h files.


*****  Sample Directory Content  *****

All the files necessary for sample build and run reside at the sample directory
(MonteCarlo) and in common directory of the root directory to which you extract
the samples.

The sample directory contains the following files:

  - montecarlo.cpp -- Monte Carlo host side implementation, including the
                      application entry point, validation routine, all OpenCL
                      resources allocation and kernel invocation.

  - cmdoptions.hpp,
    cmdoptions.cpp -- sample command-line parameters definition and checking
                      for correct values based on the OpenCL device capabilities.

  - montecarlo.cl  -- Monte Carlo sample OpenCL kernels, necessary for correct
                      application run.

  - Makefile       -- builds the sample binary.

  - README.TXT     -- instruction on how to build and run sample, information
                      on understanding the sample output.


*****  How to Build  *****

To build this sample, in the current directory (MonteCarlo) call make:

  $ make

After successful build, 'montecarlo' executable binary is created in the
current directory. If build error occurs, refer to the 'Software
Requirements' section.


*****  How to Run  *****

This is a console sample. To run it, you need to run 'montecarlo'
executable in the terminal:

  $ ./montecarlo

The Random Number Generation and option pricing algorithm are implemented
as an OpenCL kernel. Single OpenCL work-item calculates pair of European option
prices – put and call.

Algorithm implemented both for single and double precision floating point
calculations. This specific data type is selected by  --arithmetic command-line
option which can be float (default) and double value.

Global work size is defined by --options command-line option. Default value is
65536.

The kernel contains inner loop over Monte Carlo samples ruled by --samples
command line option. Default samples number is 262144. Inner loop iteration
encapsulates Mersenne twister pseudorandom number generator, Box Muller
transform and Monte Carlo sampling from Black and Scholes equation. All
underlying algorithms are interleaved together in the one inner loop to avoid
function call overheads.

The default work-group size for the OpenCL NDRange call is NULL, which means
that it is up to the OpenCL runtime implementation to choose appropriate
work-group number. You can try to adjust this parameter using the
--work_group_size command-line option to get better performance. The work-group
size of 16 work-items is optimal for the Intel(R) Xeon Phi(tm) coprocessor, as this
size provides maximal granularity (or large enough work-group size to saturate
computing facilities of the Intel Xeon Phi coprocessor) and enables an automatic
16-way vectorization done by the OpenCL runtime compiler. The sample application
prompts you if the work group size choice is not valid.

Host part of the sample initializes three buffers, which contain randomly
generated values for time to maturity, current stock price, and option strike
price. Risk-free and volatility parameters are fixed. All this data is passed
to the OpenCL kernel to calculate the resulting option put and call prices.

The --validation command-line option enables calculation of option prices on
the host side using the Black and Scholes formula. Then these values are used
for comparison with results calculated using the Monte Carlo simulation.
  
By default, the sample searches for OpenCL platform that contains
"Intel" as a substring in platform name. If the application fails to find
such platform, it prints error message and exits. In this case you need
to select platform manually either by name (as a substring) or by
index through the -p command-line option. For example, to select the platform
that contains "Different platform name" as a sub-name, run this command:

  $ ./montecarlo -p "Different platform name"

or to select platform with index 0, run this one:

  $ ./montecarlo -p 0

By default, sample chooses the first device available for the platform
(device with index 0). It can be also selected through the command-line argument
(-d or -t parameters).

You can obtain the list of all available command-line options and information
on each option by calling:

  $ ./montecarlo -h

Setting the right values for options is crucial to have good performance
for a specific OpenCL device.

Running montecarlo without any command-line options (default run) is
equivalent to the following:

  $ ./montecarlo -p Intel -d 0 -a float -s 262144 -o 65536 -g 0 -e 0.1 -i 10

These parameters are selected to provide reasonable performance
for the Intel Xeon Phi coprocessor and the Intel CPU OpenCL devices.

If the Intel Xeon Phi coprocessor OpenCL device is available, you can run on it by
providing one additional option: -t acc which means "device type: accelerator":

  $ ./montecarlo -t acc

Read the following section to select values for sample parameters
depending on the device and data type.


*****  Choosing Sample Parameters  *****

The following set of parameters should be considered as reasonable for the
following devices:

  - For Intel CPU OpenCL Device:

     $ ./montecarlo -p Intel -t cpu -a float -s 262144 -o 65536

     $ ./montecarlo -p Intel -t cpu -a double -s 262144 -o 65536

  - For Intel Xeon Phi coprocessor OpenCL Device

     $ ./montecarlo -p Intel -t acc -a float -s 262144 -o 65536 -g 16

     $ ./montecarlo -p Intel -t acc -a double -s 262144 -o 65536 -g 16


*****  Understanding the Sample Output *****

The following is an example of possible sample output (with validation
enabled):

    $ ./montecarlo --validation
    Platforms (1):
        [0] Intel(R) OpenCL [Selected]
    Devices (2):
        [0]                 Genuine Intel(R) CPU  @ 2.60GHz
        [1] Intel(R) Many Integrated Core Acceleration Card [Selected]
    Build program options: "-D__DO_FLOAT__ -cl-denorms-are-zero
    -cl-fast-relaxed-math -cl-single-precision-constant -DNSAMP=262144"
    Running Monte Carlo options pricing for 65536 options, with 262144 samples
    Size of memory region for each array: 262144 bytes
    Using risk free rate = 0.05 and volatility = 0.2
    Host time: 2.63781 sec.
    Host perf: 24844.8 Options per second
    VALIDATION PASSED
    Host time: 2.6169 sec.
    Host perf: 25043.4 Options per second
    Host time: 2.63238 sec.
    Host perf: 24896.1 Options per second
    Host time: 2.62573 sec.
    Host perf: 24959.1 Options per second
    Host time: 2.66394 sec.
    Host perf: 24601.1 Options per second
    Host time: 2.6682 sec.
    Host perf: 24561.9 Options per second
    Host time: 2.60798 sec.
    Host perf: 25129.1 Options per second
    Host time: 2.62947 sec.
    Host perf: 24923.6 Options per second
    Host time: 2.65792 sec.
    Host perf: 24656.9 Options per second
    Host time: 2.63868 sec.
    Host perf: 24836.7 Options per second

First, the sample outputs all available platforms and marks (look at the
line with [Selected]). Then goes the list of devices for the selected platform.
The selected device is also marked.

Then follows a "Build program options" section which is exact build options
line passed to the clBuildProgram OpenCL call.

Finally the sample calls the kernel several times and for each iteration
it prints two numbers: "Host time" and "Host perf". Host time is the time
measured on host for complete kernel invocation without any data transfer
to or from device. Host perf is a number of modeled options calculated
based on Host time.

In the case when --validation key is set in the command line, validation
procedure prints validation status just after the first kernel iteration.
It looks as "VALIDATION PASSED" if validation succeeded and "VALIDATION FAILED"
otherwise. An example above demonstraits usage of the --validation key.


******************************************************************************

*****  Disclaimer and Legal Information *****

THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

INFORMATION IN THIS DOCUMENT IS PROVIDED IN CONNECTION WITH INTEL
PRODUCTS. NO LICENSE, EXPRESS OR IMPLIED, BY ESTOPPEL OR OTHERWISE,
TO ANY INTELLECTUAL PROPERTY RIGHTS IS GRANTED BY THIS DOCUMENT.
EXCEPT AS PROVIDED IN INTEL'S TERMS AND CONDITIONS OF SALE FOR SUCH
PRODUCTS, INTEL ASSUMES NO LIABILITY WHATSOEVER AND INTEL DISCLAIMS
ANY EXPRESS OR IMPLIED WARRANTY, RELATING TO SALE AND/OR USE OF INTEL
PRODUCTS INCLUDING LIABILITY OR WARRANTIES RELATING TO FITNESS FOR
A PARTICULAR PURPOSE, MERCHANTABILITY, OR INFRINGEMENT OF ANY PATENT,
COPYRIGHT OR OTHER INTELLECTUAL PROPERTY RIGHT. 

A "Mission Critical Application" is any application in which failure
of the Intel Product could result, directly or indirectly, in personal
injury or death. SHOULD YOU PURCHASE OR USE INTEL'S PRODUCTS FOR ANY
SUCH MISSION CRITICAL APPLICATION, YOU SHALL INDEMNIFY AND HOLD INTEL
AND ITS SUBSIDIARIES, SUBCONTRACTORS AND AFFILIATES, AND THE DIRECTORS,
OFFICERS, AND EMPLOYEES OF EACH, HARMLESS AGAINST ALL CLAIMS COSTS,
DAMAGES, AND EXPENSES AND REASONABLE ATTORNEYS' FEES ARISING OUT OF,
DIRECTLY OR INDIRECTLY, ANY CLAIM OF PRODUCT LIABILITY, PERSONAL INJURY,
OR DEATH ARISING IN ANY WAY OUT OF SUCH MISSION CRITICAL APPLICATION,
WHETHER OR NOT INTEL OR ITS SUBCONTRACTOR WAS NEGLIGENT IN THE DESIGN,
MANUFACTURE, OR WARNING OF THE INTEL PRODUCT OR ANY OF ITS PARTS.

Intel may make changes to specifications and product descriptions at
any time, without notice. Designers must not rely on the absence or
characteristics of any features or instructions marked "reserved" or
"undefined". Intel reserves these for future definition and shall have
no responsibility whatsoever for conflicts or incompatibilities arising
from future changes to them. The information here is subject to change
without notice. Do not finalize a design with this information.

The products described in this document may contain design defects or
errors known as errata which may cause the product to deviate from
published specifications. Current characterized errata are available
on request. 

Contact your local Intel sales office or your distributor to obtain the
latest specifications and before placing your product order.

Copies of documents which have an order number and are referenced in
this document, or other Intel literature, may be obtained
by calling 1-800-548-4725, or go to:
http://www.intel.com/design/literature.htm

Intel Corporation is the author of the Materials, and requests that all
problem reports or change requests be submitted to it directly.

Intel Xeon Phi and Xeon are trademarks of Intel Corporation in the U.S.
and/or other countries.

* Other names and brands may be claimed as the property of others.

OpenCL and the OpenCL logo are trademarks of Apple Inc. used by
permission from Khronos.

Copyright (c) 2013 Intel Corporation. All rights reserved.