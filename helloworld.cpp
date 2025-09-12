#include <iostream>
#include <string>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/avutil.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/pixdesc.h>
    #include <libavutil/samplefmt.h>
}

void printVideoInfo(const char* filename) {
    AVFormatContext* formatContext = nullptr;
    
    // Open input file
    if (avformat_open_input(&formatContext, filename, nullptr, nullptr) != 0) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    // Retrieve stream information
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Error: Could not find stream information" << std::endl;
        avformat_close_input(&formatContext);
        return;
    }
    
    // Print general file information
    std::cout << "=== File Information ===" << std::endl;
    std::cout << "Filename: " << filename << std::endl;
    std::cout << "Format: " << formatContext->iformat->long_name << std::endl;
    std::cout << "Duration: " << formatContext->duration / AV_TIME_BASE << " seconds" << std::endl;
    std::cout << "Bitrate: " << formatContext->bit_rate << " bps" << std::endl;
    std::cout << "Number of streams: " << formatContext->nb_streams << std::endl;
    std::cout << std::endl;
    
    // Iterate through streams
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        AVStream* stream = formatContext->streams[i];
        AVCodecParameters* codecParams = stream->codecpar;
        const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
        
        std::cout << "=== Stream " << i << " ===" << std::endl;
        std::cout << "Type: ";
        
        switch (codecParams->codec_type) {
            case AVMEDIA_TYPE_VIDEO:
                std::cout << "Video" << std::endl;
                std::cout << "Codec: " << (codec ? codec->long_name : "Unknown") << std::endl;
                std::cout << "Resolution: " << codecParams->width << "x" << codecParams->height << std::endl;
                std::cout << "Pixel Format: " << av_get_pix_fmt_name((AVPixelFormat)codecParams->format) << std::endl;
                std::cout << "Bitrate: " << codecParams->bit_rate << " bps" << std::endl;
                if (stream->avg_frame_rate.den != 0) {
                    double fps = av_q2d(stream->avg_frame_rate);
                    std::cout << "Frame Rate: " << fps << " fps" << std::endl;
                }
                break;
                
            case AVMEDIA_TYPE_AUDIO:
                std::cout << "Audio" << std::endl;
                std::cout << "Codec: " << (codec ? codec->long_name : "Unknown") << std::endl;
                std::cout << "Sample Rate: " << codecParams->sample_rate << " Hz" << std::endl;
                std::cout << "Channels: " << codecParams->ch_layout.nb_channels << std::endl;
                std::cout << "Sample Format: " << av_get_sample_fmt_name((AVSampleFormat)codecParams->format) << std::endl;
                std::cout << "Bitrate: " << codecParams->bit_rate << " bps" << std::endl;
                break;
                
            case AVMEDIA_TYPE_SUBTITLE:
                std::cout << "Subtitle" << std::endl;
                std::cout << "Codec: " << (codec ? codec->long_name : "Unknown") << std::endl;
                break;
                
            default:
                std::cout << "Other" << std::endl;
                break;
        }
        std::cout << std::endl;
    }
    
    // Clean up
    avformat_close_input(&formatContext);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <video_file>" << std::endl;
        return 1;
    }
    
    // Initialize FFmpeg (not needed in newer versions but good practice)
    av_log_set_level(AV_LOG_ERROR);
    
    printVideoInfo(argv[1]);
    
    return 0;
}
