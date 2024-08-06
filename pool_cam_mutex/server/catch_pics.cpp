#include "commen_struct.h"

#define SET_HEIGHT 1024
#define SET_WIDTH  768

int catch_pics(void *shared_memory_ptr, size_t shm_size)
{
    /*open*/
    const char *s1 = "/dev/video0";

    int fd = open(s1, O_RDWR);
    //int fd = (int)args;

    /*app接口数据格式*/
    struct v4l2_fmtdesc fmtdesc;
    struct v4l2_frmsizeenum fsenum;
    int fmt_index = 0;
    int frame_index;

    int i;
    void* bufs[32];
    int buf_cnt;

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    struct pollfd fds[1];
    char filename[32];
    int file_cnt = 0;

    if(fd < 0)
    {
        printf("cannot open fd: %d\n", fd);
        return -1;
    }

    /*查询能力*/
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));
    if(0 == ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0){
            fprintf(stderr, "error opening device! : does not sup\n");
            return -1;
        }
        if(!(cap.capabilities & V4L2_CAP_STREAMING)){
            fprintf(stderr, "does not streaming\n");
            return -1;
        }
    }
    else
    {
        printf("cannot get capability!\n");
        return -1;
    }

    while(1)
    {
        /*enum type*/
        fmtdesc.index = fmt_index;  // 比如从0开始
        fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;  // 指定type为"捕获"
        if(0 != ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc))
        {
            break;
        }
        
        frame_index = 0;
        while(1)
        {
            /*枚举支持帧大小*/
            memset(&fsenum, 0, sizeof(fsenum));
            fsenum.pixel_format = fmtdesc.pixelformat;
            fsenum.index = frame_index;

            if(ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fsenum) == 0)
            {
                printf("format %s, %d, framesize%d: %d x %d\n", fmtdesc.description, fsenum.pixel_format, frame_index, fsenum.discrete.width, fsenum.discrete.height);
            }else
            {
                break;
            }
            frame_index++;
        }
        fmt_index++;
    }

    /*设置格式*/
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = SET_HEIGHT;
    fmt.fmt.pix.height = SET_WIDTH;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;
    if(0 == ioctl(fd, VIDIOC_S_FMT, &fmt))
    {
        printf("set format ok, %d x %d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
    }
    else
    {
        printf("cannot set format\n");
        return -1;
    }
    
/*
* request buffers
*/
    struct v4l2_requestbuffers rb;
    memset(&rb, 0, sizeof(struct v4l2_requestbuffers));
    rb.count = 32;
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rb.memory = V4L2_MEMORY_MMAP;
    
    if(0 == ioctl(fd, VIDIOC_REQBUFS, &rb)) {
    /*
     * map the buffers
     */
        buf_cnt = rb.count;
        for(i = 0; i < rb.count; i++) {
            struct v4l2_buffer buf;

            memset(&buf, 0, sizeof(struct v4l2_buffer));
            buf.index = i;
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if(0 == ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
            /*succseed*/
            /*mmap*/
                bufs[i] = mmap(0 /* start anywhere */ ,
                                buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                                buf.m.offset);
                if(bufs[i] == MAP_FAILED) {
                    perror("Unable to map buffer");
                    return -1;
                }
            }else{
                perror("Unable to query buffers");
                return -1;
            }
        }
        printf("map %d buffers succeed\n", buf_cnt);
    }
    else{
        perror("Unable to allocate buffers");
        return -1;
    }

    /*put buffer to idle queue*/
    /*
     * Queue the buffers.
     */
    for(i = 0; i < buf_cnt; ++i) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if(0 != ioctl(fd, VIDIOC_QBUF, &buf)) {
            perror("Unable to queue buffer");
            return -1;
        }
    }
    printf("queue buffer ok!\n");

    /*launch camera*/
    if(0 != ioctl(fd, VIDIOC_STREAMON, &type)) {
        perror("Unable to start capture");
        return -1;
    }
    printf("launch camera ok!\n");
    //int catch_pics_num = 0;
    while(1){
        /*poll check data*/
        if(1){
            memset(fds, 0 ,sizeof(fds));
            fds[0].fd = fd;
            fds[0].events = POLLIN;
            if(1 == poll(fds, 1, -1)){
                /*buffer 取出队列*/
                struct v4l2_buffer buf;

                memset(&buf, 0, sizeof(struct v4l2_buffer));

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;

                if(0 != ioctl(fd, VIDIOC_DQBUF, &buf)) {
                    perror("Unable to dequeue buffer");
                    return -1;
                }

                // 从 V4L2 缓冲区中读取图像数据并写入共享内存
                SharedMemoryHeader* header = reinterpret_cast<SharedMemoryHeader*>(shared_memory_ptr);

                //查询互斥量
                std::cout << "Wait Lock " << std::endl;
                pthread_mutex_lock(&header->sm_mutex);// 阻塞,等待锁释放
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                size_t bytes_to_copy = buf.bytesused; // V4L2 缓冲区中图像的字节数
                header->dataSize = bytes_to_copy;  
                header->height = SET_HEIGHT;
                header->width  = SET_WIDTH;
                if (bytes_to_copy > shm_size) {
                    bytes_to_copy = shm_size; // 限制写入字节数到共享内存最大限制  
                }  

                memcpy(shared_memory_ptr + sizeof(SharedMemoryHeader), reinterpret_cast<void *>(bufs[buf.index]), bytes_to_copy);  
                std::cout << "Copied " << bytes_to_copy << " bytes from V4L2 buffer to shared memory." << std::endl;  
                file_cnt++;

                pthread_mutex_unlock(&header->sm_mutex);// 锁释放

                /*buffer 用完放入队列*/
                if(0 != ioctl(fd, VIDIOC_QBUF, &buf)) {
                    perror("Unable to queue buffer");
                    return -1;
                }
            }
        //sleep for 50ms
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    /*close camera*/
    if(0 != ioctl(fd, VIDIOC_STREAMOFF, &type)) {
        perror("Unable to STOP capture");
        return -1;
    }
    printf("STOP camera ok!\n");

    close(fd);

    return 0;
}