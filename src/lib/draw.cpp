#include <math.h>
#include <random>
#include <chrono>
#include <vector>
#include <string>
using namespace std;
#include "lib/draw.h"
vector<vec2> gencircle(vec2 o, float begin, float end, float rad, float p) {
	vector<vec2> res;
	if(end < begin) p = -p;
	for(float i = begin; i <= end; i += p)
		res.push_back(o + vec2(cos(i), sin(i)) * rad);
	return res;
}
float mix(float a, float b, float p) {
	return a * (1 - p) + b * p;
}
COLOR mix(COLOR a, COLOR b, float p) {
	return (COLOR) {
		mix(a.r, b.r, p), 
		mix(a.g, b.g, p), 
		mix(a.b, b.b, p), 
		mix(a.a, b.a, p), 
	};
}
void initScreen(int fps, vec2 SCRL, const char* title) {
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
	printf("fps: %d\n", fps);
	SetTargetFPS(fps);
	InitWindow(SCRL.x, SCRL.y, title);
}

void drawCircle(vec2 pos, Color color, float rad) {
	DrawCircleV(pos, rad, color);
}
void drawLine(vec2 begin, vec2 end, Color color) {
	DrawLineV(begin, end, color);
}
void drawRect(vec2 pos, vec2 siz, COLOR color) {
	DrawRectangleV(pos, siz, color);
}
void drawCircleRing(vec2 pos, Color color, float rad, float p) {
	drawCircle(pos, mix(color, BLACK, .5), rad);
	drawCircle(pos, color, rad * (1 - p));
}
void drawTraingle(vec2 a, vec2 b, vec2 c, Color color) {
	if(cross(b-a, c-a) > 0) swap(b, c);
	DrawTriangle(a, b, c, color);
}
void drawPolygon(const vector<vec2>& dots, Color color) {
	for(int i = 2; i < (int)dots.size(); ++i)
		drawTraingle(dots[0], dots[i-1], dots[i], color);
}
void drawRoundRect(vec2 pos, vec2 side, float thick, Color color) {
	drawPolygon(
	    gencircle(pos + side, 0, pi/2, thick) +
	    gencircle(pos + vec2(0, side.y), pi/2, pi, thick) +
	    gencircle(pos,-pi,-pi/2, thick) +
	    gencircle(pos + vec2(side.x, 0),-pi/2, 0, thick),
	    color
	);
}
void drawLineF(vec2 begin, vec2 end, float wight, Color color) {
	vec2 d = end - begin;
	float t = atan2(d.y, d.x);
	drawPolygon(
	    gencircle(begin, t + pi/2, t + 3 * pi / 2, wight) +
	    gencircle(end, t - pi/2, t + pi/2, wight), 
	    color
	);
}
