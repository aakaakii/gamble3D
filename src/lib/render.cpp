#include "lib/render.h"
#include <raymath.h>
#include <rlgl.h>

// Bloom 参数默认值
float g_bloomThreshold = .1f;
float g_bloomIntensity = 0.8f;
int g_blurIterations = 6;
bool g_bloomEnabled = true;

// Motion Blur 参数默认值
float g_motionBlurStrength = 1.5f;
bool g_motionBlurEnabled = true;

// Motion Blur 重投影矩阵
Matrix g_prevViewProj = MatrixIdentity();
Matrix g_currViewProjInv = MatrixIdentity();

// Bloom 相关资源
static RenderTexture2D sceneTarget;
static RenderTexture2D brightnessTarget;
static RenderTexture2D hBlurTarget[2];
static RenderTexture2D vBlurTarget[2];

static Shader brightnessExtractShader;
static Shader gaussianBlurShader;
static Shader bloomMotionCompositeShader;

static bool bloomInitialized = false;
static int screenWidth = 0;
static int screenHeight = 0;

// ── 创建带可采样深度纹理的 FBO ──
static void CreateFBOWithDepth(RenderTexture2D& rt, int w, int h) {
	rt.id = rlLoadFramebuffer();
	
	// 颜色纹理
	rt.texture.id = rlLoadTexture(NULL, w, h, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
	rt.texture.width = w;
	rt.texture.height = h;
	rt.texture.mipmaps = 1;
	rt.texture.format = RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	rlTextureParameters(rt.texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
	rlTextureParameters(rt.texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
	rlFramebufferAttach(rt.id, rt.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
	
	// 深度纹理（可采样！关键区别）
	rt.depth.id = rlLoadTextureDepth(w, h, false);       // false = texture, not renderbuffer
	rt.depth.width = w;
	rt.depth.height = h;
	rt.depth.mipmaps = 1;
	rt.depth.format = RL_PIXELFORMAT_UNCOMPRESSED_R32;
	rlTextureParameters(rt.depth.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);
	rlTextureParameters(rt.depth.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
	rlTextureParameters(rt.depth.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
	rlTextureParameters(rt.depth.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
	rlFramebufferAttach(rt.id, rt.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
	
	if(!rlFramebufferComplete(rt.id)) {
		TraceLog(LOG_WARNING, "Custom FBO (depth tex) incomplete!");
	}
}

// ── 销毁自定义 FBO ──
static void DestroyFBO(RenderTexture2D& rt) {
	rlUnloadFramebuffer(rt.id);
	rlUnloadTexture(rt.texture.id);
	rlUnloadTexture(rt.depth.id);
	rt.id = 0;
	rt.texture.id = 0;
	rt.depth.id = 0;
}

void InitBloom() {
	if(bloomInitialized) return;
	
	screenWidth = GetScreenWidth();
	screenHeight = GetScreenHeight();
	
	// sceneTarget: 使用带可采样深度纹理的自定义 FBO
	CreateFBOWithDepth(sceneTarget, screenWidth, screenHeight);
	
	// 其他 RenderTexture 不需要深度纹理，用标准 LoadRenderTexture
	brightnessTarget = LoadRenderTexture(screenWidth, screenHeight);
	hBlurTarget[0] = LoadRenderTexture(screenWidth, screenHeight);
	hBlurTarget[1] = LoadRenderTexture(screenWidth, screenHeight);
	vBlurTarget[0] = LoadRenderTexture(screenWidth, screenHeight);
	vBlurTarget[1] = LoadRenderTexture(screenWidth, screenHeight);
	
	// 加载 shader
	brightnessExtractShader = LoadShader("resources/shaders/passthrough.vs", "resources/shaders/brightness_extract.fs");
	gaussianBlurShader = LoadShader("resources/shaders/passthrough.vs", "resources/shaders/gaussian_blur.fs");
	bloomMotionCompositeShader = LoadShader("resources/shaders/passthrough.vs", "resources/shaders/bloom_motion_composite.fs");
	
	if(!brightnessExtractShader.id || !gaussianBlurShader.id || !bloomMotionCompositeShader.id) {
		TraceLog(LOG_WARNING, "Bloom/Motion Blur: Failed to load some shaders");
		bloomInitialized = false;
		return;
	}
	
	// 设置分辨率 uniform
	Vector2 res = {(float)screenWidth, (float)screenHeight};
	int locRes = GetShaderLocation(gaussianBlurShader, "resolution");
	SetShaderValue(gaussianBlurShader, locRes, &res, SHADER_UNIFORM_VEC2);
	
	bloomInitialized = true;
	TraceLog(LOG_INFO, "Bloom & Motion Blur: Initialized successfully (depth texture FBO)");
}

void ResizeBloom(int width, int height) {
	if(!bloomInitialized || (screenWidth == width && screenHeight == height)) return;
	
	// 卸载旧的
	DestroyFBO(sceneTarget);
	UnloadRenderTexture(brightnessTarget);
	UnloadRenderTexture(hBlurTarget[0]);
	UnloadRenderTexture(hBlurTarget[1]);
	UnloadRenderTexture(vBlurTarget[0]);
	UnloadRenderTexture(vBlurTarget[1]);
	
	screenWidth  = width;
	screenHeight = height;
	
	// 重建
	CreateFBOWithDepth(sceneTarget, screenWidth, screenHeight);
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
	
	// 渲染到场景纹理（BeginDrawing 由外部调用）
	BeginTextureMode(sceneTarget);
	ClearBackground({0, 0, 0, 255});
}

void EndBloomRender() {
	EndTextureMode();  // 结束 sceneTarget
	
	if(!bloomInitialized) {
		return;
	}
	
	// Pass 1: 提取亮部（仅当 Bloom 启用时）
	if(g_bloomEnabled) {
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
	}
	
	// Pass 3: 最终合成 (Bloom + 重投影 Motion Blur)
	int lastBlurIdx = (g_blurIterations - 1) % 2;
	
	int locBloomIntensity = GetShaderLocation(bloomMotionCompositeShader, "bloomIntensity");
	int locBloomEnabled = GetShaderLocation(bloomMotionCompositeShader, "bloomEnabled");
	int locMotionBlurStrengthLoc = GetShaderLocation(bloomMotionCompositeShader, "motionBlurStrength");
	int locMotionBlurEnabledLoc = GetShaderLocation(bloomMotionCompositeShader, "motionBlurEnabled");
	int locCurrViewProjInv = GetShaderLocation(bloomMotionCompositeShader, "currViewProjInv");
	int locPrevViewProj = GetShaderLocation(bloomMotionCompositeShader, "prevViewProj");
	
	BeginShaderMode(bloomMotionCompositeShader);
	SetShaderValue(bloomMotionCompositeShader, locBloomIntensity, &g_bloomIntensity, SHADER_UNIFORM_FLOAT);
	
	float bloomEnabledVal = g_bloomEnabled ? 1.0f : 0.0f;
	float motionBlurEnabledVal = g_motionBlurEnabled ? 1.0f : 0.0f;
	SetShaderValue(bloomMotionCompositeShader, locBloomEnabled, &bloomEnabledVal, SHADER_UNIFORM_FLOAT);
	SetShaderValue(bloomMotionCompositeShader, locMotionBlurStrengthLoc, &g_motionBlurStrength, SHADER_UNIFORM_FLOAT);
	SetShaderValue(bloomMotionCompositeShader, locMotionBlurEnabledLoc, &motionBlurEnabledVal, SHADER_UNIFORM_FLOAT);
	
	// 设置矩阵 uniforms
	rlSetUniformMatrix(locCurrViewProjInv, g_currViewProjInv);
	rlSetUniformMatrix(locPrevViewProj, g_prevViewProj);
	
	// 绑定纹理
	int locTex0 = GetShaderLocation(bloomMotionCompositeShader, "texture0");
	int locTex1 = GetShaderLocation(bloomMotionCompositeShader, "texture1");
	int locTexDepth = GetShaderLocation(bloomMotionCompositeShader, "textureDepth");
	
	// 激活纹理单元
	rlActiveTextureSlot(0);
	rlEnableTexture(sceneTarget.texture.id);
	rlActiveTextureSlot(1);
	rlEnableTexture(g_bloomEnabled ? vBlurTarget[lastBlurIdx].texture.id : brightnessTarget.texture.id);
	rlActiveTextureSlot(2);
	rlEnableTexture(sceneTarget.depth.id);   // ← 现在是真正的纹理！
	
	// 设置纹理单元 uniform
	int texUnit0 = 0, texUnit1 = 1, texUnitDepth = 2;
	SetShaderValue(bloomMotionCompositeShader, locTex0, &texUnit0, SHADER_UNIFORM_INT);
	SetShaderValue(bloomMotionCompositeShader, locTex1, &texUnit1, SHADER_UNIFORM_INT);
	SetShaderValue(bloomMotionCompositeShader, locTexDepth, &texUnitDepth, SHADER_UNIFORM_INT);
	
	// 绘制
	DrawTextureRec(sceneTarget.texture, {0, 0, (float)sceneTarget.texture.width, -(float)sceneTarget.texture.height}, {0, 0}, WHITE);
	
	EndShaderMode();
	
	// 恢复默认纹理单元
	rlActiveTextureSlot(0);
	
	// EndDrawing 由外部调用
}
