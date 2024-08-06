//arm-buildroot-linux-gnueabihf-g++ -std=c++17  -L . -I .  -o pooltest pool_test.cpp -ltdpool -lpthread
#include <iostream>
#include <chrono>
#include <thread>
#include <sys/mman.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

#include "threadpool.h"

#include <stdio.h>
#include <string.h>

#include "server.h"

/*
在部分场景，需要获取线程执行任务的返回值
e.g.
    多线程 流水线操作
    把一个事件拆分处理，最终合并出最终结果；
*/

/*派生类 要 传入参数*/
class Cam_Launch : public Task
{
public:
    Cam_Launch(void *shared_memory_ptr_, const size_t shm_size_)
    :    shared_memory_ptr_(shared_memory_ptr_),
         shm_size_(shm_size_)
    {}
    //Ques1 如何设计run() 使其返回值可以表示任意类型；
    //在JAVA PY 中的Object 是所有其他类类型的基类，所有类都可以从它继承；
    //C++ 17 any类型
    Any run()  // run方法最终就在线程池分配的线程中去做执行了!
    {
        return catch_pics(shared_memory_ptr_, shm_size_);
    }
    //设计到基类private性质
private:
    //char* args_;
    void *shared_memory_ptr_;
    size_t shm_size_;
};

/*派生类 要 传入参数*/
class Cam_Take : public Task
{
public:
    Cam_Take(void *shared_memory_ptr_, const size_t shm_size_)
    :    shared_memory_ptr_(shared_memory_ptr_),
         shm_size_(shm_size_)
    {}
    //Ques1 如何设计run() 使其返回值可以表示任意类型；
    //在JAVA PY 中的Object 是所有其他类类型的基类，所有类都可以从它继承；
    //C++ 17 any类型
    Any run()  // run方法最终就在线程池分配的线程中去做执行了!
    {
        return fun_pic(shared_memory_ptr_, shm_size_);
    }
    //设计到基类private性质
private:
    //char* args_;
    void *shared_memory_ptr_;
    size_t shm_size_;
};

int main()
{
    //shared mem
    const char *shm_name = "/my_shm"; // 共享内存的名字  
    const size_t shm_size = 2048 * 2048; // 1 MB  

    void *shared_memory_ptr;

    // 创建共享内存  
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);  
    if (shm_fd == -1) {  
        std::cerr << "Error creating shared memory." << std::endl;  
        return 1;  
    }  
    
    // 设置共享内存的大小  
    ftruncate(shm_fd, shm_size);  

    // 映射共享内存  
    shared_memory_ptr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);  
    if (shared_memory_ptr == MAP_FAILED) {  
        std::cerr << "Error mapping shared memory." << std::endl;  
        return 1;  
    }  

    // 创建互斥锁
	pthread_mutexattr_t attr; 
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    // 创建并初始化互斥锁
    SharedMemoryHeader *HEADER = reinterpret_cast<SharedMemoryHeader*>(shared_memory_ptr);  
	pthread_mutex_init(&HEADER->sm_mutex, &attr);

    ThreadPool pool;
    pool.start(4);

    pool.submitTask(std::make_shared<Cam_Launch>(shared_memory_ptr, shm_size));
    pool.submitTask(std::make_shared<Cam_Take>(shared_memory_ptr, shm_size));

    std::cout << "main over" << std::endl;
    getchar();
    return 0;
}


