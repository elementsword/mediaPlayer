
#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include "../Decoder/decoder.h"

class VideoDecoder
{
public:
    VideoDecoder();
    ~VideoDecoder();
    bool open(const std::string &url) ;
    AVFrame *decode(const AVPacket &pkt); // 读取并解码一帧
    AVCodecContext *context() const ;
    void close() ;
    int getWidth() const;
    int getHeight() const;
    AVPixelFormat getPixelFormat() const;
    bool initConvertedFrame(AVFrame **convertFrame,
                            AVPixelFormat pixFmt,
                            int width,
                            int height); // 初始化initframe

private:
    AVFormatContext *formatCtx; // 媒体文件格式上下文，用于保存整个文件的封装格式信息（如 MP4、TS 等）

    AVCodecContext *codecCtx; // 解码器上下文，保存解码过程中的状态和参数（如分辨率、帧率、像素格式等）

    AVStream *videoStream; // 指向视频流的结构体，保存当前处理的视频流信息（如时间基、帧率、编码参数等）

    int videoStreamIndex; // 视频流在媒体文件中的索引，用于区分多个流（视频/音频/字幕）

    AVPacket *packet; // 压缩数据包，表示读取的一帧视频或音频数据（未解码前的压缩形式）

    SwsContext *swsCtx = nullptr; // ⬅️ 色彩空间转换器

    AVFrame *convertedFrame;

    AVFrame *frame;

    std::string url;
};
#endif //