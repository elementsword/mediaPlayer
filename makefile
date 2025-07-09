CXX := g++
PKG_FFMPEG_CFLAGS := $(shell pkg-config --cflags libavformat libavcodec libavutil libswscale libavdevice)
PKG_FFMPEG_LDFLAGS := $(shell pkg-config --libs libavformat libavcodec libavutil libswscale libavdevice)

CXXFLAGS := -Wall -g -std=c++17 -I/usr/include/SDL2 -I./VideoDecoder -I./sdl $(PKG_FFMPEG_CFLAGS)
LDFLAGS := -lSDL2 $(PKG_FFMPEG_LDFLAGS)


# 指定根目录列表（你想编译的源码根目录）
DIRS := main Decoder sdl PlayerControl

# 利用shell和find，递归查找这些目录里的所有cpp文件
SRCS := $(foreach dir,$(DIRS),$(shell find $(dir) -type f -name "*.cpp"))

# 生成对应的目标文件列表，将.cpp后缀替换为.o
OBJS := $(SRCS:.cpp=.o)

TARGET := video_player

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
