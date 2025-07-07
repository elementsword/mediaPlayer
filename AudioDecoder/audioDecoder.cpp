#include "audioDecoder.h"

bool AudioDecoder::open(const std::string &url)
{
}
bool AudioDecoder::readFrame(AVFrame *frame)
{
}
int AudioDecoder::getSampleRate() const
{
    return codecCtx ? codecCtx->sample_rate : 0;
}
int AudioDecoder::getChannels() const
{
    return codecCtx ? codecCtx-> : 0;
}
AVSampleFormat AudioDecoder::getSampleFormat() const
{
}
void AudioDecoder::close()
{
    // 释放 AVPacket（压缩数据包）
    if (packet)
    {
        av_packet_free(&packet); // 释放后自动将 packet 置为 nullptr
        packet = nullptr;
    }

    // 释放解码器上下文
    if (codecCtx)
    {
        avcodec_free_context(&codecCtx); // 释放后自动将 codecCtx 置为 nullptr
        codecCtx = nullptr;
    }

    // 关闭输入文件并释放 AVFormatContext
    if (formatCtx)
    {
        avformat_close_input(&formatCtx); // 内部会释放并置空
        formatCtx = nullptr;
    }

    // 清理其他状态
    audioStream = nullptr;
    audioStreamIndex = -1;
}
AudioDecoder::~AudioDecoder()
{
    close();
}
AudioDecoder::AudioDecoder() : formatCtx(nullptr), codecCtx(nullptr), audioStream(nullptr), audioStreamIndex(-1), packet(nullptr),swrCtx(nullptr)
{
}