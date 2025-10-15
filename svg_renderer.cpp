#include "svg_renderer.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <filesystem>

namespace RoofOutline {

bool SVGRenderer::isNearPoint(double x, double y, double tx, double ty, double epsilon) {
    return std::abs(x - tx) < epsilon && std::abs(y - ty) < epsilon;
}

bool SVGRenderer::containsGrayVertex(
    const std::vector<std::pair<double, double>>& face_vertices,
    const std::vector<std::pair<double, double>>& gray_vertices
) {
    int match_count = 0;
    for (const auto& fv : face_vertices) {
        for (const auto& gv : gray_vertices) {
            if (isNearPoint(fv.first, fv.second, gv.first, gv.second)) {
                match_count++;
                break;
            }
        }
    }
    return match_count >= 3; // 至少包含3个指定顶点才填充灰色
}

bool SVGRenderer::renderRidgeView(
    const std::string& filename,
    const Polygon_2& polygon,
    const SsPtr& skeleton,
    const CoordinateTransform& transform,
    const std::vector<std::pair<double, double>>& gray_vertices
) {
    std::ofstream svg_file(filename);
    if (!svg_file) {
        std::cerr << "无法创建 SVG 文件: " << filename << std::endl;
        return false;
    }

    int svg_width = transform.getSVGWidth();
    int svg_height = transform.getSVGHeight();

    // SVG 头部
    svg_file << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    svg_file << "<svg width=\"" << svg_width << "\" height=\"" << svg_height << "\" "
        << "xmlns=\"http://www.w3.org/2000/svg\">\n";
    svg_file << "<title>屋顶屋脊线俯视图</title>\n";
    svg_file << "<desc>CGAL Straight Skeleton - 30度倾斜角屋顶</desc>\n\n";

    // 背景
    svg_file << "<!-- 背景 -->\n";
    svg_file << "<rect width=\"100%\" height=\"100%\" fill=\"#f8f8f8\"/>\n\n";

    // 绘制骨架分割出的各个面
    svg_file << "<!-- 骨架分割的面 -->\n";
    svg_file << "<g id=\"faces\" opacity=\"0.8\">\n";

    for (auto fit = skeleton->faces_begin(); fit != skeleton->faces_end(); ++fit) {
        // 收集face的所有顶点
        std::vector<std::pair<double, double>> face_svg_vertices;
        auto he = fit->halfedge();
        auto start = he;
        do {
            double x = he->vertex()->point().x();
            double y = he->vertex()->point().y();
            face_svg_vertices.push_back({transform.toSVGX(x), transform.toSVGY(y)});
            he = he->next();
        } while (he != start);

        // 判断是否应该填充灰色
        bool is_gray = containsGrayVertex(face_svg_vertices, gray_vertices);
        std::string fill_color = is_gray ? "#9e9e9e" : "#e3f2fd";

        // 绘制多边形
        svg_file << "  <polygon points=\"";
        for (const auto& v : face_svg_vertices) {
            svg_file << v.first << "," << v.second << " ";
        }
        svg_file << "\" fill=\"" << fill_color << "\" stroke=\"none\" />\n";
    }
    svg_file << "</g>\n\n";

    // 绘制外轮廓多边形
    svg_file << "<!-- 屋顶外轮廓 -->\n";
    svg_file << "<polygon points=\"";
    for (auto it = polygon.vertices_begin(); it != polygon.vertices_end(); ++it) {
        svg_file << transform.toSVGX(it->x()) << "," << transform.toSVGY(it->y()) << " ";
    }
    svg_file << "\" fill=\"none\" stroke=\"#1976d2\" stroke-width=\"2\" />\n\n";

    // 绘制屋脊线（骨架边）
    svg_file << "<!-- 屋脊线（内部骨架边）-->\n";
    svg_file << "<g id=\"ridge-lines\" stroke=\"#d32f2f\" stroke-width=\"2.5\" stroke-linecap=\"round\">\n";

    for (auto hit = skeleton->halfedges_begin(); hit != skeleton->halfedges_end(); ++hit) {
        if (hit < hit->opposite()) {
            auto v1 = hit->vertex();
            auto v2 = hit->opposite()->vertex();

            double x1 = v1->point().x();
            double y1 = v1->point().y();
            double x2 = v2->point().x();
            double y2 = v2->point().y();

            svg_file << "  <line x1=\"" << transform.toSVGX(x1) << "\" y1=\"" << transform.toSVGY(y1)
                << "\" x2=\"" << transform.toSVGX(x2) << "\" y2=\"" << transform.toSVGY(y2) << "\" />\n";
        }
    }
    svg_file << "</g>\n\n";

    // 绘制骨架顶点
    svg_file << "<!-- 骨架顶点 -->\n";
    svg_file << "<g id=\"vertices\">\n";

    for (auto vit = skeleton->vertices_begin(); vit != skeleton->vertices_end(); ++vit) {
        double x = vit->point().x();
        double y = vit->point().y();

        if (vit->is_skeleton()) {
            // 内部骨架顶点
            svg_file << "  <circle cx=\"" << transform.toSVGX(x) << "\" cy=\"" << transform.toSVGY(y)
                << "\" r=\"4\" fill=\"#d32f2f\" stroke=\"white\" stroke-width=\"1\" />\n";
        } else {
            // 轮廓顶点
            svg_file << "  <circle cx=\"" << transform.toSVGX(x) << "\" cy=\"" << transform.toSVGY(y)
                << "\" r=\"3\" fill=\"#1976d2\" stroke=\"white\" stroke-width=\"1\" />\n";
        }
    }
    svg_file << "</g>\n\n";

    svg_file << "</svg>\n";
    svg_file.close();

    std::cout << "✓ SVG 文件已生成: " << filename << std::endl;
    std::cout << "  文件位置: " << std::filesystem::current_path().string() << "\\" << filename << std::endl;

    return true;
}

bool SVGRenderer::renderUnfoldedView(
    const std::string& filename,
    const std::vector<std::pair<std::vector<std::pair<double, double>>, bool>>& unfolded_faces,
    const CoordinateTransform& transform,
    double roof_angle
) {
    std::ofstream unfold_svg(filename);
    if (!unfold_svg) {
        std::cerr << "无法创建展开图 SVG 文件: " << filename << std::endl;
        return false;
    }

    int svg_width = transform.getSVGWidth();
    int svg_height = transform.getSVGHeight();

    // SVG 头部
    unfold_svg << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    unfold_svg << "<svg width=\"" << svg_width << "\" height=\"" << svg_height << "\" "
        << "xmlns=\"http://www.w3.org/2000/svg\">\n";
    unfold_svg << "<title>屋顶展开图</title>\n";
    unfold_svg << "<desc>屋顶各面展开图 - " << roof_angle << "度倾斜角</desc>\n\n";

    // 背景
    unfold_svg << "<!-- 背景 -->\n";
    unfold_svg << "<rect width=\"100%\" height=\"100%\" fill=\"#f8f8f8\"/>\n\n";

    // 绘制展开的屋面
    unfold_svg << "<!-- 展开的屋面 -->\n";
    unfold_svg << "<g id=\"unfolded-faces\" opacity=\"0.85\">\n";

    for (const auto& [vertices, is_gray] : unfolded_faces) {
        std::string fill_color = is_gray ? "#9e9e9e" : "#e3f2fd";

        // 绘制多边形
        unfold_svg << "  <polygon points=\"";
        for (const auto& v : vertices) {
            unfold_svg << transform.toSVGX(v.first) << "," << transform.toSVGY(v.second) << " ";
        }
        unfold_svg << "\" fill=\"" << fill_color << "\" stroke=\"#666\" stroke-width=\"1.5\" />\n";
    }
    unfold_svg << "</g>\n\n";

    // 绘制展开后的边缘线
    unfold_svg << "<!-- 面的边缘线 -->\n";
    unfold_svg << "<g id=\"edge-lines\" stroke=\"#1976d2\" stroke-width=\"2\" opacity=\"0.7\">\n";

    for (const auto& [vertices, is_gray] : unfolded_faces) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            const auto& v1 = vertices[i];
            const auto& v2 = vertices[(i + 1) % vertices.size()];

            unfold_svg << "  <line x1=\"" << transform.toSVGX(v1.first) << "\" y1=\"" << transform.toSVGY(v1.second)
                << "\" x2=\"" << transform.toSVGX(v2.first) << "\" y2=\"" << transform.toSVGY(v2.second) << "\" />\n";
        }
    }
    unfold_svg << "</g>\n\n";

    // SVG 结束
    unfold_svg << "</svg>\n";
    unfold_svg.close();

    std::cout << "✓ 展开图 SVG 文件已生成: " << filename << std::endl;
    std::cout << "  文件位置: " << std::filesystem::current_path().string() << "\\" << filename << std::endl;

    return true;
}

} 
