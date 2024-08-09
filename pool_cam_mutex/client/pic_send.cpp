/* g++ img_tran.cpp  -o img_tran  `pkg-config --cflags --libs opencv4`*/
#include <iostream>  
#include <opencv2/opencv.hpp>  
#include <opencv2/imgproc.hpp>  
#include <opencv2/highgui.hpp>  
#include <opencv2/objdetect.hpp>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h> 
#include <cstring>  
#include <cstdlib> 
#include <unistd.h>  
#include <fcntl.h>  
#include <sys/mman.h>  
#include <string.h>  
#include <time.h>  
#include <sys/stat.h>  

using namespace cv;  
using namespace std;  

#include "client.h"

// Socket传输
#define MAX_PACKET_SIZE 1024 // 自定义每个报文的最大大小  

// Socket传输分片函数  
void sendImageToSocket(const unsigned char* imageData, int size, const char* ipAddress, int port) {  
    int sock = socket(AF_INET, SOCK_DGRAM, 0);  
    if (sock < 0) {  
        perror("Error creating socket");  
        return;  
    }  

    int localPort = 1000;
    // 绑定本地端口  
    sockaddr_in localAddr;  
    memset(&localAddr, 0, sizeof(localAddr));  
    localAddr.sin_family = AF_INET;  
    localAddr.sin_port = htons(localPort);  
    localAddr.sin_addr.s_addr = INADDR_ANY; // 绑定到所有可用的接口  

    sockaddr_in serverAddr;  
    memset(&serverAddr, 0, sizeof(serverAddr));  
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(port);  

    if (inet_pton(AF_INET, ipAddress, &serverAddr.sin_addr) <= 0) {  
        std::cerr << "Invalid address / Address not supported" << std::endl;  
        close(sock);  
        return;  
    }  

    // 连接到服务器  
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {  
        perror("Connection failed");  
        close(sock);  
        return;  
    }  

    int totalSent = 0;  
    int sequenceNumber = 0; // 数据片的序号  
    
    while (totalSent < size) {  
        int remaining = size - totalSent;  
        int packetSize = remaining > MAX_PACKET_SIZE ? MAX_PACKET_SIZE : remaining; 
        int reamin_flag = remaining > MAX_PACKET_SIZE ? 1 : 0;
	cout << "size:" << size << endl;
        cout << "totalSent:" << totalSent << endl;
        cout << "packetSize:" << packetSize << endl;
        cout << "remaining:" << remaining << endl; 

        // 创建数据包，包含序号和数据  
        std::vector<unsigned char> packet(packetSize + sizeof(int) +  sizeof(int)); // + sizeof(int) for sequence number  
        *(int*)packet.data() = sequenceNumber; // 将序号放在数据
	*(int*)(packet.data() + sizeof(int)) = reamin_flag;// 把是否还有剩余数据包放在第二个int 
        memcpy(packet.data() + sizeof(int) + sizeof(int), imageData + totalSent, packetSize); // 复制图像数据  
        
        // 发送数据  
        //ssize_t sentBytes = send(sock, packet.data(), packet.size(), 0);
        ssize_t sentBytes = sendto(sock, packet.data(), packet.size(), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (sentBytes < 0) {  
            perror("Error sending data");  
            break;  
        }  

        totalSent += packetSize;  
        cout << sequenceNumber << endl;
	sequenceNumber++; // 增加序号  
        //cout << sequenceNumber << endl;
    }  

    close(sock);  
}  

// 图像处理函数  
void processImage(const Mat& inputImage, const char* des_ip_addr, int des_ip_port) {  
    Mat  blurredImage, detectedFaceImage;  
    
    if (inputImage.empty()) {  
        std::cerr << "Error: inputImage is empty!" << std::endl;  
        return;  // 处理错误  
    }  

    if (inputImage.channels() != 3) {  
        std::cerr << "Error: inputImage must have 3 channels (BGR)!" << std::endl;  
        return;  // 处理错误  
    }  

    imwrite("resulet.jpg",inputImage);

    time_t now;  
    char time_str[20];  

    now = time(NULL);  
    sprintf(time_str, "%ld", now);  
    printf("Processing start: %s s\n", time_str);  

    // 假设要创建一个与输入图像相同大小的灰度图像  
    Mat grayImage(inputImage.size(), CV_8UC1);   

    // 然后进行颜色空间转换  
    cvtColor(inputImage, grayImage, COLOR_BGR2GRAY);   

    // 应用高斯滤波  
    GaussianBlur(grayImage, blurredImage, Size(15, 15), 0);  

    // 人脸检测  
    CascadeClassifier faceCascade;  
    faceCascade.load("/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml"); // 请确保文件存在  
    std::vector<Rect> faces;  
    faceCascade.detectMultiScale(blurredImage, faces);  

    // 在检测到的人脸上绘制矩形框  
    detectedFaceImage = inputImage.clone();  
    for (const auto& face : faces) {  
        rectangle(detectedFaceImage, face, Scalar(0, 255, 0), 2);  
    }  

    // 获取人脸数量  
    int faceCount = faces.size();  
    std::cout << "检测到的人脸数量: " << faceCount << std::endl;  

    // 将处理后的图像编码为JPEG格式  
    std::vector<uchar> buf;  
    imencode(".jpg", detectedFaceImage, buf);
    //imencode(".jpg", inputImage, buf);

    now = time(NULL);  
    sprintf(time_str, "%ld", now);  
    printf("Processing finish: %s s\n", time_str);  

    // 发送图像数据  
    //sendImageToSocket(buf.data(), buf.size(), "192.168.5.9", 1000);
    sendImageToSocket(buf.data(), buf.size(), des_ip_addr, des_ip_port);  
}

// 共享内存读取函数  
Mat readFromSharedMemory(const char* shmName) {  

    //定义共享内存头结构体  
    struct SharedMemoryHeader {  
        size_t height;
        size_t width;
        pthread_mutex_t sm_mutex;
        
        size_t dataSize; // 数据大小
    };

    // 打开共享内存  
    int shm_fd = shm_open("/my_shm", O_RDWR, 0666);  
    if (shm_fd == -1) {  
        cerr << "Failed to open shared memory." << endl;  
        return Mat();   
    }

    // 获取共享内存的大小  
    struct stat sb;  
    if (fstat(shm_fd, &sb) == -1) {  
        cerr << "Failed to stat shared memory." << endl;  
        close(shm_fd);  
        return Mat();  
    }  

    // 映射共享内存  
    void* shmPtr = mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);  
    if (shmPtr == MAP_FAILED) {  
        cerr << "Failed to map shared memory." << endl;  
        close(shm_fd);  
        return Mat();  
    }  

    SharedMemoryHeader *header = reinterpret_cast<SharedMemoryHeader*>(shmPtr);  

    //查询互斥量
    std::cout << "Wait Lock " << std::endl;
    pthread_mutex_lock(&header->sm_mutex);// 阻塞,等待锁释放
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // 从共享内存头部读取信息  
    size_t height = header->height;
    size_t width = header->width;
    size_t dataSize = header->dataSize;

    std::cout << "Read" << dataSize <<  "byte from shared memory." << std::endl;  
    std::cout << "Height:" << header->height << std::endl;

    // 创建缓冲区并复制数据  
    std::vector<char> buffer(dataSize);  
    memcpy(buffer.data(), reinterpret_cast<char*>(shmPtr) + sizeof(SharedMemoryHeader), dataSize);  
    
    // 锁释放
    pthread_mutex_unlock(&header->sm_mutex);

    // 创建 Mat 对象并将缓冲区的数据转换成 Mat 格式  
    Mat image = imdecode(buffer, IMREAD_COLOR); 

    munmap(shmPtr, sb.st_size); // 释放映射  
    close(shm_fd);  
    return image;  
}  

int pic_send(const char *shm_name_,  const char* des_ip_addr, int des_ip_port) {
    while(1){
    // 从共享内存读取图像
    Mat image = readFromSharedMemory(shm_name_);
    if (image.empty()) {  
        std::cerr << "Error: Could not load image from shared memory!" << std::endl;  
        return -1;  
    }  

    // 处理图像  
    processImage(image, des_ip_addr, des_ip_port);  
    }
    return 0;  
}
