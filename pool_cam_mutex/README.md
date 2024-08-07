# 使用说明
 ## 关于编译
 
 **需要提前安装boost库和opencv库**
 **将threadpool的动态库和头文件分别加入系统路径**

 分别编译server和client
 ` g++ -std=c++17 -o server server.cpp catch_pics.cpp fun_pic.cpp -I . -ltdpool -lpthread `pkg-config --cflags --libs opencv4` `
 ` g++ -std=c++17 -o client client.cpp pic_send.cpp -I . -ltdpool -lpthread `pkg-config --cflags --libs opencv4` `

 ## 如何使用
 ### 环境
 局域网连接
 要求各主机固定IP地址

 ### 在树莓派终端运行server
 ` ./server `
 ### 在树莓派另一终端运行client
 ` ./client `
 按照提示输入目标推送的主机地址及端口号
 `root$ 192.168.1.10` 
 `root$ 1000 `
 即可将图片数据通过UDP转发至目标主机，可以多次输入以将数据提供给不同的局域网主机，连接断开后线程会自动回收

 ### 更新功能中
 1、增加数据图片时间戳与id，避免采集线程卡住导致重复推送单一帧
 2、提高采集帧率，增加client输入分辨率、是否标记人脸等图片配置
 3、增加远程主机客户端以实时显示