#include <iostream>
#include "../sdl/sdl.h"
#include "../VideoDecoder/VideoDecoder.h"
#include <fstream>

int main(int argc, char *argv[])
{
    const std::string &url = argv[1];
    std::fstream file(url);
    VideoDecoder videodecoder;
    Sdl sdl;
    videodecoder.open(url);
    int width = videodecoder.getWidth();
    int height = videodecoder.getHeight();
    sdl.init(width, height);

    while (true)
    {
        AVFrame *frame =nullptr;
        videodecoder.readFrame(frame);
        sdl.renderFrame(frame->data, frame->linesize);
    }
    videodecoder.close();
    sdl.cleanup();
}