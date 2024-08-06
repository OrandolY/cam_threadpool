//arm-buildroot-linux-gnueabihf-g++ -std=c++17  -L . -I .  -o pooltest pool_test.cpp -ltdpool -lpthread
#include <iostream>
#include <chrono>
#include <thread>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "threadpool.h"

#include <stdio.h>
#include <string.h>

using namespace std;

#include "client.h"

/*派生类 要 传入参数*/
class Pic_Send : public Task
{
public:
    Pic_Send(const char *shm_name_,  const char* des_ip_addr_, int des_ip_port_)
    :   shm_name_(shm_name_),
        des_ip_addr_(des_ip_addr_),
        des_ip_port_(des_ip_port_)
    {}
    //Ques1 如何设计run() 使其返回值可以表示任意类型；
    //在JAVA PY 中的Object 是所有其他类类型的基类，所有类都可以从它继承；
    //C++ 17 any类型
    Any run()  // run方法最终就在线程池分配的线程中去做执行了!
    {
        return pic_send(shm_name_, des_ip_addr_, des_ip_port_);
    }
    //设计到基类private性质
private:
    const char* shm_name_;
    const char* des_ip_addr_;
    int         des_ip_port_;
};


int main()
{
    const char *shm_name = "/my_shm";
    ThreadPool pool;
    pool.start(4);

    cout << "type in ip_addr:" << endl;
    string des_ip_addr_input;
    cin >> des_ip_addr_input;
    const char* des_ip_addr_input_const_c = des_ip_addr_input.c_str();

    cout << "type in ip_port:" << endl;
    int des_ip_port_input;
    cin >> des_ip_port_input;

    pool.submitTask(std::make_shared<Pic_Send>(shm_name, des_ip_addr_input_const_c, des_ip_port_input));

    std::cout << "main over" << std::endl;
    getchar();
    return 0;
}
