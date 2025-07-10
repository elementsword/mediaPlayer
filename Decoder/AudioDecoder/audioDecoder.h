
#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include "../Decoder/decoder.h"

class AudioDecoder : public Decoder
{
public:
    AudioDecoder();
    ~AudioDecoder();
    bool open(const std::string &url) override;
    bool readFrame(AVFrame *frame) override;  // 读取并解码一帧
    AVCodecContext *context() const override; // context
    void close() override;
    int getSampleRate() const;              // 采样率
    int getChannels() const;                // 声道数
    AVSampleFormat getSampleFormat() const; // 音频样本格式
    uint64_t getChannelLayout() const;

private:
    AVFormatContext *formatCtx; // 媒体文件格式上下文，用于保存整个文件的封装格式信息（如 MP4、TS 等）

    AVCodecContext *codecCtx; // 解码器上下文，保存解码过程中的状态和参数（如分辨率、帧率、像素格式等）

    AVStream *audioStream; // 指向音频流的结构体，保存当前处理的视频流信息（如时间基、帧率、编码参数等）

    int audioStreamIndex; // 视频流在媒体文件中的索引，用于区分多个流（视频/音频/字幕）

    AVPacket *packet; // 压缩数据包，表示读取的一帧视频或音频数据（未解码前的压缩形式）

    SwrContext *swrCtx; // 用于音频重采样/格式转换

    AVFrame *tmpFrame; 
    std::string url;
};
#endif // SDL_RENDERER_H