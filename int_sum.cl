__kernel void int_sum (
    __global int *a,
    __global int *b,
    __global int *ans)
{
    int xid = get_global_id(0);
    ans[xid] = a[xid] + b[xid];
}

