#include "core/GameState.h"
#include "lib/consts.h"
#include <fstream>
#include <algorithm>
#include <ctime>

namespace gm {

// ---- Particle ----
Particle::Particle(vec3 pos, COLOR color) : pos(pos), color(color) {
	auto rnd = []{ return uniform() - .5f; };
	vel = vec3(rnd(), rnd(), rnd()).normalized() * 2.f + vec3(0, uniform() * 2.f, 0);
}

void Particle::update(const vec3& gravity) {
	pos += vel;
	vel += gravity;
}

// ---- GameState ----
void GameState::loadConfig(const std::string& path) {
	config = Config::load(path);
}

void GameState::loadNames(const std::string& path) {
	std::ifstream fin(path);
	std::string tmp;
	while(fin >> tmp) names.push_back(tmp);
}

void GameState::init() {
	srand(time(nullptr));
	
	N = (int)names.size();
	people.resize(N);
	tags.resize(N);
	
	allChars = "剩余人数: 01234567890PLAY";
	
	auto rndvec = []{
		return vec3(uniform() - .5f, uniform() - .5f, uniform() - .5f).normalized();
	};
	
	for(int i = 0; i < N; ++i) {
		people[i].target = rndvec() * kSphereRadius;
		tags[i].text = people[i].text = names[i];
		allChars += people[i].text;
	}
	
	for(int i = 0; i < 500; ++i) {
		stars.push_back(rndvec() * 3.f * kSphereRadius);
	}
}

} // namespace gm
