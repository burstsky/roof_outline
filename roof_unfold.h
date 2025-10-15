#pragma once

#include "types.h"
#include "coordinate_transform.h"
#include <vector>
#include <utility>

namespace RoofOutline {

/**
 * 屋顶展开模块
 * 负责计算屋顶各面的展开
 */
class RoofUnfold {
public:
    /**
     * 构造函数
     * @param skeleton 直骨架
     * @param center_x, center_y 展开中心点
     * @param roof_angle 屋顶倾斜角度（度）
     * @param explosion_factor 爆炸视图系数（面之间的分离程度）
     */
    RoofUnfold(
        const SsPtr& skeleton,
        double center_x,
        double center_y,
        double roof_angle = 30.0,
        double explosion_factor = 0.15
    );

    /**
     * 计算展开后的所有面
     * @param ridge_transform 用于判断灰色顶点的坐标转换器（基于原始俯视图）
     * @param gray_vertices 需要标记为灰色的特殊顶点（SVG坐标）
     * @return 展开后的面信息 <顶点列表, 是否为灰色>
     */
    std::vector<std::pair<std::vector<std::pair<double, double>>, bool>> 
    computeUnfoldedFaces(
        const CoordinateTransform& ridge_transform,
        const std::vector<std::pair<double, double>>& gray_vertices
    );

    /**
     * 计算展开后的边界框
     * @param unfolded_faces 展开后的面
     * @param min_x, max_x, min_y, max_y 输出边界
     * @param margin 边距
     */
    static void calculateUnfoldedBoundingBox(
        const std::vector<std::pair<std::vector<std::pair<double, double>>, bool>>& unfolded_faces,
        double& min_x, double& max_x,
        double& min_y, double& max_y,
        double margin = 4.0
    );

private:
    SsPtr skeleton_;
    double center_x_;
    double center_y_;
    double roof_angle_rad_;
    double unfold_factor_;
    double explosion_factor_;

    /**
     * 检查顶点是否接近中心点
     */
    bool isNearCenterPoint(double x, double y) const;

    /**
     * 展开单个顶点
     */
    std::pair<double, double> unfoldVertex(double x, double y) const;

    /**
     * 检查面顶点是否包含灰色顶点
     */
    static bool containsGrayVertex(
        const std::vector<std::pair<double, double>>& face_svg_vertices,
        const std::vector<std::pair<double, double>>& gray_vertices
    );
};

} 
