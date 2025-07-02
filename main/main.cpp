#include <iostream>
#include "../sdl/sdl.h"
#include "../VideoDecoder/VideoDecoder.h"
#include <fstream>
int main(int argc, char *argv[])
{
    PlayerControl control;
    const std::string url = argv[1];
    std::cout << url << std::endl;
    VideoDecoder videodecoder;
    Sdl sdl;
    videodecoder.open(url);
    int width = videodecoder.getWidth();
    int height = videodecoder.getHeight();
    sdl.init(width, height);
    while (control.getState() != PlayerState::Quit)
    {
        if (control.getState() == PlayerState::Playing)
        {
            AVFrame *frame = av_frame_alloc();
            videodecoder.readFrame(frame);
            sdl.renderFrame(frame->data, frame->linesize);
            
        }
        sdl.processEvents(control);
        SDL_Delay(40); // 等待10毫秒，控制播放速度
    }
    videodecoder.close();
    sdl.cleanup();
}