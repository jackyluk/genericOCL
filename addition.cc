#include "ocl.h"
#include <fstream>
#include <sys/types.h>
#include <stdlib.h>

#define WIDTH 8


int main () {
	cl_int err;
    const uint mWidth = WIDTH;

    // Create the data sets   
    cl_int mA[] = {0,1,2,3,4,5,6,7};
    cl_int mB[] = {7,6,5,4,3,2,1,0};
    cl_int mC[] = {0,0,0,0,0,0,0,0};
    

	// Create the Platform
	cl::vector<cl::Platform> platformList;
	cl::Platform::get(&platformList);
	checkErr(platformList.size() != 0 ? CL_SUCCESS : -1, "cl::Platform::get");

	// Create the Context
	cl_context_properties cprops[3] = { CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platformList[0])(), 0 };
	cl::Context context(CL_DEVICE_TYPE_DEFAULT, cprops, NULL, NULL, &err);
    checkErr(err, "Context::Context()");

    // Find the Devices
    cl::vector<cl::Device> devices;
    devices = context.getInfo<CL_CONTEXT_DEVICES>();
    checkErr( devices.size() > 0 ? CL_SUCCESS : -1, "devices.size() > 0");

    // Load the Kernel
    std::ifstream file("int_sum.cl");
    checkErr(file.is_open() ? CL_SUCCESS:-1, "int_sum.cl");

    std::string prog(
        std::istreambuf_iterator<char>(file),
        (std::istreambuf_iterator<char>())
        );

    cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));

    // Create program and kernel
    cl::Program program(context, source);
    cl::Kernel kernel(program, "int_sum", &err);
    checkErr(err, "Kernel::Kernel()");

    // Set the Kernel arguments
    err = kernel.setArg(0, sizeof(cl_int) * mWidth, NULL);
    err = kernel.setArg(1, sizeof(cl_int) * mWidth, NULL);
    err = kernel.setArg(2, sizeof(cl_int) * mWidth, NULL);
    checkErr(err, "Kernel::setArg()");

    err = program.build(devices,"");
    checkErr(file.is_open() ? CL_SUCCESS : -1, "Program::build()");

    // Create Buffers
    cl::Buffer mA_buf(
    		context,
            CL_MEM_READ_ONLY,
            sizeof(cl_int) * mWidth,
            &mA,
            &err);
    checkErr(err, "Buffer::Buffer input()");

    cl::Buffer mB_buf(
    		context,
            CL_MEM_READ_ONLY,
            sizeof(cl_int) * mWidth,
            &mB,
            &err);
    checkErr(err, "Buffer::Buffer input()");

    cl::Buffer mC_buf(
    		context,
            CL_MEM_WRITE_ONLY,
            sizeof(cl_int) * mWidth,
            NULL,
            &err);
    checkErr(err, "Buffer::Buffer output()");

    // Create the Command Queue
    cl::CommandQueue queue(context, devices[0], 0, &err);
    checkErr(err, "CommandQueue::CommandQueue()");

    // Enqueue buffer writes
    err = queue.enqueueWriteBuffer(
        mA_buf,
        CL_TRUE,
        0,
        sizeof(cl_int)*mWidth,
        mA);
    checkErr(err, "Buffer::enqueueWriteBuffer()");

    err = queue.enqueueWriteBuffer(
        mB_buf,
        CL_TRUE,
        0,
        sizeof(cl_int)*mWidth,
        mB);
    checkErr(err, "Buffer::enqueueWriteBuffer()");

    // Enqueue the Kernel
    cl::Event event;
    err = queue.enqueueNDRangeKernel(
        kernel,
        cl::NullRange,                  // global offset (none)
        cl::NDRange(mWidth),            // global work size (1-dimensional: mWidth)
        cl::NullRange,                  // local work size (none)
        NULL,
        &event);
    checkErr(err, "CommandQueue::enqueueNDRangeKernel()");

    // Wait for the Kernel to finish
    event.wait();

    // Read the result
    err = queue.enqueueReadBuffer(
        mC_buf,
        CL_TRUE,
        0,
        sizeof(cl_int)*mWidth,
        mC);
    checkErr(err, "Buffer::enqueueReadBuffer()");
    
    queue.finish();

    // Display the result
    std::cout << "Result: " << std::endl << "expected: 7 7 7 7 7 7 7 7" << std::endl << "actual  : ";
    for(int i=0; i<8; i++){
        std::cout << ""<<mC[i]<<" ";
    }
    std::cout << std::endl;

    // Exit
    return EXIT_SUCCESS;
}

inline void checkErr(cl_int err, const char * name) {
	if (err != CL_SUCCESS) {
		std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
		exit( EXIT_FAILURE);
	}
}
