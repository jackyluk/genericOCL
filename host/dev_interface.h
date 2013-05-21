/*!****************************************************************************
 * @file dev_interface.h Host-Device interface and protocol definition
 * @author Jacky H T Luk 2013, Modified from Marcin Bujar's version
 *****************************************************************************/
#ifndef DEV_INTERFACE_H
#define DEV_INTERFACE_H


enum conn_type {CONN_CTRL, CONN_DATA};

#define PACKED_STRUCT __attribute__((packed))
#define MORACL_PROTOCOL_VERSION 1

#define RESET                   0x00
#define MEM_WRITE_CMD           0x01
#define MEM_READ_CMD            0x02
#define MEM_READ_RSP_CMD        0x03
#define LOAD_KERNEL_IMAGE       0x04
#define START_KERNEL            0x05
#define GLOBAL_WORK_SIZE        0x06

#define CTRL_ACK                     0xFE
#define CTRL_NAK                     0xFF

typedef struct {
  uint32_t globalX;
  uint32_t globalY;
  uint32_t globalZ;
} PACKED_STRUCT GlobalWorkSize_t;

typedef struct {
  uint32_t totalSize;
  uint32_t offset;
  uint32_t dataSize;
  uint8_t data[0];
} PACKED_STRUCT LoadKernel_t;

typedef struct {
  uint32_t offset;
  uint32_t accessLength;
  uint8_t data[0];
} PACKED_STRUCT MemReadWrite_t;

typedef struct {
  uint8_t version;
  uint16_t length;
  uint8_t cmdId;
  union {
    MemReadWrite_t write;
    MemReadWrite_t read;
    LoadKernel_t loadkernel;
    GlobalWorkSize_t globalWorkSize;
  } payload;
} PACKED_STRUCT CommPacket_t;
  
/**
 * connect to a device
 * @return file descriptor if connection successful, -1 on error.
 */
int dev_connect(enum conn_type);



/**
 * read from device
 * @param fd file descriptor of the connected device
 * @param buffer allocated array of bytes to which the line will be written
 * @param len length of buffer array
 * @return actual number of bytes read, -1 on error.
 */
ssize_t dev_read(int fd, void* buffer, size_t len);



/**
 * write to device
 * @param fd file descriptor of the connected device
 * @param buffer allocated array of bytes to be written
 * @param len length of buffer array
 * @return actual number of bytes written, -1 on error.
 */
ssize_t dev_write(int fd, void* buffer, size_t len);


/**
 * Set up device kernel arguments (called before kernel compilation)
 * Format: "offset size\n"
 * @param arglist string representing kernel arguments, separated by newlines
 * @return 0 on success, -1 on error.
 */
int dev_set_kernel_args(char* arglist);


/**
 * disconnect from a device
 * @param fd file descriptor of the connected device
 * @return 0 on success, -1 on error.
 */
int dev_disconnect(int fd);


#endif /* DEV_INTERFACE_H */
