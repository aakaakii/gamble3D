// convex.h
#ifndef CONVEX_H
#define CONVEX_H

#include <vector>
#include <utility>
#include "lib/types.h"
using gm::vec3;

// 主函数：计算凸包上的所有剖分边
std::vector<std::pair<vec3, vec3>> convex3d(std::vector<vec3> points);

#endif // CONVEX_H
