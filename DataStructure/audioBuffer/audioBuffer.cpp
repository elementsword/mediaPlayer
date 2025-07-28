#include "audioBuffer.h"

AudioBuffer::AudioBuffer()
{
}

// 写入数据到缓冲区
void AudioBuffer::write(const uint8_t *data, size_t size)
{

    std::lock_guard<std::mutex> lock(mutex);

    if (buffer.size() < writePos + size)
    {
        size_t newSize = buffer.size() == 0 ? writePos + size : buffer.size() * 2;
        if (newSize < writePos + size)
            newSize = writePos + size;
        buffer.resize(newSize);
    }
    // 直接覆盖数据
    std::copy(data, data + size, buffer.begin() + writePos);
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
    writePos = readPos = 0;
}
