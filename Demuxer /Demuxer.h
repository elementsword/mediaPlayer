#ifndef DEMUXER_H
#define DEMUXER_H

#include <thread>
#include <atomic>
#include <iostream>
#include <optional>
#include "../DataStructure/ThreadSafeQueue/ThreadSafeQueue.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

class Demuxer
{
public:
    Demuxer();
    ~Demuxer();
    // 写入数据到缓冲区
    void start();
    void stop();
    bool open(const std::string url);
    // 从缓冲区读取数据
    // 返回实际读取的字节数（可能小于请求）
    size_t read(uint8_t *out, size_t size);

    // 获取当前缓冲数据大小
    size_t size() const;

    // 清空缓冲区
    void clear();

private:
    AVFormatContext *fmtCtx;
    int audioStreamIndex;
    int videoStreamIndex;
    std::atomic<bool> quitFlag;
    std::thread demuxThread;
    std::string inputFile;

    ThreadSafeQueue<AVPacket> audioPacketQueue;
    ThreadSafeQueue<AVPacket> videoPacketQueue;
}
#endif