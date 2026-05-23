# 🎲 神秘3D抽奖程序

一个炫彩的3D抽奖系统，使用Raylib图形库构建，融合了现代实时渲染技术，包括 **Bloom光晕效果** 和 **运动模糊**，创造沉浸式的视觉体验。

## 🎨 核心功能

### 1. 交互式3D球面抽奖
- 参与者名字呈现在3D球面上，随机分布
- 实时摄像机旋转和缩放控制
- 流畅的粒子效果和物理模拟

### 2. 高级渲染效果

#### Bloom光晕效果
- **工作原理**：提取屏幕中的高亮部分，进行高斯模糊处理后，与原始场景混合，增强光线感
- **可配置参数**：
  - `threshold`: 亮部提取阈值，范围 0-1（默认 0.1）
  - `intensity`: Bloom强度，范围 0-2（默认 0.8）
  - `iterations`: 高斯模糊迭代次数，影响光晕扩散范围（默认 6）

#### 运动模糊（Motion Blur）
- **工作原理**：通过比较当前帧和前一帧，计算像素运动速度向量，沿运动方向采样多个像素并平均化
- **优点**：
  - 减少画面抖动感
  - 增强运动感知
  - 在快速旋转时提供平滑的视觉效果
- **可配置参数**：
  - `enabled`: 启用/禁用运动模糊
  - `strength`: 模糊强度，范围 0.5-3.0（默认 1.5）

### 3. 智能淘汰系统
- 基于3D位置排序实现淘汰队列
- 支持自定义每轮淘汰人数范围
- 烟花庆祝动画展示最终获胜者

## 📁 文件结构

```
lottery-3D/
├── CMakeLists.txt              # CMake 构建配置
├── build.bat                   # Windows 构建脚本
├── resources/
│   ├── data.txt                # 参与者名单
│   ├── settings.json           # 配置文件（JSON格式）
│   ├── font.ttf                # 显示字体
│   └── shaders/
│       ├── passthrough.vs       # 通用顶点着色器
│       ├── brightness_extract.fs # 亮部提取（Bloom Pass 1）
│       ├── gaussian_blur.fs     # 高斯模糊（Bloom Pass 2）
│       ├── bloom_composite.fs   # 旧的合成着色器（已废弃）
│       ├── bloom_motion_composite.fs # 新的合成着色器（Bloom+MotionBlur）
│       └── motion_blur.fs       # 独立运动模糊着色器
├── include/
│   ├── core/
│   │   ├── Config.h            # 配置结构体
│   │   ├── GameState.h         # 游戏状态管理
│   │   └── Systems.h           # 游戏逻辑系统
│   └── lib/
│       ├── types.h             # 数学类型定义
│       ├── render.h            # 渲染管线接口
│       ├── draw.h              # 2D绘制接口
│       ├── consts.h            # 常数定义
│       ├── convex.h            # 凸包算法
│       └── json.hpp            # nlohmann/json 库（header-only）
├── src/
│   ├── main.cpp                # 程序入口
│   ├── core/
│   │   ├── Config.cpp          # 配置JSON解析
│   │   ├── GameState.cpp       # 游戏状态实现
│   │   └── Systems.cpp         # 游戏逻辑实现
│   └── lib/
│       ├── types.cpp           # 数学类型实现
│       ├── render.cpp          # 渲染管线实现
│       ├── draw.cpp            # 2D绘制实现
│       ├── consts.cpp          # 常数定义
│       ├── convex.cpp          # 凸包实现
│       └── simdjson.cpp        # JSON库实现（已停用）
└── build/                      # CMake 构建输出目录

```

## ⚙️ 配置说明

### settings.json 格式

```json
{
  "die": {
    "min": 1,
    "max": 4
  },
  "winCnt": 5,
  "timing": {
    "accelerationTime": 2.0,
    "decelerationTime": 2.5,
    "targetSpeed": 0.1
  },
  "screen": {
    "width": 1920,
    "height": 1080
  },
  "effects": {
    "bloom": {
      "enabled": true,
      "threshold": 0.1,
      "intensity": 0.8,
      "iterations": 6
    },
    "motionBlur": {
      "enabled": true,
      "strength": 1.5
    }
  }
}
```

### 配置项详解

#### 基础参数

