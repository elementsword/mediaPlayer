#include <iostream>
#include "../sdl/sdl.h"
#include "../Decoder/decoder.h"
#include "../Decoder/AudioDecoder/audioDecoder.h"
#include "../Decoder/VideoDecoder/videoDecoder.h"
#include <fstream>
const int outSampleRate = 44100;
const int outChannels = 2;
const int outBytesPerSample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "请输入播放的视频地址:" << std::endl;
        return -1;
    }
    PlayerControl control;
    const std::string url = argv[1];
    std::cout << url << std::endl;

    VideoDecoder *videodecoder = new VideoDecoder();
    AudioDecoder *audiodecoder = new AudioDecoder();

    Sdl sdl;
    videodecoder->open(url);
    audiodecoder->open(url);
    int width = videodecoder->getWidth();
    int height = videodecoder->getHeight();
    AVFrame *videoFrame = av_frame_alloc();
    AVFrame *audioFrame = av_frame_alloc();
    sdl.initVideo(width, height);
    sdl.initAudio(outSampleRate, outChannels, AV_SAMPLE_FMT_S16);

    av_frame_free(&videoFrame);
    av_frame_free(&audioFrame);
    sdl.cleanup();
}