#include <ScreenCapture.h>
#include <shared_mutex>
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

struct RGBPixel {
    uint8_t r, g, b;
};

class ScreenRecorder {
    friend class ImageDataLock;
    std::shared_ptr<SL::Screen_Capture::ICaptureConfiguration<SL::Screen_Capture::ScreenCaptureCallback>> capture_configM;
    std::vector<uint8_t> image_dataM;
    mutable std::shared_mutex mtxM;
    bool image_data_in_useM = false;
    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> frame_grabberM;

    // note that this function runs in a different thread
    void on_frame_changed(const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor);

public:
    ScreenRecorder(const SL::Screen_Capture::Monitor& monitor);

    void start();

    RGBPixel read_pixel(unsigned idx);

};

class ImageDataLock {
    const std::vector<uint8_t>* image_dataM;
    std::shared_lock<std::shared_mutex> lockM;

public:
    ImageDataLock(ScreenRecorder& rec);
    ~ImageDataLock() = default;
 
    // Prevent copying
    ImageDataLock(const ImageDataLock&) = delete;
    ImageDataLock& operator=(const ImageDataLock&) = delete;

    const std::vector<uint8_t>* operator->() const { return image_dataM; };
    const std::vector<uint8_t>& operator*() const { return *image_dataM; };
};

} // namespace rdpp::server