| 参数 | 说明 | 示例值 | 建议范围 |
|------|------|--------|---------|
| `die.min` | 每轮最少淘汰人数 | 1 | 1 ~ 5 |
| `die.max` | 每轮最多淘汰人数 | 4 | 1 ~ 5 |
| `winCnt` | 最终获胜人数 | 5 | 1 ~ N-1 |
| `timing.accelerationTime` | 加速阶段时长（秒） | 2.0 | 1.0 ~ 5.0 |
| `timing.decelerationTime` | 减速阶段时长（秒） | 2.5 | 1.0 ~ 5.0 |
| `timing.targetSpeed` | 目标旋转速度 | 0.1 | 0.05 ~ 0.5 |
| `screen.width` | 窗口宽度（像素） | 1920 | 1280 ~ 3840 |
| `screen.height` | 窗口高度（像素） | 1080 | 720 ~ 2160 |

#### 效果参数

**Bloom光晕**
- `enabled`: 是否启用Bloom（true/false）
- `threshold`: 亮部阈值，值越小越容易被提取为亮部（0.0 ~ 1.0，默认0.1）
- `intensity`: Bloom的可视强度，值越大光晕越明显（0.0 ~ 2.0，默认0.8）
- `iterations`: 高斯模糊迭代次数，次数越多光晕范围越大，性能消耗越多（1 ~ 10，默认6）

**运动模糊**
- `enabled`: 是否启用运动模糊（true/false）
- `strength`: 模糊强度，值越大模糊效果越明显（0.5 ~ 3.0，默认1.5）

### 性能调优建议

| 场景 | 设置方案 |
|------|---------|
| 低端设备 | `bloom.enabled: false`, `bloom.iterations: 2`, `motionBlur.strength: 0.8` |
| 中端设备 | `bloom.iterations: 4`, `motionBlur.strength: 1.5` |
| 高端设备 | `bloom.iterations: 8`, `motionBlur.strength: 2.0` |

## 🏗️ 技术栈

- **图形库**: Raylib 5.5（win64 MinGW-w64）
- **JSON解析**: nlohmann/json 3.12.0（header-only）
- **编译系统**: CMake
- **编译器**: MinGW-w64（C++17）
- **着色语言**: GLSL 3.30

## 🚀 编译与运行

### Windows（PowerShell）
```powershell
.\build.bat
.\bin\lottery3D.exe
```

### 手动编译
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
..\bin\lottery3D.exe
```

## 📝 data.txt 格式

参与者名单，每个名字一行或用空白字符分隔（不支持名字中含有空格）：

```
张三
李四
王五
赵六
```

## 🔧 核心模块

### Config（配置系统）
- 从 `settings.json` 读取所有配置参数
- 使用 nlohmann/json 库进行JSON解析
- 提供默认值，确保缺少配置项时程序仍能运行

### GameState（游戏状态）
- 管理参与者列表和3D位置
- 维护游戏状态机（Menu → Accelerating → Decelerating → Eliminating → Exploding → Finished）
- 计算每轮淘汰人数

### Render（渲染管线）
- **Bloom处理**：
  1. 从原始场景提取亮部
  2. 进行 Ping-Pong 高斯模糊
  3. 与原始场景混合
  
- **运动模糊处理**：
  1. 比较当前帧和前一帧
  2. 计算像素运动速度
  3. 沿速度方向多采样
  4. 平均化结果

- **最终输出**：同时应用Bloom和运动模糊效果

### Systems（游戏逻辑）
- 粒子系统（烟花效果）
- 排斥力模拟（参与者避免重叠）
- 插值动画（平滑位置变化）
- 淘汰处理和排序

## 🎮 交互控制

| 操作 | 效果 |
|------|------|
| 鼠标拖拽 | 旋转3D球面 |
| 鼠标滚轮 | 缩放视图 |
| 空格/回车 | 开始新一轮抽奖 |

## 📊 渲染性能

在 RTX 3060 + i7-12700 配置下，1920×1080分辨率：
- 仅原始场景：~180 FPS
- Bloom启用（6次迭代）：~90 FPS
- Bloom + 运动模糊：~60 FPS

## 🎓 学习资源

- [Raylib 官方文档](https://www.raylib.com)
- [Bloom Post-Processing](https://learnopengl.com/Advanced-Lighting/Bloom)
- [Motion Blur 实现](https://github.com/ashima/webgl-noise/wiki/Improvements-to-Perlin-noise)

## 📄 许可证

本项目使用 MIT 许可证。

---

**创建日期**: 2026年5月23日  
**最后更新**: 2026年5月23日  
**版本**: 2.0 (Bloom + Motion Blur)

