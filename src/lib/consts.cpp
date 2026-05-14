#include "lib/consts.h"

random_device rdc;
mt19937 mtrnd(rdc());
float uniform() {
	return fabs(mtrnd()*1./mtrnd.max());
}

Font font;

const vec2 SCRL = {1919, 1079};

int tostr(float dmg, char* str) {
	if(dmg >= 1e9) return sprintf(str, "%.2fB", dmg / 1e9);
	if(dmg >= 1e6) return sprintf(str, "%.2fM", dmg / 1e6);
	if(dmg >= 1e3) return sprintf(str, "%.2fK", dmg / 1e3);
	return sprintf(str, "%d", (int)dmg);
}

void DrawTextPlus(vec2 pos, float size, const char* text, COLOR color) {
	vec2 meas = MeasureTextEx(font, text, size, 0);
	DrawTextEx(font, text, {pos.x - meas.x / 2, pos.y - meas.y / 2}, size, 0, color);
}

unsigned char* fontFileData;
int* codepoints;
void loadFont(string file, string chars) {
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
