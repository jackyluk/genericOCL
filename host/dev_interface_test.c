#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "dev_interface.h"



int main(void)
{
    int fd_ctrl, fd_data;
    void* data, *data2, *ctrl;
    char buf[128];

    fd_ctrl = dev_connect(CONN_CTRL);
    fd_data = dev_connect(CONN_DATA);
    if(fd_ctrl == -1 || fd_data == -1)
        return -1;

    dev_set_kernel_args("0 100\n100 10\n110 20\n");

    data = "w 128 62 qwerty";
    data2 = "uiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890";
    dev_write(fd_data, data, strlen(data));
    dev_write(fd_data, data2, strlen(data2));
    dev_read(fd_data, buf, 4);
    printf("%s\n", buf);
    data = "w 256 8 bla";
    data2 = " test";
    dev_write(fd_data, data, strlen(data));
    dev_write(fd_data, data2, strlen(data2));
    dev_read(fd_data, buf, 4);
    printf("%s\n", buf);

    ctrl = "p";
    dev_write(fd_ctrl, ctrl, strlen(ctrl));
    dev_read(fd_ctrl, buf, 4);
    printf("%s\n", buf);

    data = "r 0 13";
    dev_write(fd_data, data, strlen(data));
    dev_read(fd_data, buf, 13);
    printf("%s\n", buf);

    dev_disconnect(fd_ctrl);
    dev_disconnect(fd_data);

    return 0;
}
