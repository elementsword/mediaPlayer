#include "sdl.h"
bool Sdl::init(int w, int h, const std::string &title)
{
    // 初始化 SDL 视频子系统，如果失败返回 false
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 保存视频宽高
    width = w;
    height = h;

    // 创建一个 SDL 窗口，显示在屏幕中央，宽高为传入的 w 和 h，窗口可见且支持缩放
    window = SDL_CreateWindow(title.c_str(),
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 基于窗口创建一个渲染器，使用硬件加速
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 创建一个 YUV420P (IYUV 格式) 的纹理，用于将视频帧上传后显示
    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_IYUV,        // YUV420P 格式
                                SDL_TEXTUREACCESS_STREAMING, // 每帧更新像素数据
                                width, height);
    if (!texture)
    {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 初始化成功
    return true;
}
// 一帧
void Sdl::renderFrame(uint8_t *data[3], int linesize[3])
{
    uint8_t *yPlane = data[0];
    uint8_t *uPlane = data[1];
    uint8_t *vPlane = data[2];
    if (!texture || !renderer)
        return;
    // 将U和V分量设置为128（灰度值）
    // 获取 Y U V 三个分量的指针 Y = x*y U,V=x*y/4

    int uvSize = width * height / 4;
    if (enableBw)
    {
        memset(uPlane, 128, uvSize);
        memset(vPlane, 128, uvSize);
    }

    // 将 YUV 数据复制到 SDL 纹理中（注意这里不需要做格式转换，SDL 支持 IYUV）
    SDL_UpdateYUVTexture(texture,
                         nullptr,             // 全部更新
                         yPlane, linesize[0], // Y 分量
                         uPlane, linesize[1], // U 分量
                         vPlane, linesize[2]  // V 分量
    );

    // 清空渲染器的上一帧内容
    SDL_RenderClear(renderer);

    // 将纹理复制到渲染器
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    // 实际显示到窗口
    SDL_RenderPresent(renderer);
}
bool Sdl::handleEvents()
{
    return true;
}
void Sdl::cleanup()
{
    // 销毁纹理资源，释放显存
    if (texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    // 销毁渲染器
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    // 销毁窗口
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    // 关闭 SDL 系统（只需调用一次，通常在程序结束时）
    SDL_Quit();
}
Sdl::~Sdl()
{
}