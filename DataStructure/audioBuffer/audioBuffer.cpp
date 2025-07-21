#include "audioBuffer.h"

AudioBuffer::AudioBuffer()
{
}

// 写入数据到缓冲区
void AudioBuffer::write(const uint8_t *data, size_t size)
{
    
    std::lock_guard<std::mutex> lock(mutex);
    buffer.insert(buffer.end(), data, data + size);
    writePos += size;
}

// 从缓冲区读取数据 尽可能多的读

size_t AudioBuffer::read(uint8_t *out, size_t size)
{
    std::unique_lock<std::mutex> lock(mutex);

    size_t available = writePos - readPos;
    size_t toRead = std::min(size, available);

    if (toRead > 0)
    {
        std::copy(buffer.begin() + readPos, buffer.begin() + readPos + toRead, out);
        readPos += toRead;

        // 如果数据都读完了，清理空间
        if (readPos == writePos)
        {
            buffer.clear();
            readPos = writePos = 0;
        }
    }

    return toRead;
}

// 获取当前缓冲数据大小
size_t AudioBuffer::size() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return writePos - readPos;
}

// 清空缓冲区
void AudioBuffer::clear()
{
    std::lock_guard<std::mutex> lock(mutex);
    buffer.clear();
    writePos = readPos = 0;
}