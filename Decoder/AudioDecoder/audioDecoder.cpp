#include "audioDecoder.h"

AudioDecoder::AudioDecoder() : formatCtx(nullptr), codecCtx(nullptr), audioStream(nullptr), audioStreamIndex(-1), packet(nullptr), swrCtx(nullptr), frame(av_frame_alloc())
{
}

AudioDecoder::~AudioDecoder()
{
    close();
}

bool AudioDecoder::open(const std::string &url)
{

    // 创建 AVChannelLayout
    AVChannelLayout in_layout, out_layout;

    // 初始化输入声道布局（以立体声为例）
    av_channel_layout_default(&in_layout, 2); // 2 通道
    av_channel_layout_from_mask(&in_layout, getChannelLayout());

    // 初始化输出声道布局
    av_channel_layout_default(&out_layout, 2);
    av_channel_layout_from_mask(&out_layout, AV_CH_LAYOUT_STEREO);
    if (avformat_open_input(&formatCtx, url.c_str(), nullptr, nullptr))
    {
        std::cerr << "Failed to open" << url << std::endl;
        return false;
    }

    if (avformat_find_stream_info(formatCtx, nullptr) < 0)
    {
        std::cerr << "Failed to find stream info." << std::endl;
        return false;
    }

    // 遍历所有流，找到类型为视频（AVMEDIA_TYPE_AUDIO）的那一条流，并记录其索引
    for (uint i = 0; i < this->formatCtx->nb_streams; i++)
    {
        std::cout << this->formatCtx->streams[i]->codecpar->codec_type << std::endl;
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            std::cout << "找到了" << std::endl;
            break;
        }
    }

    // 如果找不到音频流，则报错返回
    if (audioStreamIndex < 0)
    {
        std::cerr << "No audio stream found." << std::endl;
        return false;
    }

    // 从 formatCtx 中取出对应的音频流的压缩参数（codecpar）
    AVCodecParameters *codecpar = formatCtx->streams[audioStreamIndex]->codecpar;

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

    // 保存找到的音频流指针，后续可以直接使用它
    audioStream = formatCtx->streams[audioStreamIndex];

    swrCtx = swr_alloc();
    if (!swrCtx)
    {
        std::cerr << "Failed to allocate SwrContext." << std::endl;
        return false;
    }

    swr_alloc_set_opts2(&swrCtx, &out_layout, AV_SAMPLE_FMT_S16, 44100, &in_layout, codecCtx->sample_fmt, codecCtx->sample_rate, 0, NULL);

    int ret = swr_init(swrCtx);
    if (ret < 0)
    {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cerr << "Failed to initialize SwrContext: " << errbuf << " (" << ret << ")" << std::endl;
        swr_free(&swrCtx);
        return false;
    }
    return true;
}

AVFrame *AudioDecoder::decode(const AVPacket &pkt)
{
    av_frame_unref(frame);
    if (avcodec_send_packet(codecCtx, &pkt) < 0)
    {
        return nullptr;
    }

    if (avcodec_receive_frame(codecCtx, frame) < 0)
    {
        return nullptr;
    }

    if (!swrCtx)
    {
        std::cerr << "swrCtx is NULL!" << std::endl;
        return nullptr;
    }

    // 1. 计算输出缓冲区最大容量（样本数）
    int64_t delay = swr_get_delay(swrCtx, 44100);
    int max_output_samples = av_rescale_rnd(delay + frame->nb_samples,
                                            44100,
                                            codecCtx->sample_rate,
                                            AV_ROUND_UP);

    // 2. 设置 convertedFrame 参数
    if (!initConvertedFrame(&convertedFrame, AV_SAMPLE_FMT_S16, 44100, 2, max_output_samples))
    {
        return nullptr;
    }

    // 计算目标输出样本数（有些场景需要 swr_get_delay + av_rescale）
    int convertedSamples = swr_convert(
        swrCtx,
        convertedFrame->data, convertedFrame->nb_samples, // 输出数据和样本数
        (const uint8_t **)frame->data, frame->nb_samples  // 输入数据和样本数
    );

    if (convertedSamples < 0)
    {
        // 转换失败
        return nullptr;
    }

    convertedFrame->nb_samples = convertedSamples;
    av_frame_unref(frame);
    return convertedFrame; // 拿到一帧成功
}

AVCodecContext *AudioDecoder::context() const
{
    if (codecCtx)
    {
        return codecCtx;
    }
    else
    {
        return nullptr;
    }
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

int AudioDecoder::getSampleRate() const
{
    return codecCtx ? codecCtx->sample_rate : 0;
}
int AudioDecoder::getChannels() const
{
    if (codecCtx)
    {
        return codecCtx->ch_layout.nb_channels;
    }
    return -1;
}
AVSampleFormat AudioDecoder::getSampleFormat() const
{
    return codecCtx ? codecCtx->sample_fmt : AV_SAMPLE_FMT_NONE;
}

uint64_t AudioDecoder::getChannelLayout() const
{
    if (codecCtx && codecCtx->ch_layout.nb_channels > 0)
    {
        return (codecCtx && codecCtx->ch_layout.order == AV_CHANNEL_ORDER_NATIVE)
                   ? codecCtx->ch_layout.u.mask
                   : 0;
    }
    return 0; // 返回 0 表示未知或非标准布局
}

// 重置initframe
bool AudioDecoder::initConvertedFrame(AVFrame **convertFrame, AVSampleFormat sampleFmt,
                                      int sampleRate,
                                      int nbChannels,
                                      int nbSamples)
{
    *convertFrame = av_frame_alloc();
    // 空
    if (!*convertFrame)
    {
        std::cerr << "Failed to allocate convertFrame" << std::endl;
        return false;
    }

    (*convertFrame)->format = sampleFmt;
    (*convertFrame)->sample_rate = sampleRate;
    (*convertFrame)->nb_samples = nbSamples;
    // 设置声道布局
    av_channel_layout_default(&(*convertFrame)->ch_layout, nbChannels);
    // 分配 buffer（如果之前没有或者 nb_samples 变了）
    if (av_frame_get_buffer(*convertFrame, 0) < 0)
    {
        std::cerr << "Failed to allocate tmpFrame buffer" << std::endl;
        av_frame_free(convertFrame); // ✅ 释放已分配但未完全初始化的 frame
        return false;
    }

    return true;
}