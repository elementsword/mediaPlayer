#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <thread>
#include <atomic>
#include "../Demuxer/Demuxer.h"
#include "../Decoder/AudioDecoder/audioDecoder.h"
#include "../Decoder/VideoDecoder/videoDecoder.h"
#include "../DataStructure/ThreadSafeQueue/ThreadSafeQueue.h"
#include "../sdl/sdl.h"

class Controller
{
public:
    bool open(const std::string &url);
    void start();
    void stop();

private:
    Demuxer demuxer;
    AudioDecoder audioDecoder;
    VideoDecoder videoDecoder;
    Sdl sdl;
    std::thread audioThread;
    std::thread videoThread;
    std::atomic<bool> quitFlag;

    // 解码后的帧队列 (需要线程安全队列)
    ThreadSafeQueue<AVFrame *> videoFrameQueue;
    ThreadSafeQueue<AVFrame *> audioFrameQueue;
    bool audioThreadRunning = false;
    bool videoThreadRunning = false;
    bool videoRenderRunning = false;
    bool audioPlayRunning = false;
    void audioLoop();
    void videoLoop();
    // 负责从视频解码队列里取视频帧并调用 SDL 渲染显示
    void videoRenderLoop();

    // 负责从音频解码队列里取解码音频帧数据写入 SDL 音频缓冲区
    void audioPlayLoop();
};

#endif