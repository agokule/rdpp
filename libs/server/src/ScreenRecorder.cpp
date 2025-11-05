#include <rdpp_server/ScreenRecorder.hpp>
#include <rdpp_common/Logging.hpp>
#include <vector>

using namespace rdpp::server;
using namespace rdpp::common;

using SL::Screen_Capture::Width;
using SL::Screen_Capture::Height;

ScreenRecorder::ScreenRecorder(const SL::Screen_Capture::Monitor& monitor) {
    capture_configM = SL::Screen_Capture::CreateCaptureConfiguration(
        [&monitor](){
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
    log::printdbg("Frame difference detected!");

    auto size = Width(img) * Height(img) * sizeof(SL::Screen_Capture::ImageBGRA);
    image_dataM.reserve(size);
}

