# ORB-SLAM2-RGBD-D434i

This project builds a cleaned, RGB-D-only version of ORB-SLAM2 specifically configured for Intel RealSense D435i. It includes a C++ implementation with camera calibration settings, feature extraction, tracking, local mapping, and loop closure.
Intended as a semester-long project baseline for research and improvement.

## Progress
🟦⬜⬜⬜⬜ 20%

## Pipeline
- Initialization (Teymur)

    This step stablishes the very first map and camera pose from a single RGB-D frame captured by the RealSense camera. It begins by acquiring synchronized color and depth images, which are then converted and saved for debugging. The color frame is processed using an ORB feature detector to extract distinctive keypoints and descriptors. Each 2D keypoint is then back-projected into 3D space using its corresponding depth value and the camera’s intrinsic parameters, producing a set of `MapPoint` structures that represent points in the environment. An initial camera pose—defined as the identity transformation—is assigned to this first frame, effectively anchoring the coordinate system for subsequent mapping and tracking. This step produces a consistent, depth-based 3D map seed from which the visual SLAM system can grow.

- Local Tracking(Ryan)
- Mapping (Damien)
- Loop Closing

## Building
./build.sh ./bin/initialize_mapp

## Library

## Repo Tree
computer-vision/
├── Thirdparty/
│   ├── DBoW2/                 # Visual vocabulary library
│   └── g2o/                   # Graph optimization library
│
├── include/
│   ├── initializer.hpp        # Defines Initializer class (main pipeline)
│   ├── map_point.hpp          # Defines MapPoint structure (3D keypoint)
│   ├── orb_detector.hpp       # ORBDetector wrapper for OpenCV ORB
│   └── realsense_camera.hpp   # RealSenseCamera abstraction layer
│
├── src/
│   ├── initializer.cpp        # Core initialization pipeline logic
│   ├── initialize_mapp.cpp    # Entry point for initialization process
│   ├── orb_detector.cpp       # ORB feature detection implementation
│   ├── realsense_camera.cpp   # Frame capture and camera handling
│   ├── test_initial_setup.cpp # Standalone RGB-D test visualization (OpenCV/Pangolin)
│   └── viewer.cpp             # Minimal viewer stub (headless safe)
│
├── tests/
│   ├── integration/           # (Placeholder) for integration tests
│   ├── mocks/                 # (Placeholder) for mock hardware/data
│   ├── unit/                  # (Placeholder) for unit tests
│   ├── CMakeLists.txt
│   ├── test_framework.hpp
│   └── test_runner.cpp
│
├── CMakeLists.txt             # Build configuration
├── Dependencies.md            # Lists external dependencies (OpenCV, librealsense, Pangolin)
├── LICENSE.txt
├── License-gpl.txt
├── .gitignore
├── .gitmodules
├── run_tests.sh               # Test runner script
├── run_hardware_tests.sh      # RealSense hardware validation script
├── build.sh                   # Build automation script
└── README.md

# Contributors
Hwayeon, Teymur, Ryan, Arjav, Damien