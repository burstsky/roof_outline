#pragma once

#include <functional>

namespace RoofOutline {

class CoordinateTransform {
public:
    /**
     * 构造函数
     * @param min_x, max_x 世界坐标X范围
     * @param min_y, max_y 世界坐标Y范围
     * @param svg_width SVG画布宽度（像素）
     */
    CoordinateTransform(
        double min_x, double max_x,
        double min_y, double max_y,
        int svg_width
    );

    /**
     * 世界坐标X转换为SVG坐标X
     */
    double toSVGX(double x) const;

    /**
     * 世界坐标Y转换为SVG坐标Y（注意Y轴翻转）
     */
    double toSVGY(double y) const;

    /**
     * 获取SVG画布宽度
     */
    int getSVGWidth() const { return svg_width_; }

    /**
     * 获取SVG画布高度
     */
    int getSVGHeight() const { return svg_height_; }

    /**
     * 获取缩放比例
     */
    double getScale() const { return scale_; }

private:
    double min_x_;
    double min_y_;
    double scale_;
    int svg_width_;
    int svg_height_;
};

} 
