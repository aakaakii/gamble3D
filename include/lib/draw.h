#ifndef DRAW_H
#define DRAW_H
#include <raylib.h>
#include <vector>
#include <math.h>
#include "lib/types.h"
using namespace std;

template<class T> vector<T> operator + (vector<T> a, const vector<T>& b) {
	a.insert(a.end(), b.begin(), b.end()); return a;
}

void initScreen(int fps, vec2 SCRL, const char* title);
void drawCircle(vec2 pos, Color color, float rad);
void drawLine(vec2 begin, vec2 end, Color color);
void drawRect(vec2 pos, vec2 siz, COLOR color);
void drawCircleRing(vec2 pos, Color color, float rad, float p = .1);
void drawTriangle(vec2 a, vec2 b, vec2 c, Color color);
void drawPolygon(const vector<vec2>& dots, Color color);
void drawRoundRect(vec2 pos, vec2 side, float thick, Color color);
void drawLineF(vec2 begin, vec2 end, float wight, Color color);
float mix(float a, float b, float p);
COLOR mix(COLOR a, COLOR b, float p);
const float pi = acos(-1);
vector<vec2> gencircle(vec2 o, float begin, float end, float rad, float p = .2);
#endif
