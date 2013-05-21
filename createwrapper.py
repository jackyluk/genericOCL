from string import split
import sys



# return a list of arguments in the format [ [offset, size], ... ]
# @param filename file containing the arguments
#
def getKernelArgs(filename):
    arglist = []
    f = open(filename,'r')
    for line in f:
        arg = [0,0]
        line = line[:-1]
        line = split(line,' ')
        arg[0] = str( line[0] )
        arg[1] = str( line[1] )
        arglist = arglist + [arg]
    f.close()
    return arglist



# return a string representing the kernel wrapper code
# @param arglist a list of arguments returned by getKernelArgs
# @param name string representing the name of the kernel function
# @param rangeX x-axis length of global work space
# @param rangeY y-axis length of global work space
#
def generateWrapper(arglist, name, rangeX, rangeY):
    #global[] is an array of counters for each dimension
    #get_global_id() returns the current counter value for a given dimension
    string = "unsigned int global[] = {0,0,0};\nint get_global_id(unsigned int dimindex){\nreturn (dimindex < 2)?global[dimindex]:0;\n}\n"
    string = string + "int get_group_id(unsigned int dimindex){\nreturn 0;\n}\n"
    string = string + "int get_local_id(unsigned int dimindex){\nreturn 0;\n}\n"
    i = 0
    args = []

    #wrapper code which specifies each kernel arguments as an offset in mem
    string = string + "void kernel_wrapper (int x, int y, int z, void* mem) {\n"
    string = string + "global[0] = x; global[1] = y; global[2] = z;\n"
    for a in arglist:
        string = string + "void* arg"+str(i)+" = mem+"+str(int(a[0]) + 0)+";\n"
        args = args + ["arg"+str(i)]
        i = i + 1
    #nested loops which execute the kernel multiple times to traverse the work space (assumes one kernel execution per work item)
    string = string + name+"(" + ",".join(args) + ");\n"
    string = string + "}"
    return string



#main
#

if len(sys.argv) == 4:
    kernel_name = sys.argv[1]
    global_x = sys.argv[2]
    global_y = sys.argv[3]
else:
    print "arguments need to be of the form: kernel_name work_size_x work_size_y"
    exit()

myargs = getKernelArgs("kernelargs")
out = open("kernelwrapper.c", 'w')
out.write(generateWrapper(myargs, kernel_name, global_x, global_y))
out.close()
