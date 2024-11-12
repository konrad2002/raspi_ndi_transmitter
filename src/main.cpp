#include <cstddef>
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
    NDI_send_create_desc.p_ndi_name = "Raspi HX";
    NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc);

    if (!pNDI_send) {
        std::cerr << "Failed to create NDI send instance." << std::endl;
        return -1;
    }

    // Open the camera
    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video stream" << std::endl;
        return -1;
    }

    int target_width = 1280;
    int target_height = 720;

    cv::Mat frame;
    while (cap.read(frame)) {
        // Resize the frame
        cv::Mat resized_frame;
        cv::resize(frame, resized_frame, cv::Size(target_width, target_height));

        // Convert to BGRX format for NDI if needed
        cv::cvtColor(resized_frame, resized_frame, cv::COLOR_BGR2BGRA);

//	cv::imwrite("debug_frame.png", resized_frame);

        // Update the NDI frame struct with the new dimensions
        NDIlib_video_frame_v2_t NDI_video_frame;
        NDI_video_frame.xres = resized_frame.cols;
        NDI_video_frame.yres = resized_frame.rows;
        NDI_video_frame.FourCC = NDIlib_FourCC_type_BGRA;
        NDI_video_frame.p_data = resized_frame.data;
	NDI_video_frame.frame_rate_N = 30;  // Frame rate (use your desired FPS)
        NDI_video_frame.frame_rate_D = 1;
	NDI_video_frame.timecode = 0;
        NDI_video_frame.line_stride_in_bytes = resized_frame.step[0];


	std::cout << "First byte of frame data: " << (int)resized_frame.data[0] << std::endl;


        NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);

	std::cout << "Sending frame: " << resized_frame.cols << "x" << resized_frame.rows << std::endl;

    }

    // Clean up
    NDIlib_send_destroy(pNDI_send);
    NDIlib_destroy();

    return 0;
}
