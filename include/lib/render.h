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

#endif
