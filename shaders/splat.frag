#version 420 core

in vec4 vColor;
in vec2 vPosition;

out vec4 fragColor;

// Function to increase saturation
vec3 adjustSaturation(vec3 color, float saturation) {
    const vec3 luminanceWeights = vec3(0.2126, 0.7152, 0.0722);
    float luminance = dot(color, luminanceWeights);
    return mix(vec3(luminance), color, saturation);
}

// White point adjustment (exposure and tone mapping)
vec3 adjustWhitePoint(vec3 color, float whitePoint) {
    // Use Reinhard tone mapping variant
    return color * (1.0 + color / (whitePoint * whitePoint)) / (1.0 + color);
}

void main() {
    float A = -dot(vPosition, vPosition);
    
    // Stricter clipping for finer edges
    if (A < -4.0) discard;
    
    // Use smoother attenuation curve
    float gaussian = exp(A);
    
    // Add edge smoothing to reduce aliasing
    float edgeSmoothness = smoothstep(-4.0, -3.5, A);
    float B = gaussian * vColor.a * edgeSmoothness;
    
    vec3 color = B * vColor.rgb;
    
    // Adjust saturation (1.0 = original, >1.0 = more saturated, <1.0 = desaturated)
    color = adjustSaturation(color, 1.2);
    
    // Adjust white point (lower value = brighter highlights)
    color = adjustWhitePoint(color, 0.9);
    
    // Slight sharpening effect to enhance details
    float sharpness = 1.05;
    color = pow(color, vec3(1.0 / sharpness));
    
    fragColor = vec4(color, B);
}
