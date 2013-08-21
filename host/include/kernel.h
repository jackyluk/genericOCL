/* definitions needed to compile OpenCL kernels using a C compiler */

#include <stdlib.h>

#define __global
#define global
#define __local
#define __private
#define __kernel
#define kernel
#define __constant const
#define __attribute__(x) 
#define gentype float
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
typedef unsigned int uint;

#define max(x,y) ((x >= y)?x:y)

extern size_t get_global_id(unsigned int dimindex);
extern size_t get_global_size(unsigned int dimindex);

extern size_t get_local_id(unsigned int dimindex);
extern size_t get_local_size(unsigned int dimindex);

extern size_t get_group_id(unsigned int dimindex);
extern size_t get_num_groups(unsigned int dimindex);

uint rotate(uint i, uint j){
	return (i << j) | i >> (32 - j);
}

uint bitselect(uint a, uint b, uint c){
	return (a & ~c) | (b & c);
}
