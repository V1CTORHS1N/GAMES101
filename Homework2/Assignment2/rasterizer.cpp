// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(int x, int y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Eigen::Vector3f point = Eigen::Vector3f(x + 0.5, y + 0.5, 1.0f);

    Eigen::Vector3f _v0_v1 = _v[1] - _v[0];
    Eigen::Vector3f _v1_v2 = _v[2] - _v[1];
    Eigen::Vector3f _v2_v0 = _v[0] - _v[2];

    Eigen::Vector3f _0_p = point - _v[0];
    Eigen::Vector3f _1_p = point - _v[1];
    Eigen::Vector3f _2_p = point - _v[2];

    auto alpha = _v0_v1.cross(_0_p);
    auto beta = _v1_v2.cross(_1_p);
    auto gamma = _v2_v0.cross(_0_p);

    if (alpha.z() > 0 && beta.z() > 0 && gamma.z() > 0 || alpha.z() < 0 && beta.z() < 0 && gamma.z() < 0)
        return true;
    return false;
}

/*
@author: Weitao Xin
@date: 2023-02-02
@brief: MSAA support.
@details: static int insideTriangle(Eigen::Vector3f point, const Vector3f* _v)
*/
static int insideTriangle(Eigen::Vector3f point, const Vector3f* _v)
{
    Eigen::Vector3f _v0_v1 = _v[1] - _v[0];
    Eigen::Vector3f _v1_v2 = _v[2] - _v[1];
    Eigen::Vector3f _v2_v0 = _v[0] - _v[2];

    Eigen::Vector3f _0_p = point - _v[0];
    Eigen::Vector3f _1_p = point - _v[1];
    Eigen::Vector3f _2_p = point - _v[2];

    auto alpha = _v0_v1.cross(_0_p);
    auto beta = _v1_v2.cross(_1_p);
    auto gamma = _v2_v0.cross(_0_p);

    if (alpha.z() > 0 && beta.z() > 0 && gamma.z() > 0 || alpha.z() < 0 && beta.z() < 0 && gamma.z() < 0)
        return true;
    return false;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type, bool MSAA = false)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t, MSAA);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t, bool MSAA) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    
    // If so, use the following code to get the interpolated z value.
    // auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    // float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    // float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    // z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.

    /*
    @author: Weitao Xin
    @date: 2023-02-01
    @brief: - Find out the bounding box of current triangle.
            - Render the pixels inside the triangle with given color.
    */

    /*
    @author: Weitao Xin
    @date: 2023-02-02
    @brief: MSAA suppport.
    */

    int box_left = std::floor(std::min(v[0].x(), std::min(v[1].x(), v[2].x())));
    int box_right = std::ceil(std::max(v[0].x(), std::max(v[1].x(), v[2].x())));
    int box_bottom = std::floor(std::min(v[0].y(), std::min (v[1].y(), v[2].y())));
    int box_top = std::ceil(std::max(v[0].y(), std::max(v[1].y(), v[2].y())));

    for (int x = box_left; x <= box_right; x++)
    {
        for (int y = box_bottom; y <= box_top; y++)
        {
            if (MSAA) {
                // sampling points
                Eigen::Vector3f sample[] = {
                                                Eigen::Vector3f(x + 0.25, y + 0.25, 1.0f),
                                                Eigen::Vector3f(x + 0.75, y + 0.25, 1.0f),
                                                Eigen::Vector3f(x + 0.25, y + 0.75, 1.0f),
                                                Eigen::Vector3f(x + 0.75, y + 0.75, 1.0f)
                };

                int offset = 0;
                float z_min = FLT_MAX;
                for (Eigen::Vector3f point : sample)
                {
                    if (insideTriangle(point, t.v))
                    {
                        auto[alpha, beta, gamma] = computeBarycentric2D(point.x(), point.y(), t.v);
                        float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                        float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                        z_interpolated *= w_reciprocal;
                        if (z_interpolated < sample_depth_buf[get_index(x, y) * 4 + offset])
                        {
                            sample_frame_buf[get_index(x, y) * 4 + offset] = t.getColor() * 0.25;
                            sample_depth_buf[get_index(x, y) * 4 + offset] = z_interpolated;
                            if (z_interpolated < z_min) z_min = z_interpolated;
                        }
                    }
                    offset++;
                }

                Eigen::Vector3f color = Eigen::Vector3f(0, 0, 0);
                for (int i = 0; i < 4; i++)
                {
                    color += sample_frame_buf[get_index(x, y) * 4 + i];
                }
                set_pixel(Eigen::Vector3f(x, y, z_min), color);

            } else {
                if (insideTriangle(x, y, t.v)) {
                    auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    if (z_interpolated < depth_buf[get_index(x, y)])
                    {
                        set_pixel(Eigen::Vector3f(x, y, z_interpolated), t.getColor());
                        depth_buf[get_index(x, y)] = z_interpolated;
                    }
                }
            }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);

    sample_frame_buf.resize(w * h * 4);
    sample_depth_buf.resize(w * h * 4);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;
}

// clang-format on