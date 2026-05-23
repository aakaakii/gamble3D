#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#include <string>

namespace gm {

struct Config {
	int dieMin  = 1;
	int dieMax  = 4;
	int winCnt  = 2;
	
	double state1time  = 2.0;
	double state2time  = 2.0;
	double targetSpeed = 0.1;
	
	int screenWidth  = 1920;
	int screenHeight = 1080;
	
	// Bloom 效果参数
	bool bloomEnabled    = true;
	float bloomThreshold = 0.0f;
	float bloomIntensity = 0.8f;
	int bloomBlurIterations = 8;
	
	/// 从 settings.json 读取 (使用 nlohmann/json 解析)
	static Config load(const std::string& path);
};

} // namespace gm

#endif
