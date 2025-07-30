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
    audioDecoderThread = std::thread(&Controller::audioLoop, this);
    videoDecoderThread = std::thread(&Controller::videoLoop, this);
    audioPlayThread = std::thread(&Controller::audioPlayLoop, this);
    videoPlayThread = std::thread(&Controller::videoRenderLoop, this);
}
void Controller::stop()
{
    audioThreadRunning = false;
    if (audioDecoderThread.joinable())
        audioDecoderThread.join();
    if (videoDecoderThread.joinable())
        videoDecoderThread.join();
    if (audioPlayThread.joinable())
        audioPlayThread.join();
    if (videoPlayThread.joinable())
        videoPlayThread.join();

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
            audioFrameQueue.push(frame);
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
            videoFrameQueue.push(frame);
        }
    }
}
void Controller::videoRenderLoop()
{
    videoRenderRunning = true;
    while (videoRenderRunning)
    {
        auto optFrame = videoFrameQueue.pop();
        if (!optFrame.has_value())
        {
            // 队列停止且无数据，退出渲染循环
            break;
        }
        AVFrame *frame = optFrame.value();

        // 调用 SDL 渲染接口
        sdl.renderFrame(frame->data, frame->linesize);

        // 释放 AVFrame
        av_frame_free(&frame);

        // 控制帧率或者等待（如果需要）
        SDL_Delay(40);
    }
}

void Controller::audioPlayLoop()
{
    audioPlayRunning = true;
    while (audioPlayRunning)
    {
        auto optFrame = audioFrameQueue.pop();
        if (!optFrame.has_value())
        {
            // 队列停止且无数据，退出音频播放循环
            break;
        }
        AVFrame *frame = optFrame.value();

        // 转换 AVFrame 中的 PCM 数据为 SDL 音频缓冲，写入 sdl 的 AudioBuffer
        int dataSize = av_get_bytes_per_sample((AVSampleFormat)frame->format) * frame->nb_samples * frame->ch_layout.nb_channels;
        sdl.updateAudioBuffer(frame->data[0], dataSize);

        av_frame_free(&frame);

        // 音频数据通常不需要睡眠，由回调驱动
    }
}