#include "VideoDecoder.h"

VideoDecoder::VideoDecoder()
{
}
VideoDecoder::~VideoDecoder()
{
}

bool VideoDecoder::open(const std::string &url)
{
    // 打开媒体文件（如 .mp4、.ts 等），并初始化 AVFormatContext 结构体
    if (avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr) < 0)
    {
        std::cerr << "Failed to open input file: " << url << std::endl;
        return false;
    }

    // 读取媒体文件的流信息（如有多少条流、每条流的编码参数等），填充 formatCtx->streams
    if (avformat_find_stream_info(formatCtx, nullptr) < 0)
    {
        std::cerr << "Failed to find stream info." << std::endl;
        return false;
    }

    // 遍历所有流，找到类型为视频（AVMEDIA_TYPE_VIDEO）的那一条流，并记录其索引
    int videoIndex = -1;
    for (int i = 0; i < formatCtx->nb_streams; i++)
    {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndex = i;
        }
    }

    // 如果找不到视频流，则报错返回
    if (videoIndex < 0)
    {
        std::cerr << "No video stream found." << std::endl;
        return false;
    }

    // 从 formatCtx 中取出对应的视频流的压缩参数（codecpar）
    AVCodecParameters *codecpar = formatCtx->streams[videoIndex]->codecpar;

    // 根据 codec_id 查找对应的解码器（比如 H264 解码器）
    const AVCodec *decoder = avcodec_find_decoder(codecpar->codec_id);
    if (decoder == nullptr)
    {
        std::cerr << "decoder is nullptr." << std::endl;
        return false;
    }

    // 分配并初始化 AVCodecContext（解码器上下文），用于管理解码状态
    codecCtx = avcodec_alloc_context3(decoder);
    if (!codecCtx)
    {
        std::cerr << "Failed to allocate codec context." << std::endl;
        return false;
    }

    // 将 codecpar 中的压缩参数拷贝到 codecCtx 中（必须做，否则解码器无参数）
    if (avcodec_parameters_to_context(codecCtx, codecpar) < 0)
    {
        std::cerr << "Failed to copy codec parameters." << std::endl;
        return false;
    }

    // 打开解码器，准备好接收压缩数据（packet）进行解码
    if (avcodec_open2(codecCtx, decoder, nullptr) < 0)
    {
        std::cerr << "Failed to open codec." << std::endl;
        return false;
    }

    // 分配 AVPacket 结构体，用于读取压缩数据（如 H264 一帧）
    packet = av_packet_alloc();

    // 保存找到的视频流指针，后续可以直接使用它
    videoStream = formatCtx->streams[videoIndex];

    // 更新成员变量，保存视频流索引
    videoStreamIndex = videoIndex;

    return true;
}

int VideoDecoder::getWidth() const
{
    return codecCtx->width;
}
int VideoDecoder::getHeight() const
{
    return codecCtx->height;
}

void VideoDecoder::close()
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
    videoStream = nullptr;
    videoStreamIndex = -1;
}

// 获取当前视频的像素格式
AVPixelFormat VideoDecoder::getPixelFormat() const
{
    return codecCtx ? codecCtx->pix_fmt : AV_PIX_FMT_NONE;
}

bool VideoDecoder::readFrame(AVFrame *frame)
{
    int ret = avcodec_receive_frame(codecCtx, frame);
    if (ret == 0)
        return true; // 成功拿到一帧
    else if (ret == AVERROR(EAGAIN))
        return false; // 需要更多包，先去送包
    else if (ret == AVERROR_EOF)
        return false; // 已解码完
    else
    {
        std::cerr << "Error decoding frame: " << ret << std::endl;
        return false;
    }
}