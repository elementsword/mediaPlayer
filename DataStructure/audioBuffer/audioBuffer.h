#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <cstdint>
class AudioBuffer
{
public:
    AudioBuffer();

    // 写入数据到缓冲区
    void write(const uint8_t *data, size_t size);

    // 从缓冲区读取数据
    // 返回实际读取的字节数（可能小于请求）
    size_t read(uint8_t *out, size_t size);

    // 获取当前缓冲数据大小
    size_t size() const;

    // 清空缓冲区
    void clear();

private:
    std::vector<uint8_t> buffer;
    mutable std::mutex mutex;
    std::condition_variable cond;

    size_t readPos = 0;
    size_t writePos = 0;
};
#endif