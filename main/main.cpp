#include <iostream>
#include "../sdl/sdl.h"
#include "../Decoder/decoder.h"
#include "../Decoder/AudioDecoder/audioDecoder.h"
#include "../Decoder/VideoDecoder/videoDecoder.h"
#include <fstream>
int main(int argc, char *argv[])
{
    PlayerControl control;
    const std::string url = argv[1];
    std::cout << url << std::endl;
    VideoDecoder *videodecoder=new VideoDecoder();
    Decoder *decoder =videodecoder;
    Sdl sdl;
    decoder->open(url);
    int width = videodecoder->getWidth();
    int height = videodecoder->getHeight();
    sdl.init(width, height);
    while (control.getState() != PlayerState::Quit)
    {
        if (control.getState() == PlayerState::Playing)
        {
            AVFrame *frame = av_frame_alloc();
            decoder->readFrame(frame);
            sdl.renderFrame(frame->data, frame->linesize);
            
        }
        sdl.processEvents(control);
        SDL_Delay(40); // 等待10毫秒，控制播放速度
    }
    decoder->close();
    sdl.cleanup();
}