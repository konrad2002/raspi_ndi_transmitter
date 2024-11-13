#include <csignal>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>
#include <Processing.NDI.Advanced.h>
#include <opencv2/opencv.hpp>  // Use OpenCV to capture from /dev/video0

// Video frame structure for NDI
struct video_frame {
    const uint8_t* p_data;
    const uint8_t* p_extra;
    int64_t dts;
    int64_t pts;
    uint32_t data_size;
    uint32_t extra_size;
    bool is_keyframe;
};

// Video data setup for scatter-gather
struct video_send_data {
    NDIlib_compressed_packet_t packet;
    NDIlib_video_frame_v2_t frame;
    std::vector<uint8_t*> scatter_data;
    std::vector<int> scatter_data_size;
};

static std::atomic<bool> exit_loop(false);
static void sigint_handler(int) { exit_loop = true; }

void setup_frame(const cv::Mat& frame, int frame_num, video_send_data& dst_data) {
    // Update timecode (mock here, real application needs proper handling)
    dst_data.frame.timecode = frame_num * 10000000 / 30;  // assuming 30 fps
    dst_data.packet.version = sizeof(NDIlib_compressed_packet_t);
    dst_data.packet.pts = dst_data.frame.timecode;
    dst_data.packet.dts = dst_data.frame.timecode;
    dst_data.packet.flags = NDIlib_compressed_packet_t::flags_keyframe;
    dst_data.packet.data_size = frame.total() * frame.elemSize();
    dst_data.packet.extra_data_size = 0;
    dst_data.packet.fourCC = NDIlib_compressed_FourCC_type_H264;

    // Clear scatter-gather lists and add frame data
    dst_data.scatter_data.clear();
    dst_data.scatter_data_size.clear();
    dst_data.scatter_data.push_back((uint8_t*)&dst_data.packet);
    dst_data.scatter_data_size.push_back((int)sizeof(NDIlib_compressed_packet_t));
    dst_data.scatter_data.push_back((uint8_t*)frame.data);
    dst_data.scatter_data_size.push_back(dst_data.packet.data_size);
}

int main(int argc, char* argv[]) {
    // Catch interrupt
    signal(SIGINT, sigint_handler);

    // Initialize NDI
    if (!NDIlib_initialize()) return 0;

    // Set up NDI sender
    NDIlib_send_create_t NDI_send_description;
    NDI_send_description.p_ndi_name = "Raspi Video HX";
    NDI_send_description.clock_video = true;
    NDI_send_description.clock_audio = true;

    NDIlib_send_instance_t pNDI_send = NDIlib_send_create_v2(&NDI_send_description, nullptr);
    if (!pNDI_send) return 0;

    // Open video capture on /dev/video0
    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
    if (!cap.isOpened()) {
        fprintf(stderr, "Error: Unable to open video capture from /dev/video0\n");
        return -1;
    }

    // Set up frame parameters
    const int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    const int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    const int num_frames = 2;  // Double buffer for async
    video_send_data send_data[num_frames];

    // Loop to capture and send video frames
    for (int frame_num = 0, send_frame = 0; !exit_loop; frame_num++) {
        cv::Mat frame;
        if (!cap.read(frame)) {
            fprintf(stderr, "Error: Unable to read frame from /dev/video0\n");
            break;
        }

        // Prepare frame for sending
        setup_frame(frame, frame_num, send_data[send_frame]);
        NDIlib_video_frame_v2_t& ndi_frame = send_data[send_frame].frame;
        NDIlib_frame_scatter_t scatter = { send_data[send_frame].scatter_data.data(), send_data[send_frame].scatter_data_size.data() };

        // Send asynchronously
        NDIlib_send_send_video_scatter_async(pNDI_send, &ndi_frame, &scatter);

	std::cout << "First byte of frame data: " << (int)frame.data[0] << std::endl;
        std::cout << "Sending frame: " << frame.cols << "x" << frame.rows << std::endl;

        // Alternate between buffers
        send_frame = (send_frame + 1) % num_frames;
    }

    // Ensure async operations complete
    NDIlib_send_send_video_scatter_async(pNDI_send, nullptr, nullptr);

    // Release resources
    cap.release();
    NDIlib_send_destroy(pNDI_send);
    NDIlib_destroy();

    return 0;
}

