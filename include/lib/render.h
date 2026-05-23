#ifndef RENDER_H
#define RENDER_H
#include <raylib.h>

// 前置声明
namespace gm { struct Config; }

void InitBloom();
void BeginBloomRender();
void EndBloomRender();

// 窗口大小变化时重建 RenderTexture
void ResizeBloom(int width, int height);

// Bloom 参数 (可外部调节)
extern bool g_bloomEnabled;
extern float g_bloomThreshold;
extern float g_bloomIntensity;
extern int g_blurIterations;

// 从 Config 应用 bloom 参数
void ApplyBloomConfig(const gm::Config& config);

#endif
