#include <string>
#include <memory>
#include <thread>
#include <mutex>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

namespace rdpp::client {

class VideoDecoder {
public:
    VideoDecoder(const std::string& url);
    ~VideoDecoder();

    // return true if successful, false otherwise
    bool start();
    void stop();

    // Returns pointer to latest frame (thread-safe copy)
    std::shared_ptr<AVFrame> getLatestFrame();

private:
    void decodeLoop();

    std::string url_;
    std::atomic<bool> running_{false};
    std::thread decode_thread_;

    AVFormatContext* fmt_ctx_ = nullptr;
    AVCodecContext* codec_ctx_ = nullptr;
    int video_stream_index_ = -1;

    std::mutex frame_mutex_;
    std::shared_ptr<AVFrame> latest_frame_;
};

}

