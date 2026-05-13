/*
编写一个直接使用 V4L2 实现 MP4 解码的应用程序是一个非常复杂的任务，原因在于 V4L2 本身并不处理 MP4 容器格式（解封装/Demux）。

V4L2 只是一个视频编解码接口，它只能“吃”压缩的视频流数据（如 H.264/H.265 码流），它无法理解 MP4 文件头、索引信息或音频轨道。

因此，一个完整的 V4L2 MP4 解码器通常采用 "FFmpeg (Demux) + V4L2 (Decode) + DRM/KMS (Display)" 的架构。

下面提供一个基于 C 语言的核心逻辑示例。为了代码可读性，省略了部分复杂的错误检查和 FFmpeg 的详细初始化代码。
架构流程

     FFmpeg: 读取 input.mp4，提取出 H.264 码流。
     V4L2 Output: 将 H.264 码流通过 V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE 发送给解码设备（通常是 /dev/video10 或 stateful decoder）。
     V4L2 Capture: 从解码设备取回解码后的 YUV 帧数据（V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE）。
     Display: 将 YUV 数据显示到屏幕（本例简化为保存为 RAW 数据或简单处理）。


关键难点说明

     

    Stateful Decoder (状态感知解码器):
        大多数 Linux 硬件解码器（如 coda, rockchip, meson）是 Stateful 的。这意味着它们需要接收完整的 Header (SPS/PPS) 以及第一帧关键帧才能开始工作。
        如果只是随意塞入 MP4 中间截取的 H.264 片段，解码器通常会报错。
        在 VIDIOC_STREAMON 之前，通常需要先 QBUF 发送 Header 数据。
     

    分辨率协商:
        在示例代码中，我硬编码了 1920x1080。
        实际上，MP4 文件的分辨率可能变化，或者你需要解析 H.264 的 SPS 来获取准确的宽高。一旦解码器开始输出 CAPTURE buffer，你应该通过 VIDIOC_G_FMT 获取解码器实际调整后的分辨率。
     

    数据泵:
        解码器需要一个“泵”机制：只要 OUTPUT 队列有空位，就往里填数据；只要 CAPTURE 队列有数据，就取出来显示。
        如果 OUTPUT 填得太慢，解码器会饿死（CAPTURE 队列不再产出数据）。
        如果 OUTPUT 填得太快，OUTPUT 队列会满，导致阻塞。
     

    DRM/KMS 显示:
        解码出来的 YUV 数据不能直接用 fwrite 保存成图片看（那是乱码）。
        在嵌入式 Linux 上，通常需要调用 /dev/dri/card0 接口，利用 drmModeAddFB 和 drmModePageFlip 将 YUV 数据上传到 GPU 纹理或 Overlay 层进行显示。

替代方案：GStreamer

如果你不需要从零造轮子，而是想用 C++ 实现一个 MP4 播放器，强烈建议使用 GStreamer。它底层自动处理 V4L2、FFmpeg 和 DRM 的调用。

# 命令行示例：使用 v4l2sink (需要对应的接收器) 或硬件解码插件
gst-launch-1.0 filesrc location=input.mp4 ! qtdemux ! h264parse ! v4l2h264dec ! video/x-raw,format=NV12 ! v4l2sink

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

// 假设使用 FFmpeg 解封装，这里仅展示数据流向的逻辑
// 实际项目中需要链接 libavformat libavcodec

#define OUTPUT_DEV "/dev/video10" // 解码器的输出端 (接收压缩数据)
#define CAPTURE_DEV "/dev/video10" // 解码器的捕获端 (输出原始数据，通常是同一个设备)
#define NUM_BUFFERS 4

struct buffer {
    void   *start;
    size_t  length;
};

// 1. 初始化 V4L2 解码器
int init_decoder(int fd) {
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) return -1;
    
    // 检查是否支持视频解码 (M2M 设备通常同时支持 VIDEO_OUTPUT 和 VIDEO_CAPTURE)
    if (!(cap.capabilities & V4L2_CAP_VIDEO_M2M_MPLANE)) {
        printf("Device does not support M2M mem2mem decoding\n");
        return -1;
    }

    // 2. 设置输出格式 (告诉解码器：我将给你 H.264 数据)
    struct v4l2_format fmt_out;
    memset(&fmt_out, 0, sizeof(fmt_out));
    fmt_out.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    fmt_out.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264; // MP4通常包含H.264流
    fmt_out.fmt.pix_mp.width = 1920; // 初始猜测，实际可以从SPS获取
    fmt_out.fmt.pix.mp.height = 1080;
    
    if (ioctl(fd, VIDIOC_S_FMT, &fmt_out) < 0) {
        perror("VIDIOC_S_FMT OUTPUT");
        return -1;
    }

    // 3. 设置捕获格式 (告诉解码器：我要 YUV420P 数据)
    struct v4l2_format fmt_cap;
    memset(&fmt_cap, 0, sizeof(fmt_cap));
    fmt_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt_cap.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUV420; // 或者 NV12
    fmt_cap.fmt.pix_mp.width = 1920;
    fmt_cap.fmt.pix_mp.height = 1080;
    
    if (ioctl(fd, VIDIOC_S_FMT, &fmt_cap) < 0) {
        perror("VIDIOC_S_FMT CAPTURE");
        return -1;
    }

    return 0;
}

// 4. 申请缓冲区
int setup_buffers(int fd, enum v4l2_buf_type type, struct buffer *buffers, int *num_planes) {
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = NUM_BUFFERS;
    req.type = type;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) return -1;

    for (int i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        memset(&buf, 0, sizeof(buf));
        memset(planes, 0, sizeof(planes));

        buf.type = type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        buf.length = 1; // 对于单平面格式，但 MPLANE 接口需要设置 planes 数组
        buf.m.planes = planes;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) return -1;

        // 记录平面数量 (通常 YUV420 是 1 个平面或者 3 个平面，取决于硬件)
        if (i == 0) *num_planes = buf.length; 

        for (int p = 0; p < buf.length; p++) {
            buffers[i * buf.length + p].start = mmap(NULL, buf.m.planes[p].length,
                                                     PROT_READ | PROT_WRITE, MAP_SHARED, fd, 
                                                     buf.m.planes[p].m.offset);
            buffers[i * buf.length + p].length = buf.m.planes[p].length;
        }
    }
    return req.count;
}

int main(int argc, char **argv) {
    int fd = open(CAPTURE_DEV, O_RDWR);
    if (fd < 0) { perror("Open device"); return -1; }

    init_decoder(fd);

    // --- 设置 OUTPUT 队列 (接收 H.264 码流) ---
    struct buffer out_buffers[NUM_BUFFERS];
    int out_num_planes;
    int out_buf_count = setup_buffers(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, out_buffers, &out_num_planes);
    
    // 将所有 OUTPUT buffer 入队
    for (int i = 0; i < out_buf_count; i++) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        buf.length = out_num_planes;
        buf.m.planes = planes;
        // 注意：初次入队 bytesused = 0，除非有数据要立即发
        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    // --- 设置 CAPTURE 队列 (接收 YUV 数据) ---
    struct buffer cap_buffers[NUM_BUFFERS];
    int cap_num_planes;
    int cap_buf_count = setup_buffers(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, cap_buffers, &cap_num_planes);
    
    for (int i = 0; i < cap_buf_count; i++) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[VIDEO_MAX_PLANES];
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        buf.length = cap_num_planes;
        buf.m.planes = planes;
        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    // --- 开启流 ---
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    ioctl(fd, VIDIOC_STREAMON, &type);
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    ioctl(fd, VIDIOC_STREAMON, &type);

    // --- 主循环：模拟 FFmpeg 发送数据 ---
    printf("Starting decoding loop...\n");
    
    // 模拟：这里应该有一个 FFmpeg 的 av_read_frame 循环
    // 每次得到一个 AVPacket，将其数据拷贝到 DQBUF 出来的 OUTPUT buffer 中
    int frame_count = 0;
    while (frame_count < 100) { 
        // 1. 获取一个空闲的 OUTPUT Buffer
        struct v4l2_buffer out_buf;
        struct v4l2_plane out_planes[VIDEO_MAX_PLANES];
        memset(&out_buf, 0, sizeof(out_buf));
        out_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        out_buf.memory = V4L2_MEMORY_MMAP;
        out_buf.length = out_num_planes;
        out_buf.m.planes = out_planes;
        
        if (ioctl(fd, VIDIOC_DQBUF, &out_buf) < 0) {
             // 处理 EAGAIN
             continue;
        }

        // TODO: 将 FFmpeg 的 packet->data 拷贝到 out_buffers[out_buf.index].start
        // 设置大小：out_buf.m.planes[0].bytesused = packet->size;
        // 模拟数据：
        out_buf.m.planes[0].bytesused = 1024; // 假数据
        out_buf.flags = 0; // 如果是最后一帧，设置 V4L2_BUF_FLAG_LAST

        // 重新入队发送给解码器
        ioctl(fd, VIDIOC_QBUF, &out_buf);

        // 2. 获取解码完成的 CAPTURE Buffer (YUV)
        struct v4l2_buffer cap_buf;
        struct v4l2_plane cap_planes[VIDEO_MAX_PLANES];
        memset(&cap_buf, 0, sizeof(cap_buf));
        cap_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        cap_buf.memory = V4L2_MEMORY_MMAP;
        cap_buf.length = cap_num_planes;
        cap_buf.m.planes = cap_planes;

        if (ioctl(fd, VIDIOC_DQBUF, &cap_buf) == 0) {
            printf("Decoded frame %d! Size: %d bytes\n", frame_count, cap_buf.m.planes[0].bytesused);
            
            // TODO: 这里可以将 cap_buffers[cap_buf.index].start 的 YUV 数据
            // 通过 DRM/KMS 渲染到屏幕，或者保存为文件
            
            // 处理完，放回队列
            ioctl(fd, VIDIOC_QBUF, &cap_buf);
            frame_count++;
        }
    }

    // 清理
    close(fd);
    return 0;
}