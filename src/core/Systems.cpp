#include "core/Systems.h"

#include "lib/consts.h"
#include "lib/draw.h"
#include "lib/convex.h"

#include <raymath.h>
#include <algorithm>
#include <cstdio>
#include <cmath>

namespace gm {

using ::uniform;   // 全局函数
using ::mix;       // 全局 mix(float,...) / mix(COLOR,...) 重载

using enum Phase;  // C++20

// ===================== Update Systems =====================

void updateParticles(GameState& gs) {
	for(int i = 0; i < (int)gs.particles.size(); ++i) {
		gs.particles[i].update(GameState::kGravity);
		if(gs.particles[i].pos.y < -1000.f) {
			gs.particles.erase(gs.particles.begin() + i);
			--i;
			if(gs.phase == Exploding) gs.phase = Accelerating;
		}
	}
}

void updateRepulsion(GameState& gs) {
	if(gs.phase == Finished) return;

	constexpr float kRepel = 50000.f;
	for(int i = 0; i < gs.N; ++i) {
		for(int j = 0; j < gs.N; ++j) {
			if(i == j) continue;
			vec3 d = gs.people[i].target - gs.people[j].target;
			gs.people[i].target += d.normalized() * kRepel / (d.length() * d.length());
		}
		gs.people[i].target = gs.people[i].target.normalized() * GameState::kSphereRadius;
	}
}

void updateInterpolation(GameState& gs) {
	for(int i = 0; i < gs.N; ++i) {
		gs.people[i].pos = gm::mix(gs.people[i].pos, gs.people[i].target, .1f);
		gs.people[i].rad = mix(gs.people[i].rad,  gs.people[i].targetRad, .1f);
		gs.tags[i].pos   = gm::mix(gs.tags[i].pos, gs.tags[i].target,      .2f);
	}
	for(int i = 0; i < gs.N; ++i) {
		gs.tags[i].target = {75.f, (float)(i % 30) * 30.f + 55.f};
	}
}

void pollInput(GameState& gs) {
	const auto& cfg = gs.config;

	if(gs.phase == Menu) {
		if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			vec2 p = GetMousePosition();
			int w = cfg.screenWidth, h = cfg.screenHeight;
			if(std::abs(p.x - w / 2.f) <= 300.f && std::abs(p.y - h / 2.f) <= 125.f)
				gs.mouseDown = true;
		}
		if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && gs.mouseDown) {
			gs.mouseDown = false;
			gs.phase     = Accelerating;
		}
	}

	if(IsKeyPressed(KEY_SPACE)) {
		gs.phase         = Decelerating;
		gs.rotationSpeed = 0.f;
	}
}

void updateCameraSpin(GameState& gs, double dt) {
	const auto& cfg = gs.config;

	if(gs.phase == Accelerating) {
		float add = (float)cfg.targetSpeed / (float)(cfg.state1time / dt);
		if(gs.rotationSpeed < (float)cfg.targetSpeed)
			gs.rotationSpeed += add;
		else
			gs.phase = Decelerating;
	}

	if(gs.phase == Decelerating) {
		float ratio = std::exp((std::log(0.001) - std::log(cfg.targetSpeed))
		                       / (cfg.state2time / dt));
		if(gs.rotationSpeed > 0.001f)
			gs.rotationSpeed *= ratio;
		else
			gs.phase = Eliminating;
	}
}

std::vector<int> rankByDistance(GameState& gs, const Camera3D& cam) {
	std::vector<int> order(gs.N);
	for(int i = 0; i < gs.N; ++i) order[i] = i;

	vec3 camPos = cam.position;
	std::sort(order.begin(), order.end(), [&](int a, int b) {
		return (gs.people[a].pos - camPos).length()
		     < (gs.people[b].pos - camPos).length();
	});

	for(int i = 0; i < gs.N; ++i)
		gs.people[i].targetRad = 150000.f / std::pow((gs.people[i].pos - camPos).length(), 1.5f);

	for(int i = 0; i < gs.dieN; ++i)
		gs.people[order[i]].targetRad += 120.f;

	return order;
}

void processElimination(GameState& gs, const std::vector<int>& order) {
	if(gs.phase != Eliminating) return;

	std::vector<int> del;
	for(int i = 0; i < gs.dieN; ++i) del.push_back(order[i]);
	std::sort(del.begin(), del.end());

	for(int i = (int)del.size() - 1; i >= 0; --i) {
		// 爆炸粒子
		for(int t = 32; t--; )
			gs.particles.emplace_back(
				gs.people[del[i]].pos,
				mix(LIGHTGRAY, BEIGE, uniform()));

		gs.people.erase(gs.people.begin() + del[i]);
		gs.tags.erase(gs.tags.begin() + del[i]);
		--gs.N;
	}

	gs.phase = Exploding;
	gs.dieN  = gs.calcDieCount();
	if(gs.N == gs.config.winCnt) gs.phase = Finished;
}

