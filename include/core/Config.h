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
	
	/// 从 settings.txt 读取
	static Config load(const std::string& path);
};

} // namespace gm

#endif
