#pragma once

#include "types.h"
#include <vector>

namespace RoofOutline {

/**
 * 几何计算模块
 * 负责多边形处理和直骨架计算
 */
class Geometry {
public:
    /**
     * 验证并修正多边形方向
     * @param polygon 输入多边形
     * @return true if valid, false otherwise
     */
    static bool validateAndFixPolygon(Polygon_2& polygon);

    /**
     * 创建内部直骨架
     * @param polygon 输入多边形
     * @return 直骨架智能指针
     */
    static SsPtr createInteriorSkeleton(const Polygon_2& polygon);

    /**
     * 计算多边形的边界框
     * @param polygon 输入多边形
     * @param min_x, max_x, min_y, max_y 输出边界
     * @param margin 边距
     */
    static void calculateBoundingBox(
        const Polygon_2& polygon,
        double& min_x, double& max_x,
        double& min_y, double& max_y,
        double margin = 2.0
    );

    /**
     * 查找骨架的最内部顶点（最大时间值的顶点）
     * @param skeleton 直骨架
     * @param center_x, center_y 输出中心坐标
     * @param max_time 输出最大时间值
     * @return 是否找到中心顶点
     */
    static bool findCenterVertex(
        const SsPtr& skeleton,
        double& center_x,
        double& center_y,
        double& max_time
    );
};

} 
