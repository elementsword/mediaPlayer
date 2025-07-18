#include <iostream>
#include "../sdl/sdl.h"
#include "../Decoder/decoder.h"
#include "../Decoder/AudioDecoder/audioDecoder.h"
#include "../Decoder/VideoDecoder/videoDecoder.h"
#include <fstream>

int main(int argc, char *argv[])
{
    PlayerControl control;
    const std::string url = argv[1];
    std::cout << url << std::endl;
    VideoDecoder *videodecoder = new VideoDecoder();
    AudioDecoder *audiodecoder = new AudioDecoder();
    Decoder *decoder1 = videodecoder;
    Decoder *decoder2 = audiodecoder;
    Sdl sdl;
    decoder1->open(url);
    decoder2->open(url);
    int width = videodecoder->getWidth();
    int height = videodecoder->getHeight();

    sdl.initVideo(width, height);
    sdl.initAudio(audiodecoder->getSampleRate(), audiodecoder->getChannels(), av_get_bytes_per_sample(audiodecoder->getSampleFormat()));
    while (control.getState() != PlayerState::Quit)
    {
        if (control.getState() == PlayerState::Playing)
        {
            // 1. 处理视频帧
            AVFrame *videoFrame = av_frame_alloc();
            if (videodecoder->readFrame(videoFrame)) // 读到视频帧才渲染
            {
                sdl.renderFrame(videoFrame->data, videoFrame->linesize);
            }
            av_frame_free(&videoFrame);

            // 2. 处理音频帧
            AVFrame *audioFrame = av_frame_alloc();
            if (audiodecoder->readFrame(audioFrame)) // 读到音频帧才更新缓冲
            {
                // 音频数据是平面格式还是交错格式，data[0]通常就是音频PCM数据指针，size按你解码器决定
                int dataSize = av_samples_get_buffer_size(
                    nullptr,                            // linesize, 通常为 nullptr 即可
                    audioFrame->ch_layout.nb_channels,  // 通道数（比如 2）
                    audioFrame->nb_samples,             // 样本数（每通道）
                    (AVSampleFormat)audioFrame->format, // 样本格式（如 AV_SAMPLE_FMT_S16）
                    1                                   // 对齐方式，通常为1
                );

                sdl.updateAudioBuffer(audioFrame->data[0], dataSize);
            }
            av_frame_free(&audioFrame);
        }

        sdl.processEvents(control);
        SDL_Delay(40); // 控制循环频率
    }
    decoder1->close();
    sdl.cleanup();
}