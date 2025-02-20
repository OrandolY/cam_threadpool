#include "commen_struct.h"

int fun_pic(void *shared_memory_ptr, size_t shm_size)
{
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        
        // 从共享内存中读取并处理图片数据
        SharedMemoryHeader* header = reinterpret_cast<SharedMemoryHeader*>(shared_memory_ptr);  
        
        //查询互斥量
        std::cout << "Wait Lock " << std::endl;
        pthread_mutex_lock(&header->sm_mutex);// 阻塞,等待锁释放
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        size_t data_size = header->dataSize;
        char buffer[data_size];
        memcpy(buffer, shared_memory_ptr + sizeof(SharedMemoryHeader), data_size);
        
        // 锁释放
        pthread_mutex_unlock(&header->sm_mutex);

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
        std::cout << "Height:" << header->height << std::endl;
    }
    

    return 0;
}

