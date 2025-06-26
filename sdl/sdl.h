#ifndef SDL_H
#define SDL_H
#include <iostream>
#include <fstream>
extern "C"
{
#include "SDL2/SDL.h"
}
class Sdl
{
public:
    bool init(int w, int h, const std::string &title = "Video");
    void renderFrame(uint8_t *data[3], int linesize[3]);
    bool handleEvents();
    void cleanup();
    ~Sdl();

private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    int width = 0;
    int height = 0;
    //黑白
    bool enableBw =false;
};

#endif