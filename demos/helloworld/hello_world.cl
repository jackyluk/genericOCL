// Keep Eclipse happy
#ifdef __CDT_PARSER__
#define __global
#define __local
#define __private
#define __kernel
#endif
__kernel void hello_world ( __global char* str) {
    __private const char hello_world_str[14]={'H','e','l','l','o',',',' ','W','o','r','l','d','!','\n'};

    for (int i=0;i<14;i++) 
        str[i]=hello_world_str[i];
}
