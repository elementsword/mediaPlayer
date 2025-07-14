#include "sdl.h"
bool Sdl::initVideo(int w, int h, const std::string &title)
{
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

bool Sdl::initAudio(int sampleRate, int channels, int bytesPerSample)
{
    // 保存参数，供回调和时钟计算用
    audioSampleRate = sampleRate;
    audioChannels = channels;
    audioBytesPerSample = bytesPerSample;
    // 创建并清零一个 SDL_AudioSpec 结构体，用于配置音频设备参数
    SDL_AudioSpec spec;
    SDL_zero(spec);

    spec.freq = sampleRate;             // 设置采样率（每秒采样数）
    spec.channels = channels;           // 设置通道数（例如 2 表示立体声）
    spec.format = AUDIO_S16SYS;         // 设置音频格式，这里是平台字节序的 16-bit 整型（匹配 AV_SAMPLE_FMT_S16）
    spec.samples = 1024;                // 每次 SDL 回调请求的样本数（这个值影响延迟和稳定性）
    spec.callback = Sdl::audioCallback; // 设置音频回调函数，由 SDL 自动定时调用
    spec.userdata = this;               // 将当前对象指针传递给回调函数，以便访问成员变量

    // 打开音频设备
    if (SDL_OpenAudio(&spec, nullptr) < 0)
    {
        std::cerr << "SDL_OpenAudio failed: " << SDL_GetError() << std::endl;
        return false;
    }

    // 启动音频播放
    SDL_PauseAudio(0); // 0 表示取消暂停，开始播放
    return true;
}
// 临时
void Sdl::updateAudioBuffer(uint8_t *data, int size)
{
    std::lock_guard<std::mutex> lock(audioMutex);

    // 如果之前的 buffer 还没消费完，先释放
    if (audioBufferData)
    {
        av_free(audioBufferData);
        audioBufferData = nullptr;
        audioBufferSize = 0;
    }

    // 分配新的缓冲区并复制数据
    audioBufferData = (uint8_t *)av_malloc(size);
    if (audioBufferData)
    {
        memcpy(audioBufferData, data, size);
        audioBufferSize = size;
    }
}

double Sdl::getAudioClock() const
{
    std::lock_guard<std::mutex> lock(audioMutex);
    return audioClock;
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

void Sdl::processEvents(PlayerControl &control)
{
    SDL_Event event;
    // 一次视频

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            control.setState(PlayerState::Quit);
            return;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_SPACE)
            {
                if (control.getState() == PlayerState::Playing)
                {
                    control.setState(PlayerState::Paused);
                }
                else
                {
                    control.setState(PlayerState::Playing);
                }
                SDL_SetWindowTitle(window, (control.getState() == PlayerState::Playing) ? "SDL2-YUV播放器 正在播放" : "SDL2-YUV播放器 已暂停");
            }
            break;
        default:
            break;
        }
    }
}

Sdl::Sdl()
{
    // 初始化 SDL 视频子系统，如果失败返回 false
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    }
}

Sdl::~Sdl()
{
}

// 这是因为 audioCallback 是一个静态函数（或者说普通的 C 风格回调函数），它没有绑定到某个类实例，不能直接访问类的 this 指针。
void Sdl::audioCallback(void *userdata, Uint8 *stream, int len)
{
    Sdl *self = static_cast<Sdl *>(userdata);
    std::lock_guard<std::mutex> lock(self->audioMutex);

    if (self->audioBufferSize <= 0 || !self->audioBufferData)
    {
        // 没数据就静音
        SDL_memset(stream, 0, len);
        return;
    }
    // copy
    int copyLen = std::min(len, self->audioBufferSize);
    SDL_memcpy(stream, self->audioBufferData, copyLen);

    // 移动缓冲区指针和大小
    self->audioBufferData += copyLen;
    self->audioBufferSize -= copyLen;

    // 更新音频时钟（秒）
    // 计算此次复制的音频样本数：
    // copyLen 是字节数，除以 (声道数 * 每个样本字节数) 得到样本数
    int samples = copyLen / (self->audioChannels * self->audioBytesPerSample);
    // 根据样本数和采样率计算此次音频数据对应的时间长度（秒）
    // 音频时钟累加这段时间，方便音画同步
    self->audioClock += (double)samples / self->audioSampleRate;

    // 如果数据耗尽，释放（你也可以做成环形缓冲）
    if (self->audioBufferSize <= 0)
    {
        av_free(self->audioBufferData);
        self->audioBufferData = nullptr;
        self->audioBufferSize = 0;
    }
    return;
}