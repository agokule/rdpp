#pragma once

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libswscale/swscale.h>
}

#include "ScreenRecorder.hpp"

namespace rdpp::server {

class VideoStreamer2 {
public:
    VideoStreamer2(int width, int height, int fps = 30, int bitrate_kbps = 2'000);
    ~VideoStreamer2();

    bool encode_frame(ImageDataLock& data, int64_t pts = AV_NOPTS_VALUE);

private:
    const AVCodec* codecM;
    AVFrame* frameM;
    AVPacket* pktM;
    AVCodecContext* codec_contextM;
    SwsContext* sws_contextM;
    AVFormatContext* ocM;
    AVStream* video_streamM;
    int64_t frame_countM = 0;
};

} // namespace rdpp::server

