#ifndef DEMUXER_H
#define DEMUXER_H

// Demuxer.h
enum class MediaType
{
    AudioOnly,
    VideoOnly,
    AudioVideo,
    Unknown
};
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
// 分发
class Demuxer
{
public:
    Demuxer();
    ~Demuxer();

    void start();
    void stop();
    bool open(const std::string url);

    ThreadSafeQueue<AVPacket *> &getAudioQueue();
    ThreadSafeQueue<AVPacket *> &getVideoQueue();
    int getAudioStreamIndex() const;
    int getVideoStreamIndex() const;
    bool isOpened() const;
    MediaType getMediaType() const;

private:
    void run();
    AVFormatContext *formatCtx;
    int audioStreamIndex;
    int videoStreamIndex;
    std::atomic<bool> quitFlag;
    std::thread demuxThread;
    ThreadSafeQueue<AVPacket *> audioPacketQueue;
    ThreadSafeQueue<AVPacket *> videoPacketQueue;
};
#endif