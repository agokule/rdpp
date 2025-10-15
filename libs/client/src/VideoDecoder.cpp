#include <rdpp_client/VideoDecoder.hpp>

#include <atomic>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <iostream>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

VideoDecoder::VideoDecoder(const std::string& url) : url_(url) {}

VideoDecoder::~VideoDecoder() { stop(); }

bool VideoDecoder::start() {
    if (running_) return false;
    avformat_network_init();

    if (avformat_open_input(&fmt_ctx_, url_.c_str(), nullptr, nullptr) < 0) {
        std::cerr << "Failed to open input\n";
        return false;
    }
    if (avformat_find_stream_info(fmt_ctx_, nullptr) < 0) {
        std::cerr << "Failed to find stream info\n";
        return false;
    }

    for (unsigned i = 0; i < fmt_ctx_->nb_streams; i++) {
        if (fmt_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index_ = i;
            break;
        }
    }
    if (video_stream_index_ == -1) {
        std::cerr << "No video stream found\n";
        return false;
    }

    AVCodecParameters* codecpar = fmt_ctx_->streams[video_stream_index_]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    codec_ctx_ = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx_, codecpar);
    avcodec_open2(codec_ctx_, codec, nullptr);

    codec_ctx_->thread_count = std::thread::hardware_concurrency();
    codec_ctx_->thread_type = FF_THREAD_SLICE;

    running_ = true;
    decode_thread_ = std::thread(&VideoDecoder::decodeLoop, this);
    return true;
}

void VideoDecoder::stop() {
    if (!running_) return;
    running_ = false;
    if (decode_thread_.joinable()) decode_thread_.join();
    if (codec_ctx_) avcodec_free_context(&codec_ctx_);
    if (fmt_ctx_) avformat_close_input(&fmt_ctx_);
    avformat_network_deinit();
}

std::shared_ptr<AVFrame> VideoDecoder::getLatestFrame() {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    return latest_frame_;
}

void VideoDecoder::decodeLoop() {
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    while (running_) {
        if (av_read_frame(fmt_ctx_, pkt) < 0) {
            av_seek_frame(fmt_ctx_, video_stream_index_, 0, AVSEEK_FLAG_BACKWARD);
            continue;
        }

        if (pkt->stream_index == video_stream_index_) {
            if (avcodec_send_packet(codec_ctx_, pkt) == 0) {
                while (avcodec_receive_frame(codec_ctx_, frame) == 0) {
                    auto copy = std::shared_ptr<AVFrame>(av_frame_clone(frame), [](AVFrame* f){ av_frame_free(&f); });
                    {
                        std::lock_guard<std::mutex> lock(frame_mutex_);
                        latest_frame_ = copy;
                    }
                }
            }
        }
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    av_frame_free(&frame);
}

