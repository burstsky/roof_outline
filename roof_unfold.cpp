#include "roof_unfold.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace RoofOutline {

RoofUnfold::RoofUnfold(
    const SsPtr& skeleton,
    double center_x,
    double center_y,
    double roof_angle,
    double explosion_factor
)
    : skeleton_(skeleton)
    , center_x_(center_x)
    , center_y_(center_y)
    , explosion_factor_(explosion_factor)
{
    // 转换角度为弧度
    roof_angle_rad_ = roof_angle * M_PI / 180.0;
    
    // 展开系数：从中心到边缘的距离需要根据倾斜角展开
    unfold_factor_ = 1.0 / std::cos(roof_angle_rad_);
}

bool RoofUnfold::isNearCenterPoint(double x, double y) const {
    double epsilon = 0.01;
    return std::abs(x - center_x_) < epsilon && std::abs(y - center_y_) < epsilon;
}

std::pair<double, double> RoofUnfold::unfoldVertex(double x, double y) const {
    // 如果是中心顶点，保持原位
    if (isNearCenterPoint(x, y)) {
        return {x, y};
    }

    // 非中心顶点：从中心点径向展开
    double dx = x - center_x_;
    double dy = y - center_y_;
    double dist = std::sqrt(dx * dx + dy * dy);

    // 展开：径向距离按展开系数调整
    double unfolded_dist = dist * unfold_factor_;

    // 保持角度，调整距离
    if (dist > 0.001) {
        double unfolded_x = center_x_ + (dx / dist) * unfolded_dist;
        double unfolded_y = center_y_ + (dy / dist) * unfolded_dist;
        return {unfolded_x, unfolded_y};
    }

    return {x, y};
}

bool RoofUnfold::containsGrayVertex(
    const std::vector<std::pair<double, double>>& face_svg_vertices,
    const std::vector<std::pair<double, double>>& gray_vertices
) {
    int match_count = 0;
    double epsilon = 0.5;
    
    for (const auto& fv : face_svg_vertices) {
        for (const auto& gv : gray_vertices) {
            if (std::abs(fv.first - gv.first) < epsilon && 
                std::abs(fv.second - gv.second) < epsilon) {
                match_count++;
                break;
            }
        }
    }
    return match_count >= 3; // 至少包含3个指定顶点才填充灰色
}

std::vector<std::pair<std::vector<std::pair<double, double>>, bool>> 
RoofUnfold::computeUnfoldedFaces(
    const CoordinateTransform& ridge_transform,
    const std::vector<std::pair<double, double>>& gray_vertices
) {
    std::vector<std::pair<std::vector<std::pair<double, double>>, bool>> unfolded_faces;

    for (auto fit = skeleton_->faces_begin(); fit != skeleton_->faces_end(); ++fit) {
        std::vector<std::pair<double, double>> original_verts;
        std::vector<std::pair<double, double>> unfolded_verts;

        // 收集原始顶点
        auto he = fit->halfedge();
        auto start = he;
        do {
            double x = he->vertex()->point().x();
            double y = he->vertex()->point().y();
            original_verts.push_back({x, y});
            he = he->next();
        } while (he != start);

        // 展开每个顶点
        for (const auto& v : original_verts) {
            auto unfolded_v = unfoldVertex(v.first, v.second);
            unfolded_verts.push_back(unfolded_v);
        }

        // 应用爆炸效果
        // 计算展开后face的中心点
        double face_cx = 0, face_cy = 0;
        for (const auto& v : unfolded_verts) {
            face_cx += v.first;
            face_cy += v.second;
        }
        face_cx /= unfolded_verts.size();
        face_cy /= unfolded_verts.size();

        // 计算从全局中心到face中心的方向
        double dx_explode = face_cx - center_x_;
        double dy_explode = face_cy - center_y_;
        double dist_explode = std::sqrt(dx_explode * dx_explode + dy_explode * dy_explode);

        double offset_x = 0, offset_y = 0;
        if (dist_explode > 0.01) {
            offset_x = (dx_explode / dist_explode) * explosion_factor_ * dist_explode;
            offset_y = (dy_explode / dist_explode) * explosion_factor_ * dist_explode;
        }

        // 应用爆炸偏移到所有顶点（中心顶点除外）
        for (size_t i = 0; i < unfolded_verts.size(); ++i) {
            auto& v = unfolded_verts[i];
            const auto& orig_v = original_verts[i];

            // 如果不是中心顶点，则应用爆炸偏移
            if (!isNearCenterPoint(orig_v.first, orig_v.second)) {
                v.first += offset_x;
                v.second += offset_y;
            }
        }

        // 判断是否为灰色face（使用原始顶点转换为SVG坐标后判断）
        std::vector<std::pair<double, double>> svg_verts_for_check;
        for (const auto& v : original_verts) {
            svg_verts_for_check.push_back({
                ridge_transform.toSVGX(v.first), 
                ridge_transform.toSVGY(v.second)
            });
        }
        bool is_gray = containsGrayVertex(svg_verts_for_check, gray_vertices);

        unfolded_faces.push_back({unfolded_verts, is_gray});
    }

    return unfolded_faces;
}

void RoofUnfold::calculateUnfoldedBoundingBox(
    const std::vector<std::pair<std::vector<std::pair<double, double>>, bool>>& unfolded_faces,
    double& min_x, double& max_x,
    double& min_y, double& max_y,
    double margin
) {
    bool first = true;

    for (const auto& [vertices, is_gray] : unfolded_faces) {
        for (const auto& v : vertices) {
            if (first) {
                min_x = max_x = v.first;
                min_y = max_y = v.second;
                first = false;
            } else {
                min_x = std::min(min_x, v.first);
                max_x = std::max(max_x, v.first);
                min_y = std::min(min_y, v.second);
                max_y = std::max(max_y, v.second);
            }
        }
    }

    // 添加边距
    min_x -= margin;
    max_x += margin;
    min_y -= margin;
    max_y += margin;
}

} 
