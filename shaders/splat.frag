#version 420 core

in vec4 vColor;
in vec2 vPosition;

out vec4 fragColor;

// 提高饱和度函数
vec3 adjustSaturation(vec3 color, float saturation) {
    const vec3 luminanceWeights = vec3(0.2126, 0.7152, 0.0722);
    float luminance = dot(color, luminanceWeights);
    return mix(vec3(luminance), color, saturation);
}

// 白点调整（曝光和色调映射）
vec3 adjustWhitePoint(vec3 color, float whitePoint) {
    // 使用Reinhard色调映射变体
    return color * (1.0 + color / (whitePoint * whitePoint)) / (1.0 + color);
}

void main() {
    float A = -dot(vPosition, vPosition);
    
    // 更严格的裁剪以获得更精细的边缘
    if (A < -4.0) discard;
    
    // 使用更平滑的衰减曲线
    float gaussian = exp(A);
    
    // 添加边缘平滑，减少锯齿
    float edgeSmoothness = smoothstep(-4.0, -3.5, A);
    float B = gaussian * vColor.a * edgeSmoothness;
    
    vec3 color = B * vColor.rgb;
    
    // 调整饱和度 (1.0 = 原始, >1.0 = 更饱和, <1.0 = 去饱和)
    color = adjustSaturation(color, 1.2);
    
    // 调整白点 (更低的值 = 更亮的高光)
    color = adjustWhitePoint(color, 0.9);
    
    // 轻微的锐化效果，增强细节
    float sharpness = 1.05;
    color = pow(color, vec3(1.0 / sharpness));
    
    fragColor = vec4(color, B);
}
