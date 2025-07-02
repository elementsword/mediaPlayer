#include <iostream>
#include "../sdl/sdl.h"
#include "../VideoDecoder/VideoDecoder.h"
#include <fstream>

int main(int argc, char *argv[])
{
    const std::string url = argv[1];
    std::cout << url << std::endl;
    VideoDecoder videodecoder;
    Sdl sdl;
    videodecoder.open(url);
    int width = videodecoder.getWidth();
    int height = videodecoder.getHeight();
    sdl.init(width, height);

    while (true)
    {
        AVFrame *frame = av_frame_alloc();
        videodecoder.readFrame(frame);
        sdl.renderFrame(frame->data, frame->linesize);
    }
    videodecoder.close();
    sdl.cleanup();
}