#include "lib/convex.h"
#include <math.h>
#include <set>
#include <map>

struct Face {
	int v[3];
	vec3 normal;
	bool active;
	
	Face(int a, int b, int c, const std::vector<vec3>& pts) {
		v[0] = a; v[1] = b; v[2] = c;
		active = true;
		vec3 edge1 = pts[b] - pts[a];
		vec3 edge2 = pts[c] - pts[a];
		normal = cross(edge1, edge2);
	}
	
	// 检查点 p 是否在面外（可见）
	bool can_see(const vec3& p, const std::vector<vec3>& pts) const {
		return (p - pts[v[0]]) * normal > 1e-6f;
	}
};

std::vector<std::pair<vec3, vec3>> convex3d(std::vector<vec3> points) {
	int n = points.size();
	if(n == 3) {
		return {
			{ points[0], points[1] }, 
			{ points[1], points[2] }, 
			{ points[0], points[2] }
		};
	}
	
	if (n < 4) return {};
	
	// 1. 寻找初始四面体 (处理退化情况)
	std::vector<Face> faces;
	bool found = false;
	for (int i = 2; i < n; ++i) {
		vec3 nm = cross(points[1] - points[0], points[i] - points[0]);
		if (nm.leng() > 1e-7f) {
			for (int j = i + 1; j < n; ++j) {
				if (abs((points[j] - points[0]) * nm) > 1e-7f) {
					// 建立初始四面体
					int a = 0, b = 1, c = i, d = j;
					if ((points[d] - points[a]) * cross(points[b] - points[a], points[c] - points[a]) > 0)
						std::swap(b, c);
					
					faces.emplace_back(a, b, c, points);
					faces.emplace_back(a, c, d, points);
					faces.emplace_back(a, d, b, points);
					faces.emplace_back(b, d, c, points);
					
					std::swap(points[0], points[a]);
					std::swap(points[1], points[b]);
					std::swap(points[i], points[c]);
					std::swap(points[j], points[d]);
					found = true; break;
				}
			}
		}
		if (found) break;
	}
	
	if (!found) return {}; // 所有点共面或共线
	
	// 2. 增量构建
	for (int i = 4; i < n; ++i) {
		std::vector<std::pair<int, int>> horizon;
		for (auto& f : faces) {
			if (f.active && f.can_see(points[i], points)) {
				f.active = false;
				for (int j = 0; j < 3; ++j) {
					int u = f.v[j], v = f.v[(j + 1) % 3];
					bool shared = false;
					for (auto& other : faces) {
						if (&f != &other && other.active && other.can_see(points[i], points)) continue;
						if (&f != &other && other.active) {
							for (int k = 0; k < 3; ++k) {
								if (other.v[k] == v && other.v[(k + 1) % 3] == u) {
									horizon.push_back({u, v});
									shared = true; break;
								}
							}
						}
						if (shared) break;
					}
				}
			}
		}
		for (auto& edge : horizon) {
			faces.emplace_back(edge.first, edge.second, i, points);
		}
	}
	
	// 3. 收集所有边
	std::set<std::pair<int, int>> edge_set;
	for (const auto& f : faces) {
		if (f.active) {
			for (int i = 0; i < 3; ++i) {
				int u = f.v[i], v = f.v[(i + 1) % 3];
				if (u > v) std::swap(u, v);
				edge_set.insert({u, v});
			}
		}
	}
	
	std::vector<std::pair<vec3, vec3>> result;
	for (auto& e : edge_set) {
		result.push_back({points[e.first], points[e.second]});
	}
	
	return result;
}
