#include <iostream>
#include <raylib.h>
#include <math.h>
#include <raymath.h>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <lib/consts.h>
#include <lib/draw.h>
#include <lib/convex.h>
#include <lib/render.h>
int N = 80;
const vec3 G = vec3(0, -.3, 0);

inline vec3 uniformvec3() {
	return vec3(uniform() - .5, uniform() - .5, uniform() - .5).normal();
}

struct Node {
	vec3 pos, vel;
	COLOR color;
	Node(vec3 pos, COLOR color): pos(pos), color(color) {
		vel = uniformvec3() * 2 + (vec3){0, uniform() * 2, 0};
	}
	void update() {
		pos += vel, vel += G;
	}
};

struct NameTag {
	string str;
	vec2 pos, to;
};

struct Person {
	string str;
	vec3 pos, to;
	float rad, torad;
};

const float radius = 200;
vector<Person> pers;
vector<Node> nodes;
vector<vec3> stars;
vector<string> names;
vector<NameTag> tag;
string ALLCHARS, TMP;
int dieMax = 0, dieMin = 0, winCnt = 0;
double state1time, state2time, targetSpeed;
int screenWidth, screenHeight;
void init() {
	ifstream fin("resources/data.txt");
	ifstream sin("resources/settings.txt");
	sin >> dieMin >> dieMax >> winCnt;
	sin >> state1time >> state2time >> targetSpeed;
	sin >> screenWidth >> screenHeight;
	while(fin >> TMP) names.push_back(TMP);
	N = names.size();
	pers.resize(N), tag.resize(N);
	ALLCHARS += "剩余人数: 01234567890PLAY";
	for(int i = 0; i < N; ++i) {
		pers[i].to = uniformvec3() * radius;
		tag[i].str = pers[i].str = names[i];
		ALLCHARS += pers[i].str;
	}
	for(int i = 0; i < 500; ++i) {
		stars.push_back(uniformvec3() * 3 * radius);
	}
}

Matrix transform(vec3 axis, float theta) {
	Matrix M = MatrixRotate((Vector3)axis, theta);
	for(auto& p: pers) p.to = Vector3Transform((Vector3)p.to, M);
	return M;
}

