#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <cstdio>
#include <vector>

#include <lib/consts.h>
#include <lib/draw.h>
#include <lib/convex.h>
#include <lib/render.h>
#include <core/GameState.h>

using namespace std;
using gm::vec2;
using gm::vec3;
using gm::COLOR;
using gm::mix;
using gm::cross;

// ---- 爆炸效果 ----
static void spawnExplosion(gm::GameState& gs, int idx) {
	for(int t = 32; t--; )
		gs.particles.emplace_back(gs.people[idx].pos, mix(LIGHTGRAY, BEIGE, uniform()));
}

int main() {
	using enum gm::Phase;
	
	gm::GameState gs;
	gs.loadNames("resources/data.txt");
	gs.loadConfig("resources/settings.txt");
	gs.init();
	
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(gs.config.screenWidth, gs.config.screenHeight, "gamble3D");
	loadFont("resources/font.ttf", gs.allChars);
	
	Camera3D camera{};
	camera.position = {325, 0, 0};
	camera.target   = {0, 0, 0};
	camera.up       = {0, 1, 0};
	camera.fovy     = 100;
	camera.projection = CAMERA_PERSPECTIVE;
	
	SetTargetFPS(60);
	
	vec3 axis = {0, 1, 0};
	gs.phase = Menu;
	
	auto& cfg = gs.config;
	auto dieCount = [&]() -> int {
		return min(rand() % (cfg.dieMax - cfg.dieMin + 1) + cfg.dieMin, gs.N - cfg.winCnt);
	};
	int dieN = dieCount();
	int fireworksTimer = 0, fireworksStep = -1;
	
	while(!WindowShouldClose()) {
		double dt = GetFrameTime();
		auto& N = gs.N;
		
		// ===== 粒子生命周期 =====
		for(int i = 0; i < (int)gs.particles.size(); ++i) {
			gs.particles[i].update(gm::GameState::kGravity);
			if(gs.particles[i].pos.y < -1000) {
				gs.particles.erase(gs.particles.begin() + i);
				--i;
				if(gs.phase == Exploding) gs.phase = Accelerating;
			}
		}
		
		// ===== 球面斥力 =====
		if(gs.phase != Finished) {
			constexpr float kRepel = 50000.f;
			for(int i = 0; i < N; ++i) {
				for(int j = 0; j < N; ++j) {
					if(i == j) continue;
					vec3 d = gs.people[i].target - gs.people[j].target;
					gs.people[i].target += d.normalized() * kRepel / (d.length() * d.length());
				}
				gs.people[i].target = gs.people[i].target.normalized() * gm::GameState::kSphereRadius;
			}
		}
		
		// ===== 位置/大小插值 =====
		for(int i = 0; i < N; ++i) {
			gs.people[i].pos   = mix(gs.people[i].pos,   gs.people[i].target,    .1f);
			gs.people[i].rad   = mix(gs.people[i].rad,   gs.people[i].targetRad, .1f);
			gs.tags[i].pos     = mix(gs.tags[i].pos,     gs.tags[i].target,      .2f);
		}
		
		// ===== 标签位置更新 =====
		for(int i = 0; i < N; ++i) {
			gs.tags[i].target = {75, (float)(i % 30) * 30 + 55};
		}
		
		// ===== 输入 =====
		if(gs.phase == Menu) {
			if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
				vec2 p = GetMousePosition();
				int w = cfg.screenWidth, h = cfg.screenHeight;
				if(abs(p.x - w/2.f) <= 300 && abs(p.y - h/2.f) <= 125)
					gs.mouseDown = true;
			}
			if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && gs.mouseDown) {
				gs.mouseDown = false;
				gs.phase = Accelerating;
			}
		}
		
		// ===== 状态机 =====
		if(gs.phase == Accelerating) {
			float add = (float)cfg.targetSpeed / (float)(cfg.state1time / dt);
			if(gs.rotationSpeed < (float)cfg.targetSpeed)
				gs.rotationSpeed += add;
			else
				gs.phase = Decelerating;
		}
		
		if(IsKeyPressed(KEY_SPACE)) {
			gs.phase = Decelerating;
			gs.rotationSpeed = 0;
		}
		
		if(gs.phase == Decelerating) {
			float ratio = exp((log(0.001) - log(cfg.targetSpeed)) / (cfg.state2time / dt));
			if(gs.rotationSpeed > 0.001f)
				gs.rotationSpeed *= ratio;
			else
				gs.phase = Eliminating;
		}
		
		// ===== 按最近距离排序 =====
		vector<int> order(N);
		for(int i = 0; i < N; ++i) order[i] = i;
		sort(order.begin(), order.end(), [&](int a, int b){
			return (gs.people[a].pos - camera.position).length()
			     < (gs.people[b].pos - camera.position).length();
		});
		
		for(int i = 0; i < N; ++i)
			gs.people[i].targetRad = 150000.f / pow((gs.people[i].pos - camera.position).length(), 1.5f);
		
		for(int i = 0; i < dieN; ++i)
			gs.people[order[i]].targetRad += 120;
		
		// ===== 淘汰 =====
		if(gs.phase == Eliminating) {
			vector<int> del;
			for(int i = 0; i < dieN; ++i) del.push_back(order[i]);
			sort(del.begin(), del.end());
			for(int i = (int)del.size() - 1; i >= 0; --i) {
				spawnExplosion(gs, del[i]);
				gs.people.erase(gs.people.begin() + del[i]);
				gs.tags.erase(gs.tags.begin() + del[i]);
				--N;
			}
			gs.phase = Exploding;
			dieN = dieCount();
			if(N == cfg.winCnt) gs.phase = Finished;
		}
		
		// ===== 获胜动画 =====
		if(gs.phase == Finished) {
			constexpr int L = -200, R = 200;
			for(int i = 0; i < cfg.winCnt; ++i) {
				gs.people[i].target = {0, L + 1.f * (R - L) * (i + 1) / (cfg.winCnt + 1), 0};
				gs.people[i].targetRad = 175;
			}
			auto camTarget = mix((vec3)camera.position, vec3{0, 0, -200}, .1f);
			camera.position = {camTarget.x, camTarget.y, camTarget.z};
			camera.up = {0, 1, 0};
			if(--fireworksTimer <= 0) {
				for(int t = 64; t--; )
					gs.particles.emplace_back(vec3{-200.f, fireworksStep * 50.f, 0}, mix(RED, GOLD, uniform()));
				for(int t = 64; t--; )
					gs.particles.emplace_back(vec3{ 200.f, fireworksStep * 50.f, 0}, mix(RED, GOLD, uniform()));
				fireworksTimer = 20;
				fireworksStep = (fireworksStep == 4) ? -4 : fireworksStep + 1;
			}
		}
		
		// ===== 渲染 =====
		BeginBloomRender();
		BeginMode3D(camera);
		
		if(gs.phase != Finished) {
			axis = vec3(Vector3Transform(Vector3{axis.x, axis.y, axis.z},
				MatrixRotate(camera.position, uniform() * .02f)));
			camera.position = Vector3Transform(camera.position,
				MatrixRotate(Vector3{axis.x, axis.y, axis.z}, gs.rotationSpeed));
			camera.up = Vector3Transform(camera.up,
				MatrixRotate(Vector3{axis.x, axis.y, axis.z}, gs.rotationSpeed));
		}
		
		if(gs.phase != Finished) {
			Color color = ColorFromHSV(GetTime() * 30, 1, 1);
			vector<vec3> poss;
			for(int i = 0; i < N; ++i) poss.push_back(gs.people[i].pos);
			for(auto& [a, b] : convex3d(poss))
				DrawCylinderEx(Vector3{a.x, a.y, a.z}, Vector3{b.x, b.y, b.z}, 1, 1, 16, color);
		}
		
		for(auto& p : gs.particles)
			DrawSphere(Vector3{p.pos.x, p.pos.y, p.pos.z}, 1, p.color);
		
		for(auto& s : gs.stars)
			DrawSphere(Vector3{s.x, s.y, s.z}, 1, WHITE);
		
		EndMode3D();
		
		// 3D 人名标签
		for(int i = 0; i < N; ++i) {
			Color color = ColorFromHSV(GetTime() * 30 + i * 37 % 360, .2f, .6f);
			DrawTextPlus(GetWorldToScreen(
				Vector3{gs.people[i].pos.x, gs.people[i].pos.y, gs.people[i].pos.z}, camera),
				gs.people[i].rad, gs.people[i].text.data(), color);
		}
		
		// 开始按钮
		if(gs.phase == Menu) {
			int w = cfg.screenWidth, h = cfg.screenHeight;
			COLOR col = gs.mouseDown
				? COLOR(255, 255, 255, 204)
				: COLOR(192, 192, 192, 153);
			DrawRectangle((w - 600) / 2, (h - 250) / 2, 600, 250, col);
			DrawTextPlus({w / 2.f, h / 2.f}, 64, "PLAY\n", WHITE);
		}
		
		// HUD
		static char buf[32];
		snprintf(buf, sizeof(buf), "剩余人数: %d", N);
		DrawTextPlus({75, 25}, 25, buf);
		DrawFPS(cfg.screenWidth - 100, cfg.screenHeight - 50);
		
		// 侧边栏姓名
		for(int i = 0; i < N; ++i)
			DrawTextPlus(gs.tags[i].pos, 25, gs.tags[i].text.data());
		
		EndBloomRender();
	}
	
	unloadFont();
	CloseWindow();
	return 0;
}
