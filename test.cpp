#include <iostream>
extern "C" {
#include <libavformat/avformat.h>
}

bool testOpenFile(const char* filename) {
    AVFormatContext* fmtCtx = nullptr;
    int ret = avformat_open_input(&fmtCtx, filename, nullptr, nullptr);
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cerr << "open_input failed: " << errbuf << std::endl;
        return false;
    }

    ret = avformat_find_stream_info(fmtCtx, nullptr);
    if (ret < 0) {
        std::cerr << "Failed to find stream info." << std::endl;
        avformat_close_input(&fmtCtx);
        return false;
    }

    for (unsigned i = 0; i < fmtCtx->nb_streams; i++) {
        auto codecpar = fmtCtx->streams[i]->codecpar;
        if (!codecpar) {
            std::cerr << "codecpar nullptr at stream " << i << std::endl;
            continue;
        }
        std::cout << "Stream " << i << " codec_type = " << (int)codecpar->codec_type << std::endl;
    }

    avformat_close_input(&fmtCtx);
    return true;
}

int main() {
    const char* filename = "/root/learn/ffmpeg/audioFile/Titanic.ts";
    bool ok = testOpenFile(filename);
    std::cout << (ok ? "Test succeeded" : "Test failed") << std::endl;
    return ok ? 0 : 1;
}