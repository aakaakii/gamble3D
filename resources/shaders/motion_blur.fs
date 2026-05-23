#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

uniform sampler2D texture0;      // 当前帧
uniform sampler2D texture1;      // 前一帧
uniform float motionBlurStrength;
uniform vec2 screenResolution;

// 简化的伪随机数生成
float rand(vec2 coord) {
    return fract(sin(dot(coord, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec2 texCoord = fragTexCoord;
    vec3 currentColor = texture(texture0, texCoord).rgb;
    vec3 previousColor = texture(texture1, texCoord).rgb;
    
    // 计算速度向量（简化方法：基于颜色差异）
    vec3 colorDiff = currentColor - previousColor;
    float velocity = length(colorDiff) * 0.5;
    
    // 添加轻微噪声以减少假影
    float noise = rand(texCoord) * 0.3;
    velocity = clamp(velocity + noise, 0.0, 3.0);
    
    // 运动模糊采样
    vec3 blurColor = currentColor;
    const int samples = 8;
    
    for(int i = 1; i < samples; i++) {
        float t = float(i) / float(samples);
        vec2 offset = normalize(colorDiff.xy + 0.01) * velocity * t * motionBlurStrength;
        vec2 sampleCoord = texCoord + offset / screenResolution;
        
        if(sampleCoord.x >= 0.0 && sampleCoord.x <= 1.0 && sampleCoord.y >= 0.0 && sampleCoord.y <= 1.0) {
            blurColor += texture(texture0, sampleCoord).rgb;
        }
    }
    
    blurColor /= float(samples);
    finalColor = vec4(blurColor, 1.0) * fragColor;
}
