#include <rdpp_server/VideoStreamer2.hpp>
#include <rdpp_common/Logging.hpp>

using namespace rdpp::server;
using namespace rdpp::common;

constexpr std::string_view default_codec = "libvpx-vp9";

VideoStreamer2::VideoStreamer2(int width, int height, int fps, int bitrate_kbps) {
    codecM = avcodec_find_encoder_by_name(default_codec.data());
    log::release_assert(codecM, std::format("Codec \"{}\" not found", default_codec));

    codec_contextM = avcodec_alloc_context3(codecM);
    log::release_assert(codecM, std::format("Could not allocate codec context for {}", default_codec));

    pktM = av_packet_alloc();
    log::release_assert(codecM, std::format("Could not allocate packet for {}", default_codec));

    codec_contextM->bit_rate = bitrate_kbps * 1'000;
    codec_contextM->width = width;
    codec_contextM->height = height;
    codec_contextM->time_base = AVRational {1, fps};
    codec_contextM->framerate = AVRational {fps, 1};
    codec_contextM->pix_fmt = AV_PIX_FMT_YUV420P;

    auto ret = avcodec_open2(codec_contextM, codecM, NULL);
    log::release_assert(!(ret < 0), "Could not open the codec context");

    frameM = av_frame_alloc();
    log::release_assert(frameM, "Could not allocate the video frame");
    frameM->format = codec_contextM->pix_fmt;
    frameM->width  = codec_contextM->width;
    frameM->height = codec_contextM->height;

    ret = av_frame_get_buffer(frameM, 0);
    log::release_assert(!(ret < 0), "Could not allocate the video frame data");

    // Converts RGB24 input -> YUV420P output
    sws_contextM = sws_getContext(
        width, height, AV_PIX_FMT_RGB24,
        width, height, AV_PIX_FMT_YUV420P,
        SWS_BILINEAR, nullptr, nullptr, nullptr);
    log::release_assert(sws_contextM, "Could not allocate the sws context");
}

VideoStreamer2::~VideoStreamer2() {
}

bool VideoStreamer2::encode_frame(ImageDataLock& data, int64_t pts) {
    const uint8_t* src_data[1] = { data->data() };

}

