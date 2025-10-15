#pragma once

#include "types.h"
#include "coordinate_transform.h"
#include <string>
#include <vector>
#include <utility>

namespace RoofOutline {

/**
 * SVG渲染模块
 * 负责生成SVG文件
 */
class SVGRenderer {
public:
    /**
     * 渲染屋脊线俯视图
     * @param filename 输出文件名
     * @param polygon 多边形
     * @param skeleton 直骨架
     * @param transform 坐标转换器
     * @param gray_vertices 需要标记为灰色的特殊顶点（SVG坐标）
     * @return 是否成功
     */
    static bool renderRidgeView(
        const std::string& filename,
        const Polygon_2& polygon,
        const SsPtr& skeleton,
        const CoordinateTransform& transform,
        const std::vector<std::pair<double, double>>& gray_vertices
    );

    /**
     * 渲染屋顶展开图
     * @param filename 输出文件名
     * @param unfolded_faces 展开后的面信息
     * @param transform 坐标转换器
     * @param roof_angle 屋顶倾斜角度
     * @return 是否成功
     */
    static bool renderUnfoldedView(
        const std::string& filename,
        const std::vector<std::pair<std::vector<std::pair<double, double>>, bool>>& unfolded_faces,
        const CoordinateTransform& transform,
        double roof_angle
    );

private:
    /**
     * 检查顶点是否接近目标点
     */
    static bool isNearPoint(double x, double y, double tx, double ty, double epsilon = 0.5);

    /**
     * 检查面是否包含灰色顶点
     */
    static bool containsGrayVertex(
        const std::vector<std::pair<double, double>>& face_vertices,
        const std::vector<std::pair<double, double>>& gray_vertices
    );
};

} 
