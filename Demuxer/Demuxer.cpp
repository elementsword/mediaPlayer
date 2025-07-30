#include "Demuxer.h"

// 分发

Demuxer::Demuxer() : formatCtx(nullptr), audioStreamIndex(-1), videoStreamIndex(-1), demuxThread(), audioPacketQueue(), videoPacketQueue()
{
}
Demuxer::~Demuxer()
{
    // 通知队列不再接受新数据，并唤醒所有可能在等待中的线程（如 pop 操作）
    audioPacketQueue.stop();
    videoPacketQueue.stop();

    // 等待 demux 解复用线程执行完毕，确保不会有线程在访问 AVFormatContext 等共享资源
    if (demuxThread.joinable())
        demuxThread.join();

    // 释放 AVFormatContext 占用的资源（关闭文件、释放内部缓冲等）
    if (formatCtx)
    {
        avformat_close_input(&formatCtx); // 安全关闭输入文件
        formatCtx = nullptr;              // 避免悬空指针
    }
}

void Demuxer::start()
{
    quitFlag = false;
    // 启动线程 类函数 需要传参
    demuxThread = std::thread(&Demuxer::run, this);
}
void Demuxer::stop()
{
    quitFlag = true;
    audioPacketQueue.stop();
    videoPacketQueue.stop();
}

bool Demuxer::open(const std::string url)
{
    videoStreamIndex = -1;
    audioStreamIndex = -1;
    // 打开媒体文件（如 .mp4、.ts 等），并初始化 AVFormatContext 结构体
    if (avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr))
    {
        std::cerr << "Failed to open" << url << std::endl;
        return false;
    }

    // 读取媒体文件的流信息（如有多少条流、每条流的编码参数等），填充 formatCtx->streams
    if (avformat_find_stream_info(formatCtx, nullptr) < 0)
    {
        std::cerr << "Failed to find stream info." << std::endl;
        return false;
    }

    // 遍历所有流，找到类型为视频（AVMEDIA_TYPE_VIDEO）的那一条流，并记录其索引
    for (uint i = 0; i < this->formatCtx->nb_streams; i++)
    {
        std::cout << this->formatCtx->streams[i]->codecpar->codec_type << std::endl;
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            std::cout << "找到了" << std::endl;
            break;
        }
    }

    // 如果找不到视频流，则报错返回
    if (videoStreamIndex < 0)
    {
        std::cerr << "No video stream found." << std::endl;
        return false;
    }

    // 遍历所有流，找到类型为音频（AVMEDIA_TYPE_AUDIO）的那一条流，并记录其索引
    for (uint i = 0; i < this->formatCtx->nb_streams; i++)
    {
        std::cout << this->formatCtx->streams[i]->codecpar->codec_type << std::endl;
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            std::cout << "找到了" << std::endl;
            break;
        }
    }

    // 如果找不到音频流，则报错返回
    if (audioStreamIndex < 0)
    {
        std::cerr << "No audio stream found." << std::endl;
        return false;
    }

    return true;
}

ThreadSafeQueue<AVPacket*> &Demuxer::getAudioQueue()
{
    return audioPacketQueue;
}
ThreadSafeQueue<AVPacket*> &Demuxer::getVideoQueue()
{
    return videoPacketQueue;
}
int Demuxer::getAudioStreamIndex() const
{
    return audioStreamIndex;
}
int Demuxer::getVideoStreamIndex() const
{
    return videoStreamIndex;
}

bool Demuxer::isOpened() const
{
    return formatCtx != nullptr;
}
MediaType Demuxer::getMediaType() const
{
    bool hasVideo = videoStreamIndex >= 0;
    bool hasAudio = audioStreamIndex >= 0;

    if (hasVideo && hasAudio)
        return MediaType::AudioVideo;
    else if (hasVideo)
        return MediaType::VideoOnly;
    else if (hasAudio)
        return MediaType::AudioOnly;
    else
        return MediaType::Unknown;
}

void Demuxer::run()
{
    AVPacket *pkt = av_packet_alloc();

    while (!quitFlag)
    {
        int ret = av_read_frame(formatCtx, pkt);
        if (ret < 0)
        {
            // 读完或错误，退出循环
            break;
        }

        if (pkt->stream_index == videoStreamIndex)
        {
            videoPacketQueue.push(pkt);
        }
        else if (pkt->stream_index == audioStreamIndex)
        {
            audioPacketQueue.push(pkt);
        }
        else
        {
            // 不是音频也不是视频，释放包
            av_packet_unref(pkt);
        }
    }

    // 结束时停止队列，通知消费者
    audioPacketQueue.stop();
    videoPacketQueue.stop();
}