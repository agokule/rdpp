#include <rdpp_server/ScreenRecorder.hpp>
#include <rdpp_common/Logging.hpp>
#include <vector>

using namespace rdpp::server;
using namespace rdpp::common;

using SL::Screen_Capture::Width;
using SL::Screen_Capture::Height;

// taken from the screen_capture_lite example
void extract_and_convert_to_RGB(const SL::Screen_Capture::Image &img, std::vector<uint8_t>& dst) {
    // TODO: Make proper assertion macros in rdpp_common
    assert(dst.size() >= static_cast<size_t>(SL::Screen_Capture::Width(img) * SL::Screen_Capture::Height(img) * sizeof(RGBPixel)));
    const SL::Screen_Capture::ImageBGRA* imgsrc = StartSrc(img);
    auto imgdist = dst.begin();
    for (auto h = 0; h < Height(img); h++) {
        auto startimgsrc = imgsrc;
        for (auto w = 0; w < Width(img); w++) {
            *imgdist++ = imgsrc->R;
            *imgdist++ = imgsrc->G;
            *imgdist++ = imgsrc->B;
            // ignore the alpha value
            imgsrc++;
        }
        imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
    }
}

ScreenRecorder::ScreenRecorder(const SL::Screen_Capture::Monitor& monitor) {
    capture_configM = SL::Screen_Capture::CreateCaptureConfiguration(
        [monitor](){
            return std::vector{monitor};
        }
    )->onFrameChanged(std::bind(&ScreenRecorder::on_frame_changed, this,
                                std::placeholders::_1,
                                std::placeholders::_2));

}

void ScreenRecorder::start() {
    frame_grabberM = capture_configM->start_capturing();
}

void ScreenRecorder::on_frame_changed(const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor) {
    auto size = Width(img) * Height(img) * sizeof(RGBPixel);

    std::unique_lock<std::shared_mutex> lock(mtxM);
    image_dataM.resize(size);
    extract_and_convert_to_RGB(img, image_dataM);
}

RGBPixel ScreenRecorder::read_pixel(unsigned idx) {
    std::shared_lock lock(mtxM);
    unsigned starting_idx = idx * sizeof(RGBPixel);
    return {
        image_dataM.at(starting_idx),
        image_dataM.at(starting_idx + 1),
        image_dataM.at(starting_idx + 2)
    };
}

ImageDataLock::ImageDataLock(ScreenRecorder& rec)
    : lockM(rec.mtxM), image_dataM(&rec.image_dataM) {

}

