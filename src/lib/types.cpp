#include "lib/types.h"
#include <math.h>

float vec2::leng() {
	if(x == 0 and y == 0) return 1e-9;
	return sqrt(x * x + y * y);
}

vec2 vec2::normal() {
	if(x == 0 and y == 0) return {1e-9, 1e-9};
	float l = leng();
	return vec2(x / l, y / l);
} 

float vec3::leng() {
	if(x == 0 and y == 0) return 1e-9;
	return sqrt(x * x + y * y + z * z);
}

vec3 vec3::normal() {
	float l = 1 / leng(); 
	if(l == 0) return vec3();
	return vec3(x * l, y * l, z * l);
} 

COLOR::COLOR(float r, float g, float b, float a): r(r), g(g), b(b), a(a) {}
