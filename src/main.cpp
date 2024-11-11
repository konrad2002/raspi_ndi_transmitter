#include <Processing.NDI.Lib.h>
#include <iostream>
#include <opencv2/opencv.hpp>

int main() {
    // Initialize NDI
    if (!NDIlib_initialize()) {
        std::cerr << "Cannot run NDI." << std::endl;
        return -1;
    }

    // Setup NDI send settings
    NDIlib_send_create_t NDI_send_create_desc;
    NDI_send_create_desc.p_ndi_name = "Raspberry Pi Camera Stream";
    NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc);

    if (!pNDI_send) {
        std::cerr << "Failed to create NDI send instance." << std::endl;
        return -1;
    }

    // Open the camera
    cv::VideoCapture cap("/dev/video0");
    if (!cap.isOpened()) {
        std::cerr << "Error opening video stream" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (cap.read(frame)) {
        // Encode frame to H.264 or handle as required

        // Send over NDI
        NDIlib_video_frame_v2_t NDI_video_frame;
        NDI_video_frame.xres = frame.cols;
        NDI_video_frame.yres = frame.rows;
        NDI_video_frame.FourCC = NDIlib_FourCC_type_BGRX;
        NDI_video_frame.p_data = frame.data;
        NDI_video_frame.line_stride_in_bytes = frame.step;

        NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);
    }

    // Clean up
    NDIlib_send_destroy(pNDI_send);
    NDIlib_destroy();

    return 0;
}
