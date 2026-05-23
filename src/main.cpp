#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <lib/consts.h>
#include <lib/render.h>
#include <core/GameState.h>
#include <core/Systems.h>

int main() {
	using enum gm::Phase;

	gm::GameState gs;
	gs.loadNames("resources/data.txt");
	gs.loadConfig("resources/settings.json");
	gs.init();
	gs.dieN = gs.calcDieCount();

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(gs.config.screenWidth, gs.config.screenHeight, "lottery3D");
	loadFont("resources/font.ttf", gs.allChars);

	// 应用 Bloom 和 Motion Blur 配置
	extern float g_bloomThreshold;
	extern float g_bloomIntensity;
	extern int g_blurIterations;
	extern bool g_bloomEnabled;
	extern float g_motionBlurStrength;
	extern bool g_motionBlurEnabled;
	extern Matrix g_prevViewProj;
	extern Matrix g_currViewProjInv;
	
	g_bloomEnabled = gs.config.bloomEnabled;
	g_bloomThreshold = gs.config.bloomThreshold;
	g_bloomIntensity = gs.config.bloomIntensity;
	g_blurIterations = gs.config.bloomIterations;
	g_motionBlurEnabled = gs.config.motionBlurEnabled;
	g_motionBlurStrength = gs.config.motionBlurStrength;

	Camera3D camera{};
	camera.position = {325, 0, 0};
	camera.target   = {0, 0, 0};
	camera.up       = {0, 1, 0};
	camera.fovy     = 100;
	camera.projection = CAMERA_PERSPECTIVE;

	SetTargetFPS(60);
	gm::vec3 axis = {0, 1, 0};
	gs.phase = Menu;

	while(!WindowShouldClose()) {
		// ====== UPDATE ======
		gm::updateParticles(gs);
		gm::updateRepulsion(gs);
		gm::updateInterpolation(gs);
		gm::pollInput(gs);
		gm::updateCameraSpin(gs, GetFrameTime());

		if(gs.phase != Finished) {
			auto order = gm::rankByDistance(gs, camera);
			gm::processElimination(gs, order);
		}

		if(gs.phase == Finished)
			gm::updateWinnerAnimation(gs, camera);

		// ====== RENDER ======
		BeginDrawing();
		ClearBackground({0, 0, 0, 255});
		
		// 1) 渲染 3D 场景到 offscreen 纹理
		BeginBloomRender();
		BeginMode3D(camera);
		
		// 在 applyCameraRotation 之前捕获矩阵（当前帧的变换）
		Matrix proj = rlGetMatrixProjection();
		Matrix view = rlGetMatrixModelview();
		Matrix currViewProj = MatrixMultiply(proj, view);
		g_currViewProjInv = MatrixInvert(currViewProj);
		
		gm::applyCameraRotation(gs, camera, axis);
		gm::render3DWorld(gs);
		EndMode3D();
		EndBloomRender();  // 后处理：bloom + 重投影 motion blur → 输出到屏幕
		
		// 2) UI 在后处理之后绘制，永远不被模糊
		gm::renderOverlay(gs, camera);
		gm::renderMenuButton(gs);
		
		EndDrawing();
		
		// 保存当前帧 viewProj 作为下一帧的"前一帧"
		g_prevViewProj = currViewProj;
	}

	unloadFont();
	CloseWindow();
	return 0;
}
