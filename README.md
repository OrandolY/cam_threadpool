## 基于手写threadpool实现的camera控制与图像处理模板
### 平台树莓派4B+csi_camera

### threadpool
参见repo
交叉编译成动态库

### camera控制 通过v4l2框架实现设备控制、图像存储

### 共享内存实现线程间图像数据传输

### 互斥锁实现对临界内存资源的读取控制

### 可以通过多个buffer乒乓操作避免读写冲突