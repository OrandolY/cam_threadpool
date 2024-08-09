#include <iostream>  
#include <vector>  
#include <cstring>  
#include <unistd.h>  
#include <arpa/inet.h>  
#include <fstream>  

#define MAX_PACKET_SIZE 1024 // 自定义每个报文的最大大小  

// Socket接收分片函数
void receiveImageFromSocket(const std::string& sourceIp, int sourcePort, int listenPort, const std::string& outputFilePath) {  
    int sock = socket(AF_INET, SOCK_DGRAM, 0);  
    if (sock < 0) {  
        perror("Error creating socket");  
        return;  
    }  

    sockaddr_in serverAddr;  
    memset(&serverAddr, 0, sizeof(serverAddr));  
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_port = htons(listenPort);  
    serverAddr.sin_addr.s_addr = INADDR_ANY; // 允许接收任何IP发来的数据  

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {  
        perror("Bind failed");  
        close(sock);  
        return;  
    }  

    std::vector<unsigned char> imageData; // 存储接收到的图像数据  
    int expectedSequenceNumber = 1; // 从1开始接收  
    bool moreData = true;  

    while (moreData) {  
        std::vector<unsigned char> packet(MAX_PACKET_SIZE);  
        sockaddr_in clientAddr;  
        socklen_t addrLen = sizeof(clientAddr);  
        
        ssize_t receivedBytes = recvfrom(sock, packet.data(), MAX_PACKET_SIZE, 0, (struct sockaddr*)&clientAddr, &addrLen);  
        if (receivedBytes < 0) {  
            perror("Error receiving data");  
            break;  
        }  

        // 检查来源IP和端口  
        if (inet_ntoa(clientAddr.sin_addr) != sourceIp || ntohs(clientAddr.sin_port) != sourcePort) {  
            std::cout << "Received data from an unknown source, ignoring." << std::endl;  
            continue; // 忽略来自其他地址的数据  
        }  

        // 解析数据包  
        int sequenceNumber = *(int*)packet.data(); // 获取序号  
        int reamin_flag = *(int*)(packet.data() + sizeof(int)); // 获取剩余数据标志  

        // 仅在序号为1时开始存储数据  
        if (sequenceNumber == expectedSequenceNumber) {  
            int dataOffset = sizeof(int) + sizeof(int);  
            // 只保存大于等于期望序号的数据  
            imageData.insert(imageData.end(), packet.data() + dataOffset, packet.data() + receivedBytes);  
            expectedSequenceNumber++; // 增加期望的序号  
        }  

        // 检查是否还有剩余的数据包  
        moreData = (remain_flag != 0);  
    }  

    // 关闭socket  
    close(sock);  

    // 写入图像数据到文件  
    std::ofstream outputFile(outputFilePath, std::ios::binary);  
    outputFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());  
    outputFile.close();  

    std::cout << "Image received and saved to " << outputFilePath << std::endl;  
}  

int main() {  
    std::string sourceIp = "192.168.5.12"; // 指定发送方的IP地址  
    int sourcePort = 1000; // 指定发送方的端口  
    int listenPort = 1000; // 本机监听端口  
    std::string outputFilePath = "received_image.jpg"; // 输出文件路径  

    receiveImageFromSocket(sourceIp, sourcePort, listenPort, outputFilePath);  
    
    return 0;  
}