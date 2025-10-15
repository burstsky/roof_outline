#include "types.h"
#include "geometry.h"
#include "coordinate_transform.h"
#include "svg_renderer.h"
#include "roof_unfold.h"
#include <iostream>
#include <vector>

using namespace RoofOutline;

int main()
{
	// 创建多边形 
	Polygon_2 polygon;
	polygon.push_back(Point(0, 0));
	polygon.push_back(Point(0, -10));
	polygon.push_back(Point(15, -10));
	polygon.push_back(Point(15, 5));
	polygon.push_back(Point(-5, 5));
	polygon.push_back(Point(-5, 0));

	//  验证并修正多边形
	if (!Geometry::validateAndFixPolygon(polygon)) {
		return 1;
	}

	// 创建直骨架
	SsPtr skeleton = Geometry::createInteriorSkeleton(polygon);
	if (!skeleton) {
		return 1;
	}

	//  计算边界框和坐标转换
	double min_x, max_x, min_y, max_y;
	Geometry::calculateBoundingBox(polygon, min_x, max_x, min_y, max_y);
	CoordinateTransform ridge_transform(min_x, max_x, min_y, max_y, 800);

	// 定义灰色顶点（SVG坐标）
	std::vector<std::pair<double, double>> gray_vertices = {
		{150, 149.667},
		{316.667, 149.667},
		{483.333, 316.333},
		{733.333, 66.3333},
		{66.6667, 66.3333}
	};

	// 渲染屋脊线俯视图
	if (!SVGRenderer::renderRidgeView("roof_ridges.svg", polygon, skeleton, 
		ridge_transform, gray_vertices)) {
		return 1;
	}

	// 查找中心顶点
	double center_x, center_y, max_time;
	if (!Geometry::findCenterVertex(skeleton, center_x, center_y, max_time)) {
		// 如果没找到，使用几何中心
		center_x = 5.0;
		center_y = -2.5;
		std::cout << "使用默认几何中心: (" << center_x << ", " << center_y << ")" << std::endl;
	}

	//  计算屋顶展开
	double roof_angle = 30.0; 
	RoofUnfold unfolder(skeleton, center_x, center_y, roof_angle, 0.15);
	auto unfolded_faces = unfolder.computeUnfoldedFaces(ridge_transform, gray_vertices);

	double unfold_min_x, unfold_max_x, unfold_min_y, unfold_max_y;
	RoofUnfold::calculateUnfoldedBoundingBox(unfolded_faces, 
		unfold_min_x, unfold_max_x, unfold_min_y, unfold_max_y);
	CoordinateTransform unfold_transform(unfold_min_x, unfold_max_x, 
		unfold_min_y, unfold_max_y, 1000);

	// 渲染展开图
	if (!SVGRenderer::renderUnfoldedView("roof_unfolded.svg", unfolded_faces, 
		unfold_transform, roof_angle)) {
		return 1;
	}

	std::cout << "\n✓ 所有文件生成完成！" << std::endl;
	return 0;
}
