# ORB-SLAM2-RGBD-D434i

This project builds a cleaned, RGB-D-only version of ORB-SLAM2 specifically configured for Intel RealSense D435i. It includes a C++ implementation with camera calibration settings, feature extraction, tracking, local mapping, and loop closure.
Intended as a semester-long project baseline for research and improvement.

## Pipeline
- Initialization(Teymur)
```
Added include/ headers: initializer.hpp, map_point.hpp, orb_detector.hpp, realsense_camera.hpp
Added src/ implementations: realsense_camera.cpp, orb_detector.cpp, initializer.cpp, viewer.cpp, initialize_map.cpp
Added tests/ with unit, integration and hardware tests plus a small test framework and mocks
Build & CI

Updated top-level CMakeLists.txt: new libraries (realsense_camera, orb_detector, initializer, viewer), new executables (test_initial_setup, initialize_map), enable testing and add tests/ subdir
Added helper scripts: build.sh, run_tests.sh, run_hardware_tests.sh, setup_realsense.sh

Tests & mocks
Test framework (tests/test_framework.hpp) and many test sources under tests/unit and tests/integration
MockRealSenseCamera in mock_camera.hpp to allow CPU-only testing of pipeline
Hardware integration tests that exercise RealSense hardware if present

Debug / UX
Initializer saves debug images and performs ORB feature detection + back-projection into 3D map points
ORB detector implements fallback/relaxed parameter attempts if no keypoints found
RealSense camera wrapper warms up the pipeline and exposes intrinsics
```
- Tracking(Ryan)
- Local Mapping
- Loop Closing
## Building

```
sudo bash setup_realsense.sh
```

## Library

## Repo Tree
```
ORB-SLAM2-RGBD-D435i/
├── cmake_modules/
│   └── ...         # Custom CMake module to locate Eigen3 headers on the system, if CMake cannot find it automatically
├── RGB-D/
│   └── ...         # Entry points
├── include/
│   └── ...         # ORB-SLAM2 core classes
├── src/
│   └── ...         # Implementations
├── Thirdparty/
│   ├── DBoW2/
│   └── g2o/
├── Vocabulary/
│   └── ...          # ORB feature vocabulary used for loop closure detection in ORB-SLAM2
├── CMakeLists.txt
├── Dependencies.md
├── LICENSE.txt
├── .gitignore
├── build.sh
├── License-gpl.txt
└── README.md
```

# Contributors
Hwayeon, Arjav, Damien, Ryan, Teymur