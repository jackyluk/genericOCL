// Keep Eclipse happy
#ifdef __CDT_PARSER__
#define __global
#define __local
#define __private
#define __kernel
#endif

__kernel void matrix_mul (
    __global int *mA,
    __global int *mB,
    __global int *mC) {
    unsigned int mWidth = 16;
    unsigned int x=get_global_id(0);
    unsigned int y=get_global_id(1);
    int elt=0;
    for (unsigned int i=0;i<mWidth;i++) {    
        elt+=mA[y*mWidth+i]*mB[i*mWidth+x];
    }
    mC[x+mWidth*y]=elt;

}

