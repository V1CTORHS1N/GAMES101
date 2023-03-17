//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v) {

        auto u_img = u * width;
        auto v_img = (1 - v) * width;

        float u_min = std::floor(u_img);
        float u_max = std::ceil(u_img);
        float v_min = std::floor(v_img);
        float v_max = std::ceil(v_img);

        auto _01 = image_data.at<cv::Vec3b>(v_max, u_max);
        auto _11 = image_data.at<cv::Vec3b>(v_min, u_max);
        auto _00 = image_data.at<cv::Vec3b>(v_max, u_min);
        auto _10 = image_data.at<cv::Vec3b>(v_min, u_min);


        float s = u_img - u_min;
        float t = v_img - v_min;
        auto u_0 = _00 + s * (_10 - _00);
        auto u_1 = _01 + s * (_11 - _01);
        auto color = u_0 + t * (u_1 - u_0);

        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
