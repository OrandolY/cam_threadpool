
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <poll.h>
#include <errno.h>
//#include <pthread.h>
#include <thread>

#include <stdio.h>
#include <string.h>

struct SharedMemoryHeader {  
    size_t dataSize; // 数据大小  
};

int fun_pic(void *shared_memory_ptr, size_t shm_size);

int catch_pics(void *shared_memory_ptr, const size_t shm_size);