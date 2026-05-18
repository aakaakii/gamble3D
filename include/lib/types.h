#ifndef TYPES_H
#define TYPES_H
#include <raylib.h>

namespace gm {

struct vec3 {
	float x, y, z; 
	
	constexpr vec3(): x(0), y(0), z(0) {}
	vec3(const Vector3& v): x(v.x), y(v.y), z(v.z) {}
	constexpr vec3(float x, float y, float z): x(x), y(y), z(z) {}
	
	float length() const;
	vec3 normalized() const;
	explicit operator Vector3() const {return {x, y, z};}
};

inline bool operator ==(const vec3& a, const vec3& b) {return a.x == b.x && a.y == b.y && a.z == b.z;}
inline vec3 operator + (const vec3& a, const vec3& b) {return {a.x + b.x, a.y + b.y, a.z + b.z};}
inline vec3 operator - (const vec3& a, const vec3& b) {return {a.x - b.x, a.y - b.y, a.z - b.z};}
inline vec3 operator * (const vec3& a, float b) {return {a.x * b, a.y * b, a.z * b};}
inline vec3 operator * (float b, const vec3& a) {return {a.x * b, a.y * b, a.z * b};}
inline vec3 operator / (const vec3& a, float b) {return {a.x / b, a.y / b, a.z / b};}
inline vec3& operator += (vec3& a, const vec3& b) {return a = a + b;}
inline vec3& operator -= (vec3& a, const vec3& b) {return a = a - b;}
inline vec3& operator *= (vec3& a, float b) {return a = a * b;}
inline vec3 operator - (const vec3& a) {return {-a.x, -a.y, -a.z};}
inline float operator * (const vec3& a, const vec3& b) {return a.x * b.x + a.y * b.y + a.z * b.z;}
inline vec3 mix(const vec3& a, const vec3& b, float p) {return a * (1 - p) + b * p;} 
inline vec3 cross(const vec3& a, const vec3& b) {
	return {
		a.y * b.z - a.z * b.y, 
		a.z * b.x - a.x * b.z, 
		a.x * b.y - a.y * b.x
	};
}

struct vec2 {
	float x, y; 
	
	constexpr vec2(): x(0), y(0) {}
	constexpr vec2(float x, float y): x(x), y(y) {}
	vec2(const Vector2& v): x(v.x), y(v.y) {}
	
	float length() const;
	vec2 normalized() const;
	operator Vector2() const {return {x, y};}
};

inline bool operator ==(const vec2& a, const vec2& b) {return a.x == b.x && a.y == b.y;}
inline vec2 operator + (const vec2& a, const vec2& b) {return {a.x + b.x, a.y + b.y};}
inline vec2 operator - (const vec2& a, const vec2& b) {return {a.x - b.x, a.y - b.y};}
inline vec2 operator * (const vec2& a, float b) {return {a.x * b, a.y * b};}
inline vec2 operator * (float b, const vec2& a) {return {a.x * b, a.y * b};}
inline vec2 operator / (const vec2& a, float b) {return {a.x / b, a.y / b};}
inline vec2& operator += (vec2& a, const vec2& b) {return a = a + b;}
inline vec2& operator -= (vec2& a, const vec2& b) {return a = a - b;}
inline vec2 operator - (const vec2& a) {return {-a.x, -a.y};}
inline float operator * (const vec2& a, const vec2& b) {return a.x * b.x + a.y * b.y;}
inline vec2 mix(const vec2& a, const vec2& b, float p) {return a * (1 - p) + b * p;} 
inline float cross(const vec2& a, const vec2& b) {return a.x * b.y - a.y * b.x;}

struct COLOR {
	float r, g, b, a;
	COLOR(float r, float g, float b, float a = 255);
	COLOR(const Color& c): r(c.r), g(c.g), b(c.b), a(c.a) {}
	operator Color() const {return {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a};}
};

} // namespace gm

#endif
