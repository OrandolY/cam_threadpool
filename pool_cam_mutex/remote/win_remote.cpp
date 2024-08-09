#include <iostream>  
#include <fstream>  
#include <vector>  
#include <winsock2.h>  
//#include <opencv2/opencv.hpp>  


void receiveImageFromSocket(const std::string& listenIp, int listenPort, const std::string& outputFilePath) {  
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  
    if (sock == INVALID_SOCKET) {  
        std::cerr << "Socket creation failed!" << std::endl;  
        return;  
    }  

    sockaddr_in serverAddr = {};  
    serverAddr.sin_family = AF_INET;  
    serverAddr.sin_addr.s_addr = inet_addr(listenIp.c_str());  
    serverAddr.sin_port = htons(listenPort);  

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {  
        std::cerr << "Binding failed!" << std::endl;  
        closesocket(sock);  
        return;  
    }  

while(1){
    std::vector<char> packet(4096); // Buffer for incoming packets  
    std::vector<char> imageData; // Vector to hold image data  
    int expectedSequenceNumber = 0; // Expected sequence number  

    std::cout << "Listening for incoming data on " << listenIp << ":" << listenPort << "..." << std::endl;  

    while (true) {  
        sockaddr_in clientAddr = {};  
        int addrLen = sizeof(clientAddr);  
        
        ssize_t receivedBytes = recvfrom(sock, packet.data(), packet.size(), 0, (struct sockaddr*)&clientAddr, &addrLen);  
        if (receivedBytes < 0) {  
            perror("Error receiving data");  
            continue; // Continue to listen in case of an error  
        }  

        std::cout << "Received " << receivedBytes << " bytes from "  
                  << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;  

        int sequenceNumber = *(int*)packet.data();  
        int remain_flag = *(int*)(packet.data() + sizeof(int));  

        std::cout << "Sequence Number: " << sequenceNumber << ", Remain Flag: " << remain_flag << std::endl;  

        if (sequenceNumber == expectedSequenceNumber) {  
            int dataOffset = sizeof(int) + sizeof(int);  
            imageData.insert(imageData.end(), packet.data() + dataOffset, packet.data() + receivedBytes);  
            expectedSequenceNumber++;  
            std::cout << "Image data added, expected sequence number updated to " << expectedSequenceNumber << std::endl;  
        } else {  
            std::cout << "Unexpected sequence number. Expected: " << expectedSequenceNumber << ", but received: " << sequenceNumber << std::endl;  
        }  

        bool moreData = (remain_flag != 0);  
        if (!moreData) {  
            break; // Exit the loop if there's no more data  
        }  
    }  
    /*
    // 将图像数据转换为 cv::Mat 进行显示  
    cv::Mat img = cv::imdecode(imageBuffer, cv::IMREAD_COLOR);  
    if (img.empty()) {  
        std::cerr << "Could not decode image!" << std::endl;  
        return;  
    } 
    // 显示图像  
    cv::imshow("Received Image", img); */
    std::ofstream outputFile(outputFilePath, std::ios::binary);  
    outputFile.write(reinterpret_cast<const char*>(imageData.data()), imageData.size());  
    outputFile.close();

    std::cout << "Image received and saved to " << outputFilePath << std::endl; 
}
    closesocket(sock);  
}  

int main() {  
    WSADATA wsaData;  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {  
        std::cerr << "WSAStartup failed!" << std::endl;  
        return 1;  
    }  

    std::string listenIp = "192.168.5.13";  // Listening IP  
    int listenPort = 1000;                   // Listening port  
    std::string outputFilePath = "received_image.jpg";  

    receiveImageFromSocket(listenIp, listenPort, outputFilePath);  
    getchar();
    WSACleanup();
    return 0;  
}