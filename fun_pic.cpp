#include "commen_struct.h"

int fun_pic(void *shared_memory_ptr, size_t shm_size)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    // 从共享内存中读取并处理图片数据
    SharedMemoryHeader* header = reinterpret_cast<SharedMemoryHeader*>(shared_memory_ptr);  
    size_t data_size = header->dataSize;
    char buffer[data_size];
    memcpy(buffer, shared_memory_ptr + sizeof(SharedMemoryHeader), data_size);

    char filename[32];
    int file_cnt = 0;
    
    sprintf(filename, "video_raw_%04d.jpg", file_cnt);
    int fd_file = open(filename ,O_RDWR | O_CREAT, 0666);
    if(fd_file < 0){
        printf("cannot create file;\n");
    }
    printf("capture to %s \n", filename);
    write(fd_file, buffer, data_size);
    close(fd_file);

    std::cout << "Read" << data_size <<  "byte from shared memory." << std::endl;  

    return 0;
}

