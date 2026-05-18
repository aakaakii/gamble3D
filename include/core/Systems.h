#ifndef CORE_SYSTEMS_H
#define CORE_SYSTEMS_H

#include "core/GameState.h"
#include <raylib.h>
#include <vector>

namespace gm {

// ===================== Update Systems =====================

/// 粒子物理更新 + 越界回收
void updateParticles(GameState& gs);

/// 球面斥力 (O(N²)，Finished 阶段自动跳过)
void updateRepulsion(GameState& gs);

/// 人物/标签位置插值 + 侧边栏定位
void updateInterpolation(GameState& gs);

/// 输入轮询 (Menu 点击开始、Space 跳过)
void pollInput(GameState& gs);

/// 旋转速度状态机 (加速→减速→淘汰)
void updateCameraSpin(GameState& gs, double dt);

/// 按距离排序 + 计算 targetRad
/// @return 排序索引 (近→远)
std::vector<int> rankByDistance(GameState& gs, const Camera3D& cam);

/// 淘汰执行 (仅 Eliminating 阶段生效)
void processElimination(GameState& gs, const std::vector<int>& order);

/// 获胜动画 (排成一行 + 摄像机推近 + 烟花)
void updateWinnerAnimation(GameState& gs, Camera3D& cam);

// ===================== Render Systems =====================

/// 摄像机旋转矩阵应用到 camera/axis
void applyCameraRotation(const GameState& gs, Camera3D& cam, vec3& axis);

/// 渲染 3D 世界 (凸包 + 粒子 + 星空)
void render3DWorld(const GameState& gs);

/// 渲染 2D 覆盖层 (3D人名标签 + HUD + 侧边栏)
void renderOverlay(const GameState& gs, const Camera3D& cam);

/// 渲染 Menu 开始按钮
void renderMenuButton(const GameState& gs);

} // namespace gm

#endif
