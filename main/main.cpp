#include <iostream>
#include "../sdl/sdl.h"
#include "../Decoder/decoder.h"
#include "../Decoder/AudioDecoder/audioDecoder.h"
#include "../Decoder/VideoDecoder/videoDecoder.h"
#include "../Controller/controller.h"
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

    Controller *controller = new Controller();

    controller->open(url);
    controller->start();
    
}