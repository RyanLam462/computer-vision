#include <pangolin/pangolin.h>
#include <opencv2/opencv.hpp>

int main(int argc, char** argv)
{
    // Pangolin window
    pangolin::CreateWindowAndBind("Pangolin Test", 640, 480);
    glEnable(GL_DEPTH_TEST);

    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(640,480,420,420,320,240,0.1,1000),
        pangolin::ModelViewLookAt(0,0,-3, 0,0,0, pangolin::AxisY)
    );

    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
        .SetBounds(0.0, 1.0, 0.0, 1.0, -640.0f/480.0f)
        .SetHandler(&handler);

    while(!pangolin::ShouldQuit())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        // Draw a simple cube
        glColor3f(1.0, 0.0, 0.0);
        pangolin::glDrawColouredCube();

        pangolin::FinishFrame();
    }

    // OpenCV test
    cv::Mat img = cv::Mat::zeros(480, 640, CV_8UC3);
    cv::putText(img, "OpenCV works!", cv::Point(50, 50),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0,255,0), 2);
    cv::imshow("OpenCV Test", img);
    cv::waitKey(0);

    return 0;
}
