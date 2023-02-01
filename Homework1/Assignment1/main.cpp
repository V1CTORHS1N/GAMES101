#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

float DEG2RAD(float deg)
{
    return deg * MY_PI/180;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle, bool axis, const char **argv)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    /*
    @author: Weitao Xin
    @date: 2023-01-16
    @brief: Affine Transformation Matrix
    @details:
    
    Rotation around x-axis
    [
        [  1,           0,            0,            0 ],
        [  0,          cos,         -sin,           0 ],
        [  0,          sin,          cos,           0 ],
        [  0,           0,            0,            1 ]
    ]

    Rotation around y-axis
    [
        [ cos,          0,           sin,           0 ],
        [  0,           1,            0,            0 ],
        [-sin,          0,           cos,           0 ],
        [  0,           0,            0,            1 ]
    ]

    Rotation around z-axis
    [
        [ cos,        -sin,           0,            0 ],
        [ sin,         cos,           0,            0 ],
        [  0,           0,            1,            0 ],
        [  0,           0,            0,            1 ]
    ]
    
    */

    rotation_angle = DEG2RAD(rotation_angle);
    if (axis) {
        if (!strcmp(argv[1], "-x") || !strcmp(argv[3], "-x"))
        {
            model(0, 0) = 1;
            model(1, 1) = cos(rotation_angle);
            model(1, 2) = -sin(rotation_angle);
            model(2, 1) = sin(rotation_angle);
            model(2, 2) = cos(rotation_angle);
        }
        else if (!strcmp(argv[1], "-y") || !strcmp(argv[3], "-y"))
        {
            model(0, 0) = cos(rotation_angle);
            model(0, 2) = sin(rotation_angle);
            model(1, 1) = 1;
            model(2, 0) = -sin(rotation_angle);
            model(2, 2) = cos(rotation_angle);
        }
    }
    else
    {
        model(0, 0) = cos(rotation_angle);
        model(0, 1) = -sin(rotation_angle);
        model(1, 0) = sin(rotation_angle);
        model(1, 1) = cos(rotation_angle);
        model(2, 2) = 1;
    }

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

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

    float t = -tan(DEG2RAD(eye_fov) / 2) * abs(zNear);
    float r = t * aspect_ratio;

    projection(0, 0) = zNear / r;
    projection(1, 1) = zNear / t;
    projection(2, 2) = (zNear + zFar) / (zNear - zFar);
    projection(2, 3) = -2 * zNear * zFar / (zNear - zFar);
    projection(3, 2) = 1;

    return projection;
}

int main(int argc, const char **argv)
{
    float angle = 0;
    bool command_line = false;
    bool axis = false;
    std::string filename = "output.png";

    if (argc == 2)
    {
        if (!strcmp(argv[1], "-x") || !strcmp(argv[1], "-y"))
        {
            axis = true;
        }
    }
    else if (argc >= 3)
    {
        if (!strcmp(argv[1], "-r"))
        {
            command_line = true;
            angle = std::stof(argv[2]); // -r by default
            if (!strcmp(argv[3], "-x") || !strcmp(argv[3], "-y"))
            {
                axis = true;
            }
            if (argc == 4)
            {
                filename = std::string(argv[3]);
            } 
            else if (argc == 5)
            {
                filename = std::string(argv[4]);
            }
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle, axis, argv));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle, axis, argv));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\r';
        fflush(stdout);

        if (key == 'a')
        {
            angle += 10;
        }
        else if (key == 'd')
        {
            angle -= 10;
        }
    }

    return 0;
}
