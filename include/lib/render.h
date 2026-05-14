#ifndef RENDER_H
#define RENDER_H
#include <raylib.h>

void InitBloom();
void BeginBloomRender();
void EndBloomRender();

// Bloom 参数 (可外部调节)
extern float g_bloomThreshold;
extern float g_bloomIntensity;
extern int g_blurIterations;

#endif
