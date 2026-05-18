#include "core/Config.h"
#include <fstream>
#include <iostream>

namespace gm {

Config Config::load(const std::string& path) {
	Config cfg;
	std::ifstream sin(path);
	if(!sin) {
		std::cerr << "Warning: cannot open " << path << ", using defaults.\n";
		return cfg;
	}
	sin >> cfg.dieMin >> cfg.dieMax >> cfg.winCnt;
	sin >> cfg.state1time >> cfg.state2time >> cfg.targetSpeed;
	sin >> cfg.screenWidth >> cfg.screenHeight;
	return cfg;
}

} // namespace gm
