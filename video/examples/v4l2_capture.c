


/*
这是一个基于 C 语言编写的 V4L2 完整示例代码。

代码功能：

     打开 /dev/video0 设备。
     设置图像格式为 640x480 的 MJPEG（大多数 USB 摄像头都支持 MJPEG，直接输出的就是 JPEG 数据，方便演示）。
     使用 Mmap 方式申请 4 个缓冲区。
     采集 10 帧数据，并直接保存为 capture_0.jpg, capture_1.jpg 等文件。
     释放资源并退出。

编译与运行:
gcc v4l2_capture.c -o v4l2_capture
sudo ./v4l2_capture
(注意：通常需要 sudo 权限来访问 /dev/video0)

代码核心逻辑解析:

     init_device 中的 VIDIOC_REQBUFS：
        这步告诉内核驱动：“给我留 4 个坑位（buffer）”。
     init_device 中的 mmap：
        这步非常关键。它把内核预留的那块内存地址映射到了我们当前程序的虚拟地址空间。这样我们操作 buffers[i].start 就像操作普通数组一样，实际上是在操作内核缓冲区。
     start_capturing 中的 VIDIOC_QBUF：
        开始采集前，必须先把 buffer 交还给驱动。驱动是“生产者”，我们的程序是“消费者”。我们要先把空盘子给厨师（驱动），他才能装菜。
     mainloop 中的 select：
        这是一个系统调用，用来监听 fd 是否可读。如果没有画面，程序会在这里挂起 2 秒（tv.tv_sec = 2），而不是死循环占用 CPU。
     mainloop 中的 VIDIOC_DQBUF：
        从队列中取出一个填满数据的 buffer。buf.index 告诉我们是第几个 buffer（比如 index=1）。
     文件保存：
        因为摄像头设置的是 MJPEG 格式，buffers[buf.index].start 里的数据本身就是 JPEG 格式。所以直接用 fwrite 写入文件就是一张完整的图片。
     mainloop 中的 VIDIOC_QBUF：
        处理完图片后，必须把 buffer 放回队列，否则循环几次后驱动就没 buffer 可用了，画面会卡住。

常见问题排查

    /dev/video0 no such file: 检查摄像头是否连接，使用 ls /dev/video* 查看。
    VIDIOC_S_FMT error: 通常是你设置的分辨率（如 1920x1080）或格式（如 RGB24）摄像头不支持。尝试改为 640x480 或 YUYV/MJPEG。
    生成的图片打不开: 
        如果设置的是 YUYV (Raw Data)，保存为 .jpg 是打不开的，需要用专门的看图软件（如 7yuv）。
        如果设置的是 MJPEG 但图片损坏，可能是 buf.bytesused 长度不对，或者传输过程中数据丢失（USB 带宽不足）。
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>      // open, O_RDWR
#include <unistd.h>     // close, exit
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define BUFFER_COUNT 4

// 定义缓冲区结构体
struct buffer {
    void   *start;
    size_t  length;
};

static int fd = -1;
struct buffer *buffers = NULL;

// 打印错误信息并退出
static void errno_exit(const char *s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

// xioctl 是对 ioctl 的封装，处理错误重试
static int xioctl(int fh, int request, void *arg) {
    int r;
    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

// 1. 打开设备
static void open_device(void) {
    fd = open("/dev/video0", O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd) {
        fprintf(stderr, "Cannot open '/dev/video0'\n");
        exit(EXIT_FAILURE);
    }
}

// 2. 初始化设备（设置格式）
static void init_device(void) {
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    unsigned int min;

    // 查询设备能力
    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "/dev/video0 is no V4L2 device\n");
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "/dev/video0 is no video capture device\n");
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "/dev/video0 does not support streaming i/o\n");
        exit(EXIT_FAILURE);
    }

    // 设置视频格式
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = IMAGE_WIDTH;
    fmt.fmt.pix.height      = IMAGE_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; // 使用 MJPEG 格式，直接存为 jpg
    // 如果想用 YUYV，改为 V4L2_PIX_FMT_YUYV，但保存文件时需要用 raw viewer 看
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        errno_exit("VIDIOC_S_FMT");

    // 打印实际设置的格式（驱动可能会修改我们请求的值）
    printf("Format set: %c%c%c%c %dx%d\n", 
           fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF,
           fmt.fmt.pix.width, fmt.fmt.pix.height);

    // 3. 申请缓冲区
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "/dev/video0 does not support memory mapping\n");
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on /dev/video0\n");
        exit(EXIT_FAILURE);
    }

    buffers = calloc(req.count, sizeof(*buffers));

    // 4. 映射缓冲区
    for (unsigned int i = 0; i < req.count; ++i) {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
            errno_exit("VIDIOC_QUERYBUF");

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

        if (MAP_FAILED == buffers[i].start)
            errno_exit("mmap");
    }
}

// 5. 开始采集
static void start_capturing(void) {
    enum v4l2_buf_type type;
    
    // 将所有缓冲区放入队列
    for (unsigned int i = 0; i < BUFFER_COUNT; ++i) {
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
        errno_exit("VIDIOC_STREAMON");
}

// 6. 主循环：读取并保存
static void mainloop(void) {
    unsigned int count = 10; // 采集 10 帧
    unsigned int loop = 0;

    while (loop < count) {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        // 使用 select 等待数据，避免死等
        r = select(fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
            if (EINTR == errno)
                continue;
            errno_exit("select");
        }

        if (0 == r) {
            fprintf(stderr, "select timeout\n");
            exit(EXIT_FAILURE);
        }

        // 出队
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        
        // 注意：这里阻塞直到拿到一帧
        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
                case EAGAIN: return;
                case EIO:   /* Could ignore EIO, see spec. */
                            continue;
                default:    errno_exit("VIDIOC_DQBUF");
            }
        }

        assert(buf.index < BUFFER_COUNT);

        // 保存图片文件
        char filename[32];
        sprintf(filename, "capture_%d.jpg", loop);
        FILE *fp = fopen(filename, "wb");
        if (fp) {
            fwrite(buffers[buf.index].start, buf.bytesused, 1, fp);
            printf("Saved %s (size: %d bytes)\n", filename, buf.bytesused);
            fclose(fp);
        } else {
            printf("Failed to open file %s\n", filename);
        }

        // 重新入队，让驱动继续使用这个 buffer
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
            
        loop++;
    }
}

// 7. 停止采集
static void stop_capturing(void) {
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
        errno_exit("VIDIOC_STREAMOFF");
}

// 8. 关闭设备与释放内存
static void uninit_device(void) {
    unsigned int i;

    for (i = 0; i < BUFFER_COUNT; ++i)
        if (-1 == munmap(buffers[i].start, buffers[i].length))
            errno_exit("munmap");

    free(buffers);
}

static void close_device(void) {
    if (-1 == close(fd))
        errno_exit("close");
    fd = -1;
}

int main(int argc, char **argv) {
    open_device();
    init_device();
    start_capturing();
    mainloop();
    stop_capturing();
    uninit_device();
    close_device();
    fprintf(stderr, "\nProgram finished successfully.\n");
    return 0;
}