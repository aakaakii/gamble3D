#include "core/Config.h"
#include "lib/json.hpp"
#include <iostream>
#include <fstream>

namespace gm {

Config Config::load(const std::string& path) {
	Config cfg;
	
	std::ifstream file(path);
	if(!file) {
		std::cerr << "Warning: cannot open " << path << ", using defaults.\n";
		return cfg;
	}
	
	try {
		// 使用 nlohmann/json 解析 JSON
		nlohmann::json doc;
		file >> doc;
		
		// 解析 die 范围
		cfg.dieMin = doc["die"]["min"];
		cfg.dieMax = doc["die"]["max"];
		
		// 解析中奖人数
		cfg.winCnt = doc["winCnt"];
		
		// 解析时间参数
		cfg.state1time = doc["timing"]["accelerationTime"];
		cfg.state2time = doc["timing"]["decelerationTime"];
		cfg.targetSpeed = doc["timing"]["targetSpeed"];
		
		// 解析屏幕尺寸
		cfg.screenWidth = doc["screen"]["width"];
		cfg.screenHeight = doc["screen"]["height"];
		
		// 解析 Bloom 配置（带默认值）
		if(doc.contains("effects") && doc["effects"].contains("bloom")) {
			auto& bloom = doc["effects"]["bloom"];
			cfg.bloomEnabled = bloom.value("enabled", true);
			cfg.bloomThreshold = bloom.value("threshold", 0.1f);
			cfg.bloomIntensity = bloom.value("intensity", 0.8f);
			cfg.bloomIterations = bloom.value("iterations", 6);
		}
		
		// 解析 Motion Blur 配置（带默认值）
		if(doc.contains("effects") && doc["effects"].contains("motionBlur")) {
			auto& mb = doc["effects"]["motionBlur"];
			cfg.motionBlurEnabled = mb.value("enabled", true);
			cfg.motionBlurStrength = mb.value("strength", 1.5f);
		}
		
	} catch(const std::exception& e) {
		std::cerr << "Warning: error parsing JSON from " << path << ": " << e.what() << ", using defaults.\n";
		return Config(); // 返回默认配置
	}
	
	return cfg;
}

} // namespace gm
