#ifndef __CLIENT_H
#define __CLIENT_H

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
#include <pthread.h>
#include <thread>
#include <stdlib.h>
#include <sys/shm.h>

#include <stdio.h>
#include <string.h>

//定义共享内存头结构体  
struct SharedMemoryHeader {  
    size_t height;
    size_t width;
    pthread_mutex_t sm_mutex;
    
    size_t dataSize; // 数据大小
};

int pic_send(const char *shm_name_,  const char* des_ip_addr_, int des_ip_port_);

#endif