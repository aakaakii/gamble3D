#include "lib/consts.h"
using gm::vec2;
using gm::vec3;
using gm::COLOR;

std::random_device rdc;
std::mt19937 mtrnd(rdc());
float uniform() {
	return fabs(mtrnd()*1./mtrnd.max());
}

Font font;

const vec2 SCRL = {1919, 1079};

void DrawTextPlus(vec2 pos, float size, const char* text, COLOR color) {
	vec2 meas = MeasureTextEx(font, text, size, 0);
	DrawTextEx(font, text, {pos.x - meas.x / 2, pos.y - meas.y / 2}, size, 0, color);
}

unsigned char* fontFileData;
int* codepoints;
void loadFont(std::string file, std::string chars) {
	int fileSize, codepointsCount;
	fontFileData = LoadFileData(file.data(), &fileSize);
	codepoints = LoadCodepoints(chars.c_str(), &codepointsCount);
	font = LoadFontFromMemory(".ttf", fontFileData, fileSize, 200, codepoints, codepointsCount);
}

void unloadFont() {
	UnloadFont(font);
	UnloadCodepoints(codepoints);
	UnloadFileData(fontFileData);
}