int main() {
	init();
	srand(time(0));
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(screenWidth, screenHeight, "gamble3D");
	loadFont("resources/font.ttf", ALLCHARS);
	
	Camera3D camera;
	camera.position = {325, 0, 0};
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 100;
	camera.projection = CAMERA_PERSPECTIVE;
	
	SetTargetFPS(60);
	
	vec3 axis = {0, 1, 0};
	
	int state = -1;
	int isdown = 0;
	float rotationSpeed = .004;
	int dieN = min(rand() % (dieMax - dieMin + 1) + dieMin, N - winCnt);
	int flagt = 0, flagp = -1;
	
	while (!WindowShouldClose()) {
		double dt = GetFrameTime();
		
		for(int i = 0; i < (int)nodes.size(); ++i) {
			nodes[i].update();
			if(nodes[i].pos.y < -1000) {
				nodes.erase(nodes.begin() + i);
				--i;
				if(state == 3) state = 0;
			}
		}
		
		if(state != 4) {
			for(int i = 0; i < N; ++i) {
				for(int j = 0; j < N; ++j) {
					if(i != j) {
						vec3 d = (pers[i].to - pers[j].to);
						pers[i].to += d.normal() * 50000 * pow(d.leng(), -2);
					}
				}
				pers[i].to = pers[i].to.normal() * radius;
			}
		}
		
		for(int i = 0; i < N; ++i) {
			pers[i].pos = mix(pers[i].pos, pers[i].to, .1);
			pers[i].rad = mix(pers[i].rad, pers[i].torad, .1);
		}
		
		if(state == -1) {
			if(IsMouseButtonPressed(0)) {
				vec2 p = GetMousePosition();
				if(((screenWidth - 600) / 2 <= p.x) and (p.x <= (screenWidth + 600) / 2) and
					((screenHeight - 250) / 2 <= p.y) and (p.y <= (screenHeight + 250) / 2)) {
					isdown = 1;
				}
			}
			if(IsMouseButtonReleased(0)) {
				if(isdown) {
					state = 0;
				}
			}
		}
		
		if(state == 0) {
			float addVel = targetSpeed / (state1time / dt);
			if(rotationSpeed < targetSpeed)
				rotationSpeed += addVel;
			else state = 1;
		}
		
		if(IsKeyPressed(KEY_SPACE)) state = 1, rotationSpeed = 0;
		
		if(state == 1) {
			float decVel = exp((log(0.001)-log(targetSpeed)) / (state2time / dt));
			if(rotationSpeed > 0.001) rotationSpeed *= decVel;
			else state = 2;
		}
		
		auto bomb = [&](int p){
			for(int t = 32; t--; )
				nodes.push_back(Node(pers[p].pos, mix(LIGHTGRAY, BEIGE, uniform())));
		};
		
		vector<int> per;
		for(int i = 0; i < N; ++i) per.push_back(i);
		sort(per.begin(), per.end(), [&](const auto& x, const auto& y){
			return (pers[x].pos - camera.position).leng() < (pers[y].pos - camera.position).leng();
		});
		
		for(int i = 0; i < N; ++i) {
			pers[i].torad = 150000 / pow((pers[i].pos - camera.position).leng(), 1.5);
		}
		
		for(int i = 0; i < dieN; ++i) {
			pers[per[i]].torad += 120;
		}
		
		if(state == 2) {
			vector<int> del;
			for(int i = 0; i < dieN; ++i)
				del.push_back(per[i]);
			sort(del.begin(), del.end());
			for(int i = del.size() - 1; i >= 0; --i) {
				bomb(del[i]);
				pers.erase(pers.begin() + del[i]), --N;
				tag.erase(tag.begin() + del[i]);
			}
			state = 3;
			dieN = min(rand() % (dieMax - dieMin + 1) + dieMin, N - winCnt);
			if(N == winCnt) state = 4;
		}
		
		if(state == 4) {
			static const int L = -200, R = 200;
			for(int i = 0; i < winCnt; ++i) {
				pers[i].to = {0, L + 1.f * (R - L) * (i+1) / (winCnt + 1), 0};
				pers[i].torad = 175;
			}
			camera.position = (Vector3)mix(camera.position, {0, 0, -200}, .1);
			camera.up = {0, 1, 0};
			if(!flagt--) {
				for(int t = 64; t--; ) nodes.push_back(Node({-200, flagp * 50.f, 0}, mix(RED, GOLD, uniform())));
				for(int t = 64; t--; ) nodes.push_back(Node({ 200, flagp * 50.f, 0}, mix(RED, GOLD, uniform())));
				flagt = 20, flagp = ++flagp == 5 ? -4 : flagp;
			}
		}
		
		for(int i = 0; i < N; ++i) { int j = i + 1;
			tag[i].to = {j / 30 * 150 + 75.f, j % 30 * 30 + 25.f};
			tag[i].pos = mix(tag[i].pos, tag[i].to, .2);
		}
		
		BeginBloomRender();
		
		BeginMode3D(camera);
		
		if(state != 4) {
			axis = Vector3Transform((Vector3)axis, MatrixRotate(camera.position, uniform() * .02));
			camera.position = Vector3Transform(camera.position, MatrixRotate((Vector3)axis, rotationSpeed));
			camera.up = Vector3Transform(camera.up, MatrixRotate((Vector3)axis, rotationSpeed));
		}
		
		if(state != 4) {
			// Color color = mix(ColorFromHSV(GetTime()*30, 1, 1), WHITE, .5);
			Color color = ColorFromHSV(GetTime()*30, 1, 1);
			vector<vec3> poss;
			for(int i = 0; i < N; ++i) poss.push_back(pers[i].pos);
			auto res = convex3d(poss);
			for(auto& [a, b]: res) {
				DrawCylinderEx((Vector3)a, (Vector3)b, 1, 1, 16, color);
				// DrawLine3D((Vector3)a, (Vector3)b, color);
			}
		}
		
		for(int i = 0; i < (int)nodes.size(); ++i) {
			DrawSphere((Vector3)nodes[i].pos, 1, nodes[i].color);
		}
		
		for(auto& V: stars) DrawSphere((Vector3)V, 1, WHITE);
		
		EndMode3D();
		
		for(int i = 0; i < N; ++i) {
			Color color = ColorFromHSV(GetTime() * 30 + i * 37 % 360, .2, .6);
			DrawTextPlus(GetWorldToScreen((Vector3)pers[i].pos, camera), pers[i].rad, pers[i].str.data(), color);
		}
		
		if(state == -1) {
			COLOR color = isdown ? COLOR(255, 255, 255, 255 * .8) : COLOR(192, 192, 192, 192 * .8);
			DrawRectangle((screenWidth - 600) / 2, (screenHeight - 250) / 2, 600, 250, color);
			DrawTextPlus({screenWidth / 2, screenHeight / 2}, 64, "PLAY\n", WHITE);
		}
		
		static char str[32];
		sprintf(str, "剩余人数: %d", N);
		DrawTextPlus({75, 25}, 25, str);
		
		DrawFPS(screenWidth - 100, screenHeight - 50);
		
		for(int i = 0; i < N; ++i) {
			DrawTextPlus(tag[i].pos, 25, tag[i].str.data());
		}
		
		EndBloomRender();
	}
	
	unloadFont();
	CloseWindow();
}
