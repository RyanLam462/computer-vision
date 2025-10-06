#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#ifndef HEADLESS
#include <pangolin/pangolin.h>
#endif
#include <iostream>

int main(int argc, char** argv)
{
    try {
        bool has_display = getenv("DISPLAY") != nullptr;

#ifndef HEADLESS
        pangolin::OpenGlRenderState s_cam;
        pangolin::Handler3D handler(s_cam);
        pangolin::View* d_cam_ptr = nullptr;
        pangolin::GlTexture color_tex, depth_tex;

        if (has_display) {
            std::cout << "[Pangolin] Display detected. Initializing window...\n";
            pangolin::CreateWindowAndBind("Pangolin RGB-D Test", 640, 480);
            glEnable(GL_DEPTH_TEST);

            s_cam = pangolin::OpenGlRenderState(
                pangolin::ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
                pangolin::ModelViewLookAt(0,0,-3, 0,0,0, pangolin::AxisY)
            );

            d_cam_ptr = &pangolin::CreateDisplay()
                .SetBounds(0.0,1.0,0.0,1.0,-640.0f/480.0f)
                .SetHandler(&handler);

            color_tex.Reinitialise(640, 480, GL_RGB);
            depth_tex.Reinitialise(640, 480, GL_LUMINANCE);
        }
#endif

        // ----------------------------
        // RealSense pipeline setup
        // ----------------------------
        rs2::pipeline pipe;
        rs2::config cfg;
        cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
        cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
        pipe.start(cfg);

        std::cout << "[RealSense] Pipeline started successfully.\n";

        cv::Mat last_color, last_depth;

#ifndef HEADLESS
        cv::namedWindow("OpenCV Color Frame", cv::WINDOW_AUTOSIZE);
#endif

        for (int i = 0; i < 10; ++i) {
            rs2::frameset frames = pipe.wait_for_frames();
            rs2::video_frame color_frame = frames.get_color_frame();
            rs2::depth_frame depth_frame = frames.get_depth_frame();

            // Convert to OpenCV Mats
            cv::Mat color(cv::Size(color_frame.get_width(), color_frame.get_height()),
                          CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

            cv::Mat depth(cv::Size(depth_frame.get_width(), depth_frame.get_height()),
                          CV_16UC1, (void*)depth_frame.get_data(), cv::Mat::AUTO_STEP);

            last_color = color.clone();
            last_depth = depth.clone();

            // Print stats
            cv::Scalar mean_color = cv::mean(color);
            double mean_depth = cv::mean(depth)[0];
            std::cout << "[Frame " << i << "] Color mean: " << mean_color
                      << " | Depth mean: " << mean_depth << std::endl;

#ifndef HEADLESS
            // OpenCV live display
            if (has_display) {
                cv::imshow("OpenCV Color Frame", last_color);
                cv::waitKey(1);
            }

            // Pangolin: upload textures
            if (d_cam_ptr) {
                color_tex.Upload(last_color.data, GL_BGR, GL_UNSIGNED_BYTE);

                // Convert depth to 8-bit grayscale for visualization
                cv::Mat depth_visual;
                depth.convertTo(depth_visual, CV_8UC1, 255.0/4000.0); // scale to 0-255
                depth_tex.Upload(depth_visual.data, GL_LUMINANCE, GL_UNSIGNED_BYTE);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                d_cam_ptr->Activate(s_cam);

                // Render color frame
                color_tex.RenderToViewportFlipY();

                pangolin::FinishFrame();
            }
#endif
        }

        std::cout << "[RealSense] Frame capture finished.\n";

#ifndef HEADLESS
        if (has_display) {
            std::cout << "[Pangolin/OpenCV] Entering persistent display loop. Close window to exit.\n";
            while (!pangolin::ShouldQuit()) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                d_cam_ptr->Activate(s_cam);

                if (!last_color.empty()) {
                    color_tex.Upload(last_color.data, GL_BGR, GL_UNSIGNED_BYTE);
                    color_tex.RenderToViewportFlipY();
                }

                cv::imshow("OpenCV Color Frame", last_color);
                cv::waitKey(1);
                pangolin::FinishFrame();
            }
            cv::destroyAllWindows();
        }
#else
        if (!last_color.empty()) cv::imwrite("last_color_frame.png", last_color);
        if (!last_depth.empty()) cv::imwrite("last_depth_frame.png", last_depth);
        std::cout << "[Headless] Saved last_color_frame.png and last_depth_frame.png\n";
#endif

    } catch (const rs2::error & e) {
        std::cerr << "[RealSense ERROR] " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception & e) {
        std::cerr << "[EXCEPTION] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
