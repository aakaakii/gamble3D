#include "lib/render.h"
#include <raymath.h>
#include <rlgl.h>

// Bloom 参数默认值
float g_bloomThreshold = 0;
float g_bloomIntensity = 0.8f;
int g_blurIterations = 8;

// Bloom 相关资源
static RenderTexture2D sceneTarget;
static RenderTexture2D brightnessTarget;
static RenderTexture2D hBlurTarget[2];
static RenderTexture2D vBlurTarget[2];

static Shader brightnessExtractShader;
static Shader gaussianBlurShader;
static Shader bloomCompositeShader;

static bool bloomInitialized = false;
static int screenWidth = 0;
static int screenHeight = 0;

void InitBloom() {
	if(bloomInitialized) return;
	
	screenWidth = GetScreenWidth();
	screenHeight = GetScreenHeight();
	
	// 创建所有 RenderTexture
	sceneTarget = LoadRenderTexture(screenWidth, screenHeight);
	brightnessTarget = LoadRenderTexture(screenWidth, screenHeight);
	hBlurTarget[0] = LoadRenderTexture(screenWidth, screenHeight);
	hBlurTarget[1] = LoadRenderTexture(screenWidth, screenHeight);
	vBlurTarget[0] = LoadRenderTexture(screenWidth, screenHeight);
	vBlurTarget[1] = LoadRenderTexture(screenWidth, screenHeight);
	
	// 加载 shader
	brightnessExtractShader = LoadShader("resources/shaders/passthrough.vs", "resources/shaders/brightness_extract.fs");
	gaussianBlurShader = LoadShader("resources/shaders/passthrough.vs", "resources/shaders/gaussian_blur.fs");
	bloomCompositeShader = LoadShader("resources/shaders/passthrough.vs", "resources/shaders/bloom_composite.fs");
	
	if(!brightnessExtractShader.id || !gaussianBlurShader.id || !bloomCompositeShader.id) {
		TraceLog(LOG_WARNING, "Bloom: Failed to load some shaders");
		bloomInitialized = false;
		return;
	}
	
	// 设置分辨率统一变量
	Vector2 res = {(float)screenWidth, (float)screenHeight};
	int locRes = GetShaderLocation(gaussianBlurShader, "resolution");
	SetShaderValue(gaussianBlurShader, locRes, &res, SHADER_UNIFORM_VEC2);
	
	bloomInitialized = true;
	TraceLog(LOG_INFO, "Bloom: Initialized successfully");
}

void ResizeBloom(int width, int height) {
	if(!bloomInitialized || (screenWidth == width && screenHeight == height)) return;
	
	// 卸载旧的
	UnloadRenderTexture(sceneTarget);
	UnloadRenderTexture(brightnessTarget);
	UnloadRenderTexture(hBlurTarget[0]);
	UnloadRenderTexture(hBlurTarget[1]);
	UnloadRenderTexture(vBlurTarget[0]);
	UnloadRenderTexture(vBlurTarget[1]);
	
	screenWidth  = width;
	screenHeight = height;
	
	// 重建
	sceneTarget      = LoadRenderTexture(screenWidth, screenHeight);
	brightnessTarget = LoadRenderTexture(screenWidth, screenHeight);
	hBlurTarget[0]   = LoadRenderTexture(screenWidth, screenHeight);
	hBlurTarget[1]   = LoadRenderTexture(screenWidth, screenHeight);
	vBlurTarget[0]   = LoadRenderTexture(screenWidth, screenHeight);
	vBlurTarget[1]   = LoadRenderTexture(screenWidth, screenHeight);
	
	Vector2 res = {(float)screenWidth, (float)screenHeight};
	int locRes = GetShaderLocation(gaussianBlurShader, "resolution");
	SetShaderValue(gaussianBlurShader, locRes, &res, SHADER_UNIFORM_VEC2);
}

void BeginBloomRender() {
	if(!bloomInitialized) InitBloom();
	
	BeginDrawing();
	ClearBackground({0, 0, 0, 255});
	
	// 渲染到场景纹理
	BeginTextureMode(sceneTarget);
	ClearBackground({0, 0, 0, 255});
}

