#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

// Camera intrinsics for Intel RealSense D435i (typical values)
// You may need to adjust these based on your specific camera calibration
const float fx = 614.789f;  // focal length x
const float fy = 615.269f;  // focal length y
const float cx = 321.558f;  // principal point x
const float cy = 242.509f;  // principal point y

// Depth scale (meters per depth unit)
const float depth_scale = 0.001f;

struct Frame {
    cv::Mat color;
    cv::Mat depth;
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    double timestamp;
};

// Convert pixel + depth to 3D point
cv::Point3f deproject(const cv::Point2f& pixel, float depth_value) {
    float z = depth_value * depth_scale;
    float x = (pixel.x - cx) * z / fx;
    float y = (pixel.y - cy) * z / fy;
    return cv::Point3f(x, y, z);
}

// Estimate pose using PnP RANSAC
bool estimatePose(const std::vector<cv::Point3f>& points3d_prev,
                  const std::vector<cv::Point2f>& points2d_curr,
                  cv::Mat& rvec, cv::Mat& tvec) {
    
    if (points3d_prev.size() < 10) {
        return false;
    }

    // Camera intrinsic matrix
    cv::Mat K = (cv::Mat_<double>(3, 3) << fx, 0, cx,
                                            0, fy, cy,
                                            0, 0, 1);
    
    // Use PnP RANSAC to estimate pose
    std::vector<int> inliers;
    bool success = cv::solvePnPRansac(
        points3d_prev,
        points2d_curr,
        K,
        cv::Mat(),  // no distortion
        rvec,
        tvec,
        false,
        100,        // iterations
        8.0,        // reprojection error threshold
        0.99,       // confidence
        inliers
    );

    if (success && inliers.size() >= 10) {
        std::cout << "  [PnP] Inliers: " << inliers.size() 
                  << "/" << points3d_prev.size() << std::endl;
        return true;
    }
    
    return false;
}

// Convert rotation vector and translation to transformation matrix
Eigen::Matrix4d toMatrix4d(const cv::Mat& rvec, const cv::Mat& tvec) {
    cv::Mat R;
    cv::Rodrigues(rvec, R);
    
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T(0, 0) = R.at<double>(0, 0);
    T(0, 1) = R.at<double>(0, 1);
    T(0, 2) = R.at<double>(0, 2);
    T(1, 0) = R.at<double>(1, 0);
    T(1, 1) = R.at<double>(1, 1);
    T(1, 2) = R.at<double>(1, 2);
    T(2, 0) = R.at<double>(2, 0);
    T(2, 1) = R.at<double>(2, 1);
    T(2, 2) = R.at<double>(2, 2);
    T(0, 3) = tvec.at<double>(0);
    T(1, 3) = tvec.at<double>(1);
    T(2, 3) = tvec.at<double>(2);
    
    return T;
}

void printPose(const Eigen::Matrix4d& pose, int frame_num) {
    Eigen::Vector3d translation = pose.block<3, 1>(0, 3);
    Eigen::Matrix3d rotation = pose.block<3, 3>(0, 0);
    
    // Convert rotation matrix to Euler angles (roll, pitch, yaw)
    Eigen::Vector3d euler = rotation.eulerAngles(0, 1, 2);
    
    std::cout << "\n=== Frame " << frame_num << " Camera Pose ===" << std::endl;
    std::cout << "Translation (x, y, z): " 
              << translation.x() << ", " 
              << translation.y() << ", " 
              << translation.z() << " meters" << std::endl;
    std::cout << "Rotation (roll, pitch, yaw): "
              << euler.x() * 180.0 / M_PI << ", "
              << euler.y() * 180.0 / M_PI << ", "
              << euler.z() * 180.0 / M_PI << " degrees" << std::endl;
    std::cout << "Transformation Matrix:\n" << pose << std::endl;
}

