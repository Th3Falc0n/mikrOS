#ifndef VFS_H
#define VFS_H

#define RES_EMPTY 0
#define RES_DIR   1
#define RES_MOUNT 2
#define RES_FIFO  3
#define RES_BLOCK 4

struct rs_dir {
  uint32_t res_type;
  void* res_ptr; /*
                  * RES_DIR   -> rs_dir
                  * RES_MOUNT -> rs_drv
                  * RES_FIFO  -> rs_buf
                  * RES_BLOCK -> rs_blk #unsure
                  */
  char* name;
  struct directory* next;
}

struct rs_buf {
  uint32_t size;
  void* data; 
}

struct rs_drv {
}

#endif
