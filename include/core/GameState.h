#ifndef CORE_GAMESTATE_H
#define CORE_GAMESTATE_H

#include "lib/types.h"
#include "core/Config.h"
#include <cstdlib>
#include <string>
#include <vector>

namespace gm {

// ---- 状态机 ----
enum class Phase {
	Menu,
	Accelerating,
	Decelerating,
	Eliminating,
	Exploding,
	Finished
};

// ---- 3D 粒子 ----
struct Particle {
	vec3 pos, vel;
	COLOR color;
	
	Particle(vec3 pos, COLOR color);
	void update(const vec3& gravity);
};

// ---- 侧边栏姓名标签 ----
struct NameTag {
	std::string text;
	vec2 pos, target;
};

// ---- 球面上的一个人 ----
struct Person {
	std::string text;
	vec3 pos, target;
	float rad = 0.f;
	float targetRad = 0.f;
};

// ---- 全局游戏状态 ----
struct GameState {
	// 配置
	Config config;
	
	// 数据
	std::vector<std::string> names;
	std::vector<Person>    people;
	std::vector<Particle>  particles;
	std::vector<vec3>      stars;
	std::vector<NameTag>   tags;
	std::string            allChars;
	
	// 状态机
	Phase phase = Phase::Menu;
	
	// 摄像机 & 球体参数
	static inline constexpr float kSphereRadius = 200.f;
	static inline constexpr vec3  kGravity      = {0.f, -.3f, 0.f};
	
	// 运行时变量
	int   N = 0;
	float rotationSpeed = .004f;
	int   dieN          = 0;     // 本轮淘汰人数
	bool  mouseDown     = false;
	
	// 烟花效果（获胜时）
	int fireworkTimer = 0;
	int fireworkStep  = -1;
	
	// ---- 初始化 ----
	void loadConfig(const std::string& path);
	void loadNames(const std::string& path);
	void init();
	
	/// 计算本轮淘汰人数
	int calcDieCount() const {
		return std::min(rand() % (config.dieMax - config.dieMin + 1) + config.dieMin, N - config.winCnt);
	}
};

} // namespace gm

#endif
