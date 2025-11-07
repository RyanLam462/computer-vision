#pragma once
#include "realsense_camera.hpp"
#include "orb_detector.hpp"
#include "map_point.hpp"
#include <opencv2/core.hpp>
#include <Eigen/Core>
#include <vector>

// Structure to hold initial map data
struct InitialMapData {
    cv::Mat color;
    cv::Mat depth;
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    std::vector<MapPoint> map_points;
    Eigen::Matrix4d initial_pose;
    
    InitialMapData() {
        initial_pose = Eigen::Matrix4d::Identity();
    }
};

class Initializer {
public:
    bool initialize(RealSenseCamera &camera);
    bool initialize(RealSenseCamera &camera, InitialMapData &initial_data);
    std::vector<MapPoint> backProjectKeypoints(
        const std::vector<cv::KeyPoint> &keypoints,
        const cv::Mat &depth,
        const rs2_intrinsics &intrinsics);
private:
};
