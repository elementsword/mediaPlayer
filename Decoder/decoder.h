#ifndef DECODER_H
#define DECODER_H
#include <iostream>
#include <fstream>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libpostproc/postprocess.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

class Decoder
{
public:
    virtual ~Decoder() = default;

    /// 打开解码器，绑定某个 AVStream
    virtual bool open(const std::string &url) = 0;
    virtual bool readFrame(AVFrame *frame) = 0;

    /// 返回 AVCodecContext
    virtual AVCodecContext *context() const = 0;

    /// 关闭解码器并释放资源
    virtual void close() = 0;
};

#endif