#pragma once

#include <thread>
#include <atomic>
#include <string>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavutil/time.h>
}

namespace rdpp::server {

class VideoStreamer {
public:
    VideoStreamer(const std::string& input, const std::string& output);
    ~VideoStreamer();
    bool start();
    void stop();

private:
    std::string input_file;
    std::string output_url;
    std::thread worker;
    std::atomic<bool> running;

    void stream_loop();
    bool stream_once();
};

}

