#include "ocl.h"
#include <fstream>
#include <sys/types.h>
#include <stdlib.h>

#define WIDTH 16


int main () {
	cl_int err;
    const uint mSize = WIDTH*WIDTH;
    const uint mWidth = WIDTH;

    // Create the data sets   
    cl_int* mA=(cl_int*)malloc(sizeof(cl_int)*mSize);
    cl_int* mB=(cl_int*)malloc(sizeof(cl_int)*mSize);

    for(unsigned int i = 0; i < mSize; i++) {
        mA[i] = i+1;
        mB[i] = i+1;
    }

    cl_int* mCref=(cl_int*)malloc(sizeof(cl_int)*mSize);
    cl_int mArow[WIDTH];

    for (uint i = 0; i<mWidth; i++) {
    	// This is an attempt to put a row in the cache.
    	// It sometimes works, giving a speed-up of 5x
    	for (uint j = 0; j<mWidth; j++) {
    		mArow[j]=mA[i*mWidth+j];
    	}
        for (uint j = 0; j<mWidth; j++) {
            cl_int elt=0;
            for (uint k = 0; k<mWidth; k++) {
            	elt+=mArow[k]*mB[k*mWidth+j];
            }
            mCref[i*mWidth+j]=elt;
        }
    }

    cl_int* mC=(cl_int*)malloc(sizeof(cl_int)*mSize);

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
    std::ifstream file("matrix_mul.cl");
    checkErr(file.is_open() ? CL_SUCCESS:-1, "matrix_mul.cl");

    std::string prog(
        std::istreambuf_iterator<char>(file),
        (std::istreambuf_iterator<char>())
        );

    cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length()+1));

    // Create program and kernel
    cl::Program program(context, source);
    cl::Kernel kernel(program, "matrix_mul", &err);
    checkErr(err, "Kernel::Kernel()");

    // Set the Kernel arguments
    err = kernel.setArg(0, sizeof(cl_int) * mSize, NULL);
    err = kernel.setArg(1, sizeof(cl_int) * mSize, NULL);
    err = kernel.setArg(2, sizeof(cl_int) * mSize, NULL);
    checkErr(err, "Kernel::setArg()");

    err = program.build(devices,"");
    checkErr(file.is_open() ? CL_SUCCESS : -1, "Program::build()");

    // Create Buffers
    cl::Buffer mA_buf(
    		context,
            CL_MEM_READ_ONLY,
            sizeof(cl_int) * mSize,
            mA,
            &err);
    checkErr(err, "Buffer::Buffer input()");

    cl::Buffer mB_buf(
    		context,
            CL_MEM_READ_ONLY,
            sizeof(cl_int) * mSize,
            mB,
            &err);
    checkErr(err, "Buffer::Buffer input()");

    cl::Buffer mC_buf(
    		context,
            CL_MEM_WRITE_ONLY,
            sizeof(cl_int) * mSize,
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
        sizeof(cl_int)*mSize,
        mA);
    checkErr(err, "Buffer::enqueueWriteBuffer()");

    err = queue.enqueueWriteBuffer(
        mB_buf,
        CL_TRUE,
        0,
        sizeof(cl_int)*mSize,
        mB);
    checkErr(err, "Buffer::enqueueWriteBuffer()");

    // Enqueue the Kernel
    cl::Event event;
    err = queue.enqueueNDRangeKernel(
        kernel,
        cl::NullRange,                  // global offset (none)
        cl::NDRange(mWidth,mWidth),     // global work size (2-dimensional: mWidth x mWidth)
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
        sizeof(cl_int)*mSize,
        mC);
    checkErr(err, "Buffer::enqueueReadBuffer()");

    queue.finish();
    // Check the multiplication
    unsigned int correct=0;               // number of correct results returned
    for (unsigned int i = 0; i < mSize; i++){
        //std::cerr << mC[i] << " : " << mCref[i] << std::endl;
    	int reldiff = mC[i] - mCref[i];
        if(reldiff==0){
            correct++;
        }else{
          std::cout << "diff " << i << std::endl;
        }
    }
    free(mCref);
    std::cout << "Computed '"<<correct<<"/"<<mSize<<"' correct values!\n";

    free(mA);
    free(mB);
    free(mC);

    // Exit
    return EXIT_SUCCESS;
}

inline void checkErr(cl_int err, const char * name) {
	if (err != CL_SUCCESS) {
		std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
		exit( EXIT_FAILURE);
	}
}
