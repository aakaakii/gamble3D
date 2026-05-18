#include "lib/types.h"
#include <math.h>

namespace gm {

float vec2::length() const {
	if(x == 0 && y == 0) return 1e-9f;
	return sqrt(x * x + y * y);
}

vec2 vec2::normalized() const {
	if(x == 0 && y == 0) return {1e-9f, 1e-9f};
	float inv = 1.f / length();
	return vec2(x * inv, y * inv);
} 

float vec3::length() const {
	if(x == 0 && y == 0 && z == 0) return 1e-9f;
	return sqrt(x * x + y * y + z * z);
}

vec3 vec3::normalized() const {
	float inv = 1.f / length(); 
	if(inv == 0) return vec3();
	return vec3(x * inv, y * inv, z * inv);
} 

COLOR::COLOR(float r, float g, float b, float a): r(r), g(g), b(b), a(a) {}

} // namespace gm
