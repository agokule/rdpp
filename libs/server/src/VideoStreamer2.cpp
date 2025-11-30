#include <rdpp_server/VideoStreamer2.hpp>
#include <rdpp_common/Logging.hpp>

extern "C" {
    #include <libavutil/opt.h>
}

using namespace rdpp::server;
using namespace rdpp::common;

constexpr std::string_view default_codec = "libopenh264";

VideoStreamer2::VideoStreamer2(int width, int height, int fps, int bitrate_kbps) {
    void* iter_state = NULL;
    while (auto codec = av_codec_iterate(&iter_state)) {
        if (av_codec_is_encoder(codec))
            log::printdbg<const char*>("Codec name: {}", {codec->name});
    }

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

    // Allocate the output context for MPEG‑TS over UDP
    avformat_alloc_output_context2(&ocM, nullptr, "rtp", "rtp://127.0.0.1:9999");
    log::release_assert(ocM, "Could not allocate output context");

    // Create a new video stream in that context
    video_streamM = avformat_new_stream(ocM, nullptr);
    log::release_assert(video_streamM, "Could not create stream");

    // Copy codec parameters from the encoder to the stream
    char errbuf[64];
    ret = avcodec_parameters_from_context(video_streamM->codecpar, codec_contextM);
    log::release_assert(
        ret >= 0, std::format("Failed to copy codec parameters: {}",
                              av_make_error_string(errbuf, sizeof(errbuf), ret)));
    video_streamM->time_base = codec_contextM->time_base;

    // Open the output URL (UDP)
    if (!(ocM->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ocM->pb, "rtp://127.0.0.1:9999", AVIO_FLAG_WRITE);
      log::release_assert(
          ret >= 0, std::format("Could not open output URL: {}",
                                av_make_error_string(errbuf, sizeof(errbuf), ret)));
    }

    // Write the container header
    ret = avformat_write_header(ocM, nullptr);
    log::release_assert(
        ret >= 0, std::format("Error writing header: {}",
                              av_make_error_string(errbuf, sizeof(errbuf), ret)));

    char sdp_file[1000] {};
    av_sdp_create(&ocM, 1, sdp_file, sizeof(sdp_file));

    log::printrel(sdp_file);
}

VideoStreamer2::~VideoStreamer2() {
    // Send a null frame to flush the encoder
    avcodec_send_frame(codec_contextM, nullptr);
    while (avcodec_receive_packet(codec_contextM, pktM) == 0) {
        pktM->stream_index = video_streamM->index;
        av_interleaved_write_frame(ocM, pktM);
        av_packet_unref(pktM);
    }

    // Write trailer and clean up
    av_write_trailer(ocM);
    avio_closep(&ocM->pb);
    avformat_free_context(ocM);

    // free other FFmpeg objects (frame, packet, codec_context, sws_context)
    av_frame_free(&frameM);
    av_packet_free(&pktM);
    avcodec_free_context(&codec_contextM);
    sws_freeContext(sws_contextM);
}

bool VideoStreamer2::encode_frame(ImageDataLock& data, int64_t pts) {
    if (!data->data())
        return false;

    int src_linesize[1] = { data.get_pixel_width() * static_cast<int>(sizeof(RGBPixel)) };

    char errbuf[64]{};

    int ret = av_frame_make_writable(frameM);
    log::release_assert(ret >= 0, av_make_error_string(errbuf, sizeof(errbuf), ret));

    const uint8_t* src_data[1] = { data->data() };
    sws_scale(sws_contextM, src_data, src_linesize, 0, data.get_pixel_height(),
              frameM->data, frameM->linesize);

    if (pts == AV_NOPTS_VALUE)
        pts = frame_countM++;
    frameM->pts = pts;

    ret = avcodec_send_frame(codec_contextM, frameM);
    log::release_assert(ret >= 0, av_make_error_string(errbuf, sizeof(errbuf), ret));
    if (ret < 0)
        return false;

    while ((ret = avcodec_receive_packet(codec_contextM, pktM)) == 0) {
        pktM->stream_index = video_streamM->index;

        // Rescale timestamps from codec timebase to stream timebase
        av_packet_rescale_ts(pktM, codec_contextM->time_base, video_streamM->time_base);

        ret = av_interleaved_write_frame((ocM), pktM);
        log::release_assert(
            ret >= 0,
            std::format("av_interleaved_write_frame failed: {}",
                        av_make_error_string(errbuf, sizeof(errbuf), ret)));
        if (ret < 0)
            return false;

        av_packet_unref(pktM);
    }

    if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
      log::printrel<char *>("Warning: avcodec_receive_packet returned {}",
                            {av_make_error_string(errbuf, sizeof(errbuf), ret)});

    return true;
}
