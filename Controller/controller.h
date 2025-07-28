#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <thread>
#include <atomic>
#include "../Demuxer/Demuxer.h"
#include "../Decoder/AudioDecoder/audioDecoder.h"
#include "../Decoder/VideoDecoder/videoDecoder.h"
#include "../DataStructure/ThreadSafeQueue/ThreadSafeQueue.h"

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

    std::thread audioThread;
    std::thread videoThread;
    std::atomic<bool> quitFlag;

    bool audioThreadRunning = false;
    bool videoThreadRunning = false;
    void audioLoop();
    void videoLoop();
};

#endif