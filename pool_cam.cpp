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

#include "commen_struct.h"

/*
�ڲ��ֳ�������Ҫ��ȡ�߳�ִ������ķ���ֵ
e.g.
    ���߳� ��ˮ�߲���
    ��һ���¼���ִ������պϲ������ս����
*/

/*������ Ҫ �������*/
class Cam_Launch : public Task
{
public:
    Cam_Launch(void *shared_memory_ptr_, const size_t shm_size_)
    :    shared_memory_ptr_(shared_memory_ptr_),
         shm_size_(shm_size_)
    {}
    //Ques1 ������run() ʹ�䷵��ֵ���Ա�ʾ�������ͣ�
    //��JAVA PY �е�Object ���������������͵Ļ��࣬�����඼���Դ����̳У�
    //C++ 17 any����
    Any run()  // run�������վ����̳߳ط�����߳���ȥ��ִ����!
    {
        return catch_pics(shared_memory_ptr_, shm_size_);
    }
    //��Ƶ�����private����
private:
    //char* args_;
    void *shared_memory_ptr_;
    size_t shm_size_;
};

/*������ Ҫ �������*/
class Cam_Take : public Task
{
public:
    Cam_Take(void *shared_memory_ptr_, const size_t shm_size_)
    :    shared_memory_ptr_(shared_memory_ptr_),
         shm_size_(shm_size_)
    {}
    //Ques1 ������run() ʹ�䷵��ֵ���Ա�ʾ�������ͣ�
    //��JAVA PY �е�Object ���������������͵Ļ��࣬�����඼���Դ����̳У�
    //C++ 17 any����
    Any run()  // run�������վ����̳߳ط�����߳���ȥ��ִ����!
    {
        return fun_pic(shared_memory_ptr_, shm_size_);
    }
    //��Ƶ�����private����
private:
    //char* args_;
    void *shared_memory_ptr_;
    size_t shm_size_;
};

int main()
{
    //shared mem
    const char *shm_name = "/my_shm"; // �����ڴ������  
    const size_t shm_size = 1024 * 1024; // 1 MB  

    void *shared_memory_ptr;

    // ���������ڴ�  
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);  
    if (shm_fd == -1) {  
        std::cerr << "Error creating shared memory." << std::endl;  
        return 1;  
    }  
    
    // ���ù����ڴ�Ĵ�С  
    ftruncate(shm_fd, shm_size);  

    // ӳ�乲���ڴ�  
    shared_memory_ptr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);  
    if (shared_memory_ptr == MAP_FAILED) {  
        std::cerr << "Error mapping shared memory." << std::endl;  
        return 1;  
    }  

    ThreadPool pool;
    pool.start(4);

    pool.submitTask(std::make_shared<Cam_Launch>(shared_memory_ptr, shm_size));
    pool.submitTask(std::make_shared<Cam_Take>(shared_memory_ptr, shm_size));




    //char str[] = "/dev/video1";
    //Result res1 = pool.submitTask(std::make_shared<MyTask>());

    //int sum1 = res1.get().cast_<int>();
    //std::cout << sum1 << std::endl;

    std::cout << "main over" << std::endl;
    getchar();
    return 0;
}


