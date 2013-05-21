#include "ocl.h"
#include <fstream>
#include <stdlib.h>

int main () {
    const int strSz=14;
	cl_int err;

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
    std::ifstream file("hello_world.cl");
    checkErr(file.is_open() ? CL_SUCCESS:-1, "hello_world.cl");

    std::string prog(
        std::istreambuf_iterator<char>(file),
        (std::istreambuf_iterator<char>())
        );

    cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));

    // Create program and kernel
    cl::Program program(context, source);
    cl::Kernel kernel(program, "hello_world", &err);
    checkErr(err, "Kernel::Kernel()");

    // Set the Kernel arguments
    err = kernel.setArg(0, strSz, NULL);
    checkErr(err, "Kernel::setArg()");

    // Build the kernel
    err = program.build(devices,"");
    checkErr(file.is_open() ? CL_SUCCESS : -1, "Program::build()");

    // Create Buffers
    cl::Buffer str_buf(
	        context,
	        CL_MEM_WRITE_ONLY,
	        strSz,
	        NULL,
	        &err);
	checkErr(err, "Buffer::Buffer()");


    // Create the Command Queue
    cl::CommandQueue queue(context, devices[0], 0, &err);
    checkErr(err, "CommandQueue::CommandQueue()");

    // Enqueue the Kernel
    cl::Event event;
    err = queue.enqueueNDRangeKernel(
        kernel,
        cl::NullRange,                  // global offset (none)
        cl::NDRange(1),                 // global work size (1-dimensional with size 1)
        cl::NullRange,                  // local work size (none)
        NULL,
        &event);
    checkErr(err, "CommandQueue::enqueueNDRangeKernel()");

    // Wait for the Kernel to finish
    event.wait();
    // Read back the result
    char* str=(char*)malloc(strSz);
    err = queue.enqueueReadBuffer(
        str_buf,
        CL_TRUE,
        0,
        strSz,
        str);
    
    //Wait for result
    queue.finish();
    // Display the result
    std::cout << "<"<<str<<">" << std::endl;
    // Exit

    return EXIT_SUCCESS;
}

inline void checkErr(cl_int err, const char * name) {
	if (err != CL_SUCCESS) {
		std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
		exit( EXIT_FAILURE);
	}
}
