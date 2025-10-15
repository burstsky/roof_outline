#include "coordinate_transform.h"

namespace RoofOutline {

CoordinateTransform::CoordinateTransform(
    double min_x, double max_x,
    double min_y, double max_y,
    int svg_width
)
    : min_x_(min_x)
    , min_y_(min_y)
    , svg_width_(svg_width)
{
    double width = max_x - min_x;
    double height = max_y - min_y;
    
    // 计算SVG高度，保持宽高比
    svg_height_ = static_cast<int>(svg_width * height / width);
    
    // 计算缩放比例
    scale_ = svg_width / width;
}

double CoordinateTransform::toSVGX(double x) const {
    return (x - min_x_) * scale_;
}

double CoordinateTransform::toSVGY(double y) const {
    // Y轴翻转
    return svg_height_ - (y - min_y_) * scale_;
}

} 
