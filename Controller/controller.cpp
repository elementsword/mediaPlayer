#include "controller.h"

bool Controller::open(const std::string &url)
{
    demuxer.open(url);
    videoDecoder.open(url);
    audioDecoder.open(url);
}
void Controller::start()
{
    audioThreadRunning = true;
    demuxer.start();
    audioThread = std::thread(&Controller::audioLoop, this);
    videoThread = std::thread(&Controller::videoLoop, this);
}
void Controller::stop()
{
    audioThreadRunning = false;
    if (audioThread.joinable())
        audioThread.join();
    if (videoThread.joinable())
        videoThread.join();

    audioDecoder.close();
    videoDecoder.close();
    demuxer.stop();
}

void Controller::audioLoop()
{
    while (audioThreadRunning) // 线程运行控制变量
    {
        auto optPktPtr = demuxer.getAudioQueue().pop();
        if (!optPktPtr.has_value())
        {
            break;
        }

        AVPacket *audioPkt = optPktPtr.value(); // 获取指针

        if (!audioPkt)
        {
            continue;
        }

        // 解码音频包，假设 audioDecoder.decode 接收 AVPacket*
        AVFrame *frame = audioDecoder.decode(*audioPkt); // 注意传引用或指针根据接口

        // 用完后释放包内存（如果你负责管理包生命周期）
        av_packet_unref(audioPkt);
        av_packet_free(&audioPkt);

        if (frame)
        {
            // 处理解码后帧，推入队列或者播放
            // 记得管理frame生命周期，比如后续用完av_frame_free(&frame);
        }
    }
}
void Controller::videoLoop()
{
    while (videoThreadRunning) // 线程运行控制变量
    {
        auto optPktPtr = demuxer.getVideoQueue().pop();
        if (!optPktPtr.has_value())
        {
            break;
        }

        AVPacket *videoPkt = optPktPtr.value(); // 获取指针

        if (!videoPkt)
        {
            continue;
        }

        // 解码音频包，假设 audioDecoder.decode 接收 AVPacket*
        AVFrame *frame = audioDecoder.decode(*videoPkt); // 注意传引用或指针根据接口

        // 用完后释放包内存（如果你负责管理包生命周期）
        av_packet_unref(videoPkt);
        av_packet_free(&videoPkt);

        if (frame)
        {
            // 处理解码后帧，推入队列或者播放
            // 记得管理frame生命周期，比如后续用完av_frame_free(&frame);
        }
    }
}