void updateWinnerAnimation(GameState& gs, Camera3D& cam) {
	if(gs.phase != Finished) return;

	constexpr int L = -200, R = 200;
	for(int i = 0; i < gs.config.winCnt; ++i) {
		gs.people[i].target    = {0.f, L + 1.f * (R - L) * (i + 1) / (gs.config.winCnt + 1), 0.f};
		gs.people[i].targetRad = 175.f;
	}

	auto camTarget = gm::mix((vec3)cam.position, vec3{0.f, 0.f, -200.f}, .1f);
	cam.position = {camTarget.x, camTarget.y, camTarget.z};
	cam.up       = {0.f, 1.f, 0.f};

	if(--gs.fireworkTimer <= 0) {
		for(int t = 64; t--; )
			gs.particles.emplace_back(
				vec3{-200.f, gs.fireworkStep * 50.f, 0.f},
				mix(RED, GOLD, uniform()));
		for(int t = 64; t--; )
			gs.particles.emplace_back(
				vec3{200.f, gs.fireworkStep * 50.f, 0.f},
				mix(RED, GOLD, uniform()));
		gs.fireworkTimer = 20;
		gs.fireworkStep  = (gs.fireworkStep == 4) ? -4 : gs.fireworkStep + 1;
	}
}

// ===================== Render Systems =====================

void applyCameraRotation(const GameState& gs, Camera3D& cam, vec3& axis) {
	if(gs.phase == Finished) return;

	axis = vec3(Vector3Transform(
		Vector3{axis.x, axis.y, axis.z},
		MatrixRotate(cam.position, uniform() * .02f)));

	cam.position = Vector3Transform(cam.position,
		MatrixRotate(Vector3{axis.x, axis.y, axis.z}, gs.rotationSpeed));
	cam.up = Vector3Transform(cam.up,
		MatrixRotate(Vector3{axis.x, axis.y, axis.z}, gs.rotationSpeed));
}

void render3DWorld(const GameState& gs) {
	if(gs.phase != Finished) {
		Color color = ColorFromHSV(GetTime() * 30.f, 1.f, 1.f);
		std::vector<vec3> poss;
		for(int i = 0; i < gs.N; ++i) poss.push_back(gs.people[i].pos);
		for(auto& [a, b] : convex3d(poss))
			DrawCylinderEx(Vector3{a.x, a.y, a.z}, Vector3{b.x, b.y, b.z}, 1.f, 1.f, 16, color);
	}

	for(auto& p : gs.particles)
		DrawSphere(Vector3{p.pos.x, p.pos.y, p.pos.z}, 1.f, p.color);

	for(auto& s : gs.stars)
		DrawSphere(Vector3{s.x, s.y, s.z}, 1.f, WHITE);
}

void renderOverlay(const GameState& gs, const Camera3D& cam) {
	// 3D → 2D 名字标签
	for(int i = 0; i < gs.N; ++i) {
		Color color = ColorFromHSV(GetTime() * 30.f + i * 37 % 360, .2f, .6f);
		Vector2 screen = GetWorldToScreen(
			Vector3{gs.people[i].pos.x, gs.people[i].pos.y, gs.people[i].pos.z}, cam);
		DrawTextPlus(screen, gs.people[i].rad, gs.people[i].text.data(), color);
	}

	// HUD
	static char buf[32];
	std::snprintf(buf, sizeof(buf), "剩余人数: %d", gs.N);
	DrawTextPlus({75.f, 25.f}, 25.f, buf);
	DrawFPS(gs.config.screenWidth - 100, gs.config.screenHeight - 50);

	// 侧边栏姓名
	for(int i = 0; i < gs.N; ++i)
		DrawTextPlus(gs.tags[i].pos, 25.f, gs.tags[i].text.data());
}

void renderMenuButton(const GameState& gs) {
	if(gs.phase != Menu) return;

	int w = gs.config.screenWidth, h = gs.config.screenHeight;
	COLOR col = gs.mouseDown
		? COLOR(255.f, 255.f, 255.f, 204.f)
		: COLOR(192.f, 192.f, 192.f, 153.f);
	DrawRectangle((w - 600) / 2, (h - 250) / 2, 600, 250, col);
	DrawTextPlus({w / 2.f, h / 2.f}, 64.f, "PLAY\n", WHITE);
}

} // namespace gm
