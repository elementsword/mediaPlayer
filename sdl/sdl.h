#ifndef SDL_H
#define SDL_H
#include <iostream>
#include <fstream>
#include "../PlayerControl/playercontrol.h"

extern "C"
{
#include "SDL2/SDL.h"
}
class Sdl
{
public:
    bool init(int w, int h, const std::string &title = "Video");
    void renderFrame(uint8_t *data[3], int linesize[3]);
    void cleanup();
    // 新增：处理事件，返回是否请求退出
    void processEvents(PlayerControl &control);

    ~Sdl();

private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    int width = 0;
    int height = 0;
    // 黑白
    bool enableBw = false;
    bool paused =false;
};

#endif