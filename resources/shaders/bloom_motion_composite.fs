#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;      // 原始场景颜色
uniform sampler2D texture1;      // 模糊后的亮部（bloom）
uniform sampler2D textureDepth;  // 深度纹理（可采样）

uniform float bloomIntensity;
uniform float bloomEnabled;

uniform float motionBlurStrength;
uniform float motionBlurEnabled;

uniform mat4 currViewProjInv;    // 当前帧 viewProj 的逆
uniform mat4 prevViewProj;       // 前一帧 viewProj

// ── 重投影：为每个像素计算运动向量 ──
// 原理：用当前帧深度反算世界坐标，再用前一帧矩阵投影回NDC，
//      差值即为该像素在两帧之间的真实运动速度
vec2 calcVelocity(vec2 texCoord) {
    float depth = texture(textureDepth, texCoord).r;

    // 跳过无效深度（深度为0或1表示没有有效几何体）
    if(depth < 0.001 || depth > 0.999) return vec2(0.0);

    // 1) 当前 NDC 坐标
    vec4 currNdc = vec4(texCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);

    // 2) NDC → 世界空间（通过当前 viewProj 的逆矩阵）
    vec4 worldPos = currViewProjInv * currNdc;
    worldPos /= worldPos.w;

    // 3) 世界空间 → 前一帧 NDC（通过前一帧 viewProj 矩阵）
    vec4 prevNdc = prevViewProj * worldPos;
    prevNdc /= prevNdc.w;

    // 4) NDC 差值 → 纹理空间速度向量
    vec2 vel = (currNdc.xy - prevNdc.xy) * 0.5;

    // 限制速度范围，防止边缘伪影
    float maxVel = 0.05;
    vel = clamp(vel, -maxVel, maxVel);

    return vel;
}

// ── 基于重投影的运动模糊 ──
vec3 applyMotionBlur(vec3 sceneColor) {
    if(motionBlurEnabled < 0.5) return sceneColor;

    vec2 texCoord = fragTexCoord;
    vec2 vel = calcVelocity(texCoord);

    // 没有运动则跳过
    float len = length(vel);
    if(len < 0.0001) return sceneColor;

    vec3 blurColor = vec3(0.0);
    const int samples = 10;        // 参考原 MotionBlur.glsl

    // 沿速度方向采样（参考原版实现）
    int halfSamples = (samples - 1) / 2;
    for(int i = -halfSamples; i <= halfSamples; ++i) {
        vec2 coord = texCoord + vel * float(i) * motionBlurStrength;

        if(coord.x >= 0.0 && coord.x <= 1.0 &&
           coord.y >= 0.0 && coord.y <= 1.0) {
            blurColor += texture(texture0, coord).rgb;
        } else {
            blurColor += sceneColor;
        }
    }

    return blurColor / float(samples);
}

// ACES 色调映射
vec3 acesToneMapping(vec3 color) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return (color * (a * color + b)) / (color * (c * color + d) + e);
}

void main()
{
    vec3 sceneColor = texture(texture0, fragTexCoord).rgb;

    // 应用重投影运动模糊
    sceneColor = applyMotionBlur(sceneColor);

    // 应用 Bloom
    if(bloomEnabled > 0.5) {
        vec3 bloomColor = texture(texture1, fragTexCoord).rgb;
        sceneColor = sceneColor + bloomColor * bloomIntensity;
    }

    // ACES 色调映射
    sceneColor = acesToneMapping(sceneColor);

    // Gamma 校正
    sceneColor = pow(sceneColor, vec3(1.0 / 2.2));

    finalColor = vec4(sceneColor, 1.0) * fragColor;
}
