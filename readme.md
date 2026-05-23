# 神秘3d抽奖程序

## data.txt

data.txt 中包含所有参与抽奖成员的名称。用空白字符（换行/空格）分隔。这意味着名字中不能含有空格。

不需要显式指定数量，参考给出的格式。

## settings.json

包含所有可配置项，使用 JSON 格式。默认配置视觉效果较好，可根据需要自行修改。

### 基础配置

| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `die.min` | 整数 | 每次出局人数下限，建议 $1$ |
| `die.max` | 整数 | 每次出局人数上限，建议 $\le 5$ |
| `winCnt` | 整数 | 中奖人数（最终剩下的人数） |
| `timing.accelerationTime` | 浮点数 | 加速阶段时长（秒） |
| `timing.decelerationTime` | 浮点数 | 减速阶段时长（秒） |
| `timing.targetSpeed` | 浮点数 | 目标速度，运行时用 $t_1$ 秒加速至此值，再在 $t_2$ 秒内减速到 $0$ |
| `screen.width` | 整数 | 窗口宽度（像素） |
| `screen.height` | 整数 | 窗口高度（像素） |

### Bloom 效果配置

| 字段 | 类型 | 默认值 | 说明 |
| :--- | :--- | :--- | :--- |
| `bloom.enabled` | 布尔值 | `true` | 是否启用 Bloom 泛光效果 |
| `bloom.threshold` | 浮点数 | `0.0` | 亮度阈值，高于此值的像素产生泛光 |
| `bloom.intensity` | 浮点数 | `0.8` | 泛光强度 |
| `bloom.blurIterations` | 整数 | `8` | 模糊迭代次数，越大效果越柔和（性能消耗也越大） |

示例配置：

```json
{
  "die": { "min": 1, "max": 1 },
  "winCnt": 5,
  "timing": {
    "accelerationTime": 4.0,
    "decelerationTime": 4.0,
    "targetSpeed": 0.5
  },
  "screen": { "width": 1920, "height": 1080 },
  "bloom": {
    "enabled": true,
    "threshold": 0.0,
    "intensity": 0.8,
    "blurIterations": 8
  }
}
```
