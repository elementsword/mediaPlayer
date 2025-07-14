#ifndef SDL_H
#define SDL_H
#include <iostream>
#include <fstream>
#include <queue>
#include <condition_variable>
#include "../PlayerControl/playercontrol.h"

extern "C"
{
#include "SDL2/SDL.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libpostproc/postprocess.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
class Sdl
{
public:
    bool initVideo(int w, int h, const std::string &title = "Video");
    // 初始化音频播放系统
    bool initAudio(int sampleRate, int channels, int bytesPerSample);

    // 向音频缓冲区添加PCM数据（由解码线程调用）
    void updateAudioBuffer(uint8_t *data, int size);

    double getAudioClock() const ;
    void renderFrame(uint8_t *data[3], int linesize[3]);
    // 获取当前音频时钟

    void cleanup();
    // 新增：处理事件，返回是否请求退出
    void processEvents(PlayerControl &control);
    Sdl();
    ~Sdl();

private:
    static void audioCallback(void *userdata, Uint8 *stream, int len); // SDL拉数据时回调

    // 视频
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    int width = 0;
    int height = 0;
    // 黑白
    bool enableBw = false;
    bool paused = false;

    // 音频相关
    uint8_t *audioBufferData = nullptr; // 指向PCM数据
    int audioBufferSize = 0;            // 剩余PCM字节数
    double audioClock = 0;              // 当前音频播放时间（秒）
    int audioSampleRate = 0;
    int audioChannels = 0;
    int audioBytesPerSample = 0;

    // 视频
    //  视频缓冲区
    std::queue<AVFrame *> videoQueue;
    std::mutex videoMutex;
    std::condition_variable videoCond;
    const size_t maxVideoQueueSize = 20; // 可调节
    mutable std::mutex audioMutex;               // 保护 audioBufferData 和 audioBufferSize 的线程安全
};

#endif