#ifndef RENDER_H
#define RENDER_H
#include <raylib.h>

void InitBloom();
void BeginBloomRender();
void EndBloomRender();

// 窗口大小变化时重建 RenderTexture
void ResizeBloom(int width, int height);

// Bloom 参数 (可外部调节)
extern float g_bloomThreshold;
extern float g_bloomIntensity;
extern int g_blurIterations;
extern bool g_bloomEnabled;

// Motion Blur 参数 (可外部调节)
extern float g_motionBlurStrength;
extern bool g_motionBlurEnabled;

// Motion Blur 重投影矩阵 (由 main.cpp 每帧更新)
extern Matrix g_prevViewProj;       // 前一帧 viewProj
extern Matrix g_currViewProjInv;    // 当前帧 viewProj 的逆矩阵

#endif