void EndBloomRender() {
	EndTextureMode();  // 结束 sceneTarget
	
	if(!bloomInitialized) {
		EndDrawing();
		return;
	}
	
	// Pass 1: 提取亮部
	BeginTextureMode(brightnessTarget);
	ClearBackground({0, 0, 0, 255});
	BeginShaderMode(brightnessExtractShader);
	
	int locThreshold = GetShaderLocation(brightnessExtractShader, "threshold");
	SetShaderValue(brightnessExtractShader, locThreshold, &g_bloomThreshold, SHADER_UNIFORM_FLOAT);
	
	DrawTextureRec(sceneTarget.texture, {0, 0, (float)sceneTarget.texture.width, -(float)sceneTarget.texture.height}, {0, 0}, WHITE);
	
	EndShaderMode();
	EndTextureMode();
	
	// Pass 2: 多遍高斯模糊 (ping-pong)
	Vector2 dirH = {1.0f, 0.0f};
	Vector2 dirV = {0.0f, 1.0f};
	
	int locDirection = GetShaderLocation(gaussianBlurShader, "direction");
	
	for(int i = 0; i < g_blurIterations; i++) {
		int srcIdx = i % 2;
		int dstIdx = (i + 1) % 2;
		
		// 水平模糊
		BeginTextureMode(hBlurTarget[dstIdx]);
		ClearBackground({0, 0, 0, 255});
		BeginShaderMode(gaussianBlurShader);
		SetShaderValue(gaussianBlurShader, locDirection, &dirH, SHADER_UNIFORM_VEC2);
		
		if(i == 0) {
			DrawTextureRec(brightnessTarget.texture, {0, 0, (float)brightnessTarget.texture.width, -(float)brightnessTarget.texture.height}, {0, 0}, WHITE);
		} else {
			DrawTextureRec(vBlurTarget[srcIdx].texture, {0, 0, (float)vBlurTarget[srcIdx].texture.width, -(float)vBlurTarget[srcIdx].texture.height}, {0, 0}, WHITE);
		}
		
		EndShaderMode();
		EndTextureMode();
		
		// 垂直模糊
		BeginTextureMode(vBlurTarget[dstIdx]);
		ClearBackground({0, 0, 0, 255});
		BeginShaderMode(gaussianBlurShader);
		SetShaderValue(gaussianBlurShader, locDirection, &dirV, SHADER_UNIFORM_VEC2);
		
		DrawTextureRec(hBlurTarget[dstIdx].texture, {0, 0, (float)hBlurTarget[dstIdx].texture.width, -(float)hBlurTarget[dstIdx].texture.height}, {0, 0}, WHITE);
		
		EndShaderMode();
		EndTextureMode();
	}
	
	// Pass 3: 最终合成
	int lastBlurIdx = (g_blurIterations - 1) % 2;
	int locBloomIntensity = GetShaderLocation(bloomCompositeShader, "bloomIntensity");
	
	BeginShaderMode(bloomCompositeShader);
	SetShaderValue(bloomCompositeShader, locBloomIntensity, &g_bloomIntensity, SHADER_UNIFORM_FLOAT);
	
	// 绑定两个纹理
	int locTex0 = GetShaderLocation(bloomCompositeShader, "texture0");
	int locTex1 = GetShaderLocation(bloomCompositeShader, "texture1");
	
	// 激活纹理单元
	rlActiveTextureSlot(0);
	rlEnableTexture(sceneTarget.texture.id);
	rlActiveTextureSlot(1);
	rlEnableTexture(vBlurTarget[lastBlurIdx].texture.id);
	
	// 设置统一变量
	int texUnit0 = 0;
	int texUnit1 = 1;
	SetShaderValue(bloomCompositeShader, locTex0, &texUnit0, SHADER_UNIFORM_INT);
	SetShaderValue(bloomCompositeShader, locTex1, &texUnit1, SHADER_UNIFORM_INT);
	
	// 绘制场景纹理
	DrawTextureRec(sceneTarget.texture, {0, 0, (float)sceneTarget.texture.width, -(float)sceneTarget.texture.height}, {0, 0}, WHITE);
	
	EndShaderMode();
	
	// 恢复默认纹理单元
	rlActiveTextureSlot(0);
	
	EndDrawing();
}
