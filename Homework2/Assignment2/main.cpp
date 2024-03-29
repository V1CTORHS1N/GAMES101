// clang-format off
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.hpp"
#include "global.hpp"
#include "Triangle.hpp"

constexpr double MY_PI = 3.1415926;

float DEG2RAD(float deg)
{
    return deg * MY_PI/180;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
    // TODO: Copy-paste your implementation from the previous assignment.
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    /*
    @author: Weitao Xin
    @date: 2023-02-01
    @brief: Perspective Projection Matrix
    @details:
    
    [
        [ n/r,          0,            0,            0      ],
        [  0,          n/t,           0,            0      ],
        [  0,           0,       (n+f)/(n-f),   -2nf/(n-f) ],
        [  0,           0,            1,            0      ]
    ]
    */

    float t = -tan(DEG2RAD(eye_fov / 2)) * abs(zNear);
    float r = t * aspect_ratio;

    projection(0, 0) = zNear / r;
    projection(1, 1) = zNear / t;
    projection(2, 2) = (zNear + zFar) / (zNear - zFar);
    projection(2, 3) = -2 * zNear * zFar / (zNear - zFar);
    projection(3, 2) = 1;

    return projection;
}

int main(int argc, const char** argv) {
    float angle = 0;
    bool command_line = false;
    bool MSAA = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        if (!strcmp(argv[1], "-MSAA") && !strcmp(argv[2], "-s")) {
            MSAA = true;
            filename = argv[3] != NULL ? std::string(argv[3]) : filename;
        } else if (!strcmp(argv[1], "-s")) {
            filename = argv[argc - 1] != NULL ? argv[argc - 1] : filename;
        } else {
            std::cout << "Command Not Found!" << std::endl;
            exit(0);
        }
    } else if (argc >= 2) {
        if (argc == 2) {
            if (!strcmp(argv[1], "-MSAA")) {
                MSAA = true;
                command_line = false;
            } else if (!strcmp(argv[1], "-s")) {
                command_line = true;
            } else {
                std::cout << "Command Not Found!" << std::endl;
                exit(0);
            }
        }
    }


    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0,0,5};

    std::vector<Eigen::Vector3f> pos
            {
                    {2, 0, -2},
                    {0, 2, -2},
                    {-2, 0, -2},
                    {3.5, -1, -5},
                    {2.5, 1.5, -5},
                    {-1, 0.5, -5}
            };

    std::vector<Eigen::Vector3i> ind
            {
                    {0, 1, 2},
                    {3, 4, 5}
            };

    std::vector<Eigen::Vector3f> cols
            {
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0}
            };

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    auto col_id = r.load_colors(cols);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle, MSAA);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle, MSAA);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\r';
        fflush(stdout);
    }

    return 0;
}
// clang-format on