#include <chrono>
#include <rdpp_server/VideoStreamer.hpp>

#include <rdpp_common/NetworkSerialization.hpp>
#include <ScreenCapture.h>

#include <rdpp_common/Logging.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/time.h>
}

using rdpp::common::log::printdbg;
using rdpp::common::log::printrel;

using rdpp::server::VideoStreamer;

VideoStreamer::VideoStreamer(const std::string& input, const std::string& output)
    : input_file {input}, output_url{output}, running {false} {
}

VideoStreamer::~VideoStreamer() {
    stop();
}

bool VideoStreamer::start() {
    if (running.load())
        return false;

    running = true;
    worker = std::thread(&VideoStreamer::stream_loop, this);
    return true;
}

void VideoStreamer::stop() {
    running = false;
    if (worker.joinable())
        worker.join();
}

void VideoStreamer::stream_loop() {
    while (running) {
        if (!stream_once()) {
            printrel("Stream failed — retrying in 2 seconds...");
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
}

bool VideoStreamer::stream_once() {
    AVFormatContext* in_fmt = nullptr;
    AVFormatContext* out_fmt = nullptr;

    printdbg("Setting up video streamer...");

    if (avformat_open_input(&in_fmt, input_file.c_str(), nullptr, nullptr) < 0) {
        printrel<std::string>("Could not open input file \"{}\".", {input_file});
        return false;
    }

    if (avformat_find_stream_info(in_fmt, nullptr) < 0) {
        printrel("Could not find stream info.");
        avformat_close_input(&in_fmt);
        return false;
    }

    if (avformat_alloc_output_context2(&out_fmt, nullptr, "mpegts", output_url.c_str()) < 0) {
        printrel("Could not create output context.");
        avformat_close_input(&in_fmt);
        return false;
    }

    for (unsigned i = 0; i < in_fmt->nb_streams; i++) {
        AVStream* in_stream = in_fmt->streams[i];
        AVStream* out_stream = avformat_new_stream(out_fmt, nullptr);
        avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        out_stream->codecpar->codec_tag = 0;
    }

    if (!(out_fmt->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&out_fmt->pb, output_url.c_str(), AVIO_FLAG_WRITE) < 0) {
            printrel("Could not open output URL.");
            avformat_close_input(&in_fmt);
            avformat_free_context(out_fmt);
            return false;
        }
    }

    if (avformat_write_header(out_fmt, nullptr) < 0) {
        printrel("Error writing header.");
        avformat_close_input(&in_fmt);
        avio_closep(&out_fmt->pb);
        avformat_free_context(out_fmt);
        return false;
    }

    printdbg("Finished set-up video streamer...");

    int64_t start_time = av_gettime();
    AVPacket pkt;

    while (running && av_read_frame(in_fmt, &pkt) >= 0) {
        AVStream* in_stream  = in_fmt->streams[pkt.stream_index];
        AVStream* out_stream = out_fmt->streams[pkt.stream_index];

        // Real-time pacing: emulate `-re`
        AVRational time_base = in_stream->time_base;
        int64_t pts_time = av_rescale_q(pkt.dts, time_base, AVRational{1, AV_TIME_BASE});
        int64_t now_time = av_gettime() - start_time;
        if (pts_time > now_time)
            av_usleep(pts_time - now_time);

        // Rescale and send packet
        pkt.pts = av_rescale_q_rnd(pkt.pts, time_base, out_stream->time_base,
                                    (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, time_base, out_stream->time_base,
                                    (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, time_base, out_stream->time_base);
        pkt.pos = -1;

        if (av_interleaved_write_frame(out_fmt, &pkt) < 0) {
            printrel("Error writing frame.");
            av_packet_unref(&pkt);
            break;
        }

        av_packet_unref(&pkt);
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }

    av_write_trailer(out_fmt);
    avformat_close_input(&in_fmt);
    if (!(out_fmt->oformat->flags & AVFMT_NOFILE))
        avio_closep(&out_fmt->pb);
    avformat_free_context(out_fmt);

    // Finished one loop of the video
    printdbg("Loop finished, restarting...");
    return true;
}

