#include <raylib.h>

#include <lib/consts.h>
#include <lib/render.h>
#include <core/GameState.h>
#include <core/Systems.h>

int main() {
	using enum gm::Phase;

	gm::GameState gs;
	gs.loadNames("resources/data.txt");
	gs.loadConfig("resources/settings.json");
	ApplyBloomConfig(gs.config);
	gs.init();
	gs.dieN = gs.calcDieCount();

	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(gs.config.screenWidth, gs.config.screenHeight, "lottery3D");
	loadFont("resources/font.ttf", gs.allChars);

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
		BeginBloomRender();
		BeginMode3D(camera);
		gm::applyCameraRotation(gs, camera, axis);
		gm::render3DWorld(gs);
		EndMode3D();
		gm::renderOverlay(gs, camera);
		gm::renderMenuButton(gs);
		EndBloomRender();
	}

	unloadFont();
	CloseWindow();
	return 0;
}
