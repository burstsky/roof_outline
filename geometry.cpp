#include "geometry.h"
#include <iostream>
#include <algorithm>

namespace RoofOutline {

bool Geometry::validateAndFixPolygon(Polygon_2& polygon) {
    // 检查多边形方向
    if (polygon.is_clockwise_oriented()) {
        polygon.reverse_orientation();
        std::cout << "多边形已转换为逆时针方向" << std::endl;
    }

    // 检查多边形是否自交
    if (!polygon.is_simple()) {
        std::cerr << "错误：多边形存在自交！" << std::endl;
        return false;
    }

    return true;
}

SsPtr Geometry::createInteriorSkeleton(const Polygon_2& polygon) {
    std::cout << "正在计算内部直骨架（屋脊线）..." << std::endl;
    SsPtr skeleton = CGAL::create_interior_straight_skeleton_2(polygon);
    
    if (!skeleton) {
        std::cerr << "错误：无法创建直骨架！" << std::endl;
    }
    
    return skeleton;
}

void Geometry::calculateBoundingBox(
    const Polygon_2& polygon,
    double& min_x, double& max_x,
    double& min_y, double& max_y,
    double margin
) {
    bool first = true;
    
    for (auto it = polygon.vertices_begin(); it != polygon.vertices_end(); ++it) {
        if (first) {
            min_x = max_x = it->x();
            min_y = max_y = it->y();
            first = false;
        } else {
            min_x = std::min(min_x, it->x());
            max_x = std::max(max_x, it->x());
            min_y = std::min(min_y, it->y());
            max_y = std::max(max_y, it->y());
        }
    }
    
    // 添加边距
    min_x -= margin;
    max_x += margin;
    min_y -= margin;
    max_y += margin;
}

bool Geometry::findCenterVertex(
    const SsPtr& skeleton,
    double& center_x,
    double& center_y,
    double& max_time
) {
    max_time = -1;
    bool found = false;
    
    for (auto vit = skeleton->vertices_begin(); vit != skeleton->vertices_end(); ++vit) {
        if (vit->is_skeleton()) {
            double time_val = vit->time();
            if (time_val > max_time) {
                max_time = time_val;
                center_x = vit->point().x();
                center_y = vit->point().y();
                found = true;
            }
        }
    }
    
    if (found) {
        std::cout << "中心顶点坐标: (" << center_x << ", " << center_y 
                  << "), 时间值: " << max_time << std::endl;
    }
    
    return found;
}

}
