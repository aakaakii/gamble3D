#ifndef CONSTS_H
#define CONSTS_H
#include "lib/types.h"
#include "lib/draw.h"
#include <random>
#include <string>

extern std::random_device rdc;
extern std::mt19937 mtrnd;
float uniform();

extern Font font;
extern const vec2 SCRL;

void DrawTextPlus(vec2 pos, float size, const char* text, COLOR color=WHITE);

void loadFont(std::string file, std::string chars);
void unloadFont();
#endif