int main(int argc, char** argv) {
    try {
        std::cout << "=== Basic Visual Odometry Tracker ===" << std::endl;
        std::cout << "Initializing RealSense camera..." << std::endl;

        // Initialize RealSense pipeline
        rs2::pipeline pipe;
        rs2::config cfg;
        cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
        cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
        
        rs2::pipeline_profile profile = pipe.start(cfg);
        
        // Get depth scale
        auto depth_sensor = profile.get_device().first<rs2::depth_sensor>();
        float actual_depth_scale = depth_sensor.get_depth_scale();
        std::cout << "Depth scale: " << actual_depth_scale << " meters/unit" << std::endl;

        // Create ORB feature detector
        cv::Ptr<cv::ORB> orb = cv::ORB::create(1000);  // 1000 features
        
        // BFMatcher for feature matching
        cv::BFMatcher matcher(cv::NORM_HAMMING);
        
        Frame prev_frame, curr_frame;
        Eigen::Matrix4d cumulative_pose = Eigen::Matrix4d::Identity();
        
        bool is_first_frame = true;
        int frame_count = 0;

        std::cout << "\nStarting visual odometry tracking...\n" << std::endl;

        // Main tracking loop
        while (true) {
            // 1. Grab synchronized RGB and Depth frames
            rs2::frameset frames = pipe.wait_for_frames();
            rs2::video_frame color_frame = frames.get_color_frame();
            rs2::depth_frame depth_frame = frames.get_depth_frame();
            
            curr_frame.color = cv::Mat(
                cv::Size(color_frame.get_width(), color_frame.get_height()),
                CV_8UC3, 
                (void*)color_frame.get_data(), 
                cv::Mat::AUTO_STEP
            ).clone();
            
            curr_frame.depth = cv::Mat(
                cv::Size(depth_frame.get_width(), depth_frame.get_height()),
                CV_16UC1,
                (void*)depth_frame.get_data(),
                cv::Mat::AUTO_STEP
            ).clone();
            
            curr_frame.timestamp = color_frame.get_timestamp();
            
            // Convert to grayscale for feature detection
            cv::Mat gray;
            cv::cvtColor(curr_frame.color, gray, cv::COLOR_BGR2GRAY);
            
            // 2. Extract ORB features from RGB frame
            orb->detectAndCompute(gray, cv::Mat(), curr_frame.keypoints, curr_frame.descriptors);
            
            std::cout << "Frame " << frame_count << ": Detected " 
                      << curr_frame.keypoints.size() << " features" << std::endl;
            
            if (is_first_frame) {
                // Initialize first frame
                prev_frame = curr_frame;
                is_first_frame = false;
                printPose(cumulative_pose, frame_count);
                frame_count++;
                
                // Small delay to allow camera to stabilize
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            // 3. Track features from previous frame to current frame
            if (prev_frame.descriptors.empty() || curr_frame.descriptors.empty()) {
                std::cout << "  [Warning] No descriptors in one of the frames, skipping..." << std::endl;
                prev_frame = curr_frame;
                frame_count++;
                continue;
            }
            
            std::vector<cv::DMatch> matches;
            matcher.match(prev_frame.descriptors, curr_frame.descriptors, matches);
            
            // Filter matches by distance
            double min_dist = 100.0;
            for (const auto& match : matches) {
                if (match.distance < min_dist) {
                    min_dist = match.distance;
                }
            }
            
            std::vector<cv::DMatch> good_matches;
            for (const auto& match : matches) {
                if (match.distance <= std::max(2.0 * min_dist, 30.0)) {
                    good_matches.push_back(match);
                }
            }
            
            std::cout << "  [Matching] Good matches: " << good_matches.size() 
                      << "/" << matches.size() << std::endl;
            
            if (good_matches.size() < 10) {
                std::cout << "  [Warning] Not enough good matches, skipping pose estimation..." << std::endl;
                prev_frame = curr_frame;
                frame_count++;
                continue;
            }
            
            // 4. Estimate camera's new pose
            // Build 3D-2D correspondences
            std::vector<cv::Point3f> points3d_prev;
            std::vector<cv::Point2f> points2d_curr;
            
            for (const auto& match : good_matches) {
                cv::Point2f pt_prev = prev_frame.keypoints[match.queryIdx].pt;
                cv::Point2f pt_curr = curr_frame.keypoints[match.trainIdx].pt;
                
                // Get depth at previous frame keypoint
                int x = static_cast<int>(pt_prev.x);
                int y = static_cast<int>(pt_prev.y);
                
                if (x >= 0 && x < prev_frame.depth.cols && 
                    y >= 0 && y < prev_frame.depth.rows) {
                    
                    float depth_val = prev_frame.depth.at<uint16_t>(y, x);
                    
                    if (depth_val > 0 && depth_val < 10000) {  // valid depth (< 10m)
                        cv::Point3f point3d = deproject(pt_prev, depth_val);
                        points3d_prev.push_back(point3d);
                        points2d_curr.push_back(pt_curr);
                    }
                }
            }
            
            std::cout << "  [3D-2D] Valid correspondences: " << points3d_prev.size() << std::endl;
            
            // Estimate pose using PnP
            cv::Mat rvec, tvec;
            if (estimatePose(points3d_prev, points2d_curr, rvec, tvec)) {
                // Convert to transformation matrix
                Eigen::Matrix4d relative_pose = toMatrix4d(rvec, tvec);
                
                // Update cumulative pose (T_new = T_old * T_relative^-1)
                cumulative_pose = cumulative_pose * relative_pose.inverse();
                
                // 5. Print estimated camera pose
                printPose(cumulative_pose, frame_count);
            } else {
                std::cout << "  [Warning] Pose estimation failed, keeping previous pose..." << std::endl;
            }
            
            // Update for next iteration
            prev_frame = curr_frame;
            frame_count++;
            
            // Optional: limit frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Optional: break after certain number of frames for testing
            // if (frame_count >= 50) break;
        }
        
        std::cout << "\nTracking finished." << std::endl;
        
    } catch (const rs2::error& e) {
        std::cerr << "[RealSense ERROR] " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "[EXCEPTION] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
