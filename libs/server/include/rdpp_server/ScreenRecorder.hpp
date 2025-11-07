#include <ScreenCapture.h>
#include <vector>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavdevice/avdevice.h>
    #include <libavfilter/avfilter.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/opt.h>
}

namespace rdpp::server {

class ScreenRecorder {
    friend class ImageDataLock;
    std::shared_ptr<SL::Screen_Capture::ICaptureConfiguration<SL::Screen_Capture::ScreenCaptureCallback>> capture_configM;
    std::vector<uint8_t> image_dataM;
    bool image_data_in_useM = false;
    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> frame_grabberM;

    // note that this function runs in a different thread
    void on_frame_changed(const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor);

public:
    ScreenRecorder(const SL::Screen_Capture::Monitor& monitor);

    void start();
    inline const ImageDataLock get_image_data();
};



class ImageDataLock {
    std::vector<uint8_t>& image_dataM;
    ScreenRecorder& recorder;

public:
    ImageDataLock(std::vector<uint8_t>& img_data, ScreenRecorder& rec);
    ~ImageDataLock();

    std::vector<uint8_t>& operator->();
};

} // namespace rdpp::server

