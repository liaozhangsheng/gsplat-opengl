#version 420 core

uniform usampler2D u_texture;
uniform mat4 projection;
uniform mat4 view;
uniform vec2 focal;
uniform vec2 viewport;

in vec2 position;
in uint index;

out vec4 vColor;
out vec2 vPosition;

void main() {
    // Fetch gaussian data from texture
    uvec4 cen = texelFetch(u_texture, ivec2((uint(index) & 0x3ffu) << 1, uint(index) >> 10), 0);
    
    // Transform position to camera space
    vec4 cam = view * vec4(uintBitsToFloat(cen.xyz), 1.0);
    vec4 pos2d = projection * cam;
    
    // Frustum culling
    float clip = 1.2 * pos2d.w;
    if (pos2d.z < -pos2d.w || pos2d.z > pos2d.w || 
        pos2d.x < -clip || pos2d.x > clip || 
        pos2d.y < -clip || pos2d.y > clip) {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }
    
    // Fetch covariance data
    uvec4 cov = texelFetch(u_texture, ivec2(((uint(index) & 0x3ffu) << 1) | 1u, uint(index) >> 10), 0);
    
    // Unpack half-precision covariance
    vec2 u1 = unpackHalf2x16(cov.x);
    vec2 u2 = unpackHalf2x16(cov.y);
    vec2 u3 = unpackHalf2x16(cov.z);
    mat3 Vrk = mat3(
        u1.x, u1.y, u2.x,
        u1.y, u2.y, u3.x,
        u2.x, u3.x, u3.y
    );
    
    // Compute 2D covariance
    mat3 J = mat3(
        focal.x / cam.z, 0.0, -(focal.x * cam.x) / (cam.z * cam.z),
        0.0, focal.y / cam.z, -(focal.y * cam.y) / (cam.z * cam.z),
        0.0, 0.0, 0.0
    );
    
    mat3 T = transpose(mat3(view)) * J;
    mat3 cov2d = transpose(T) * Vrk * T;
    
    // Add low-pass filter (减小值以获得更精细的splat)
    cov2d[0][0] += 0.1;
    cov2d[1][1] += 0.1;
    
    // Compute eigenvalues for ellipse
    float mid = (cov2d[0][0] + cov2d[1][1]) / 2.0;
    float radius = length(vec2((cov2d[0][0] - cov2d[1][1]) / 2.0, cov2d[0][1]));
    float lambda1 = mid + radius;
    float lambda2 = mid - radius;
    
    if (lambda2 < 0.0) {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }
    
    vec2 diagonalVector = normalize(vec2(cov2d[0][1], lambda1 - cov2d[0][0]));
    // 减小scale以获得更精细的splat覆盖
    float scale = 2.5;
    vec2 majorAxis = scale * min(sqrt(2.0 * lambda1), 1024.0) * diagonalVector;
    vec2 minorAxis = scale * min(sqrt(2.0 * lambda2), 1024.0) * vec2(diagonalVector.y, -diagonalVector.x);
    
    // Unpack color
    vec4 color = vec4(
        float((cov.w) & 0xffu),
        float((cov.w >> 8) & 0xffu),
        float((cov.w >> 16) & 0xffu),
        float((cov.w >> 24) & 0xffu)
    ) / 255.0;
    
    vColor = color;
    vPosition = position;
    
    // Compute final position
    vec2 vCenter = vec2(pos2d) / pos2d.w;
    gl_Position = vec4(
        vCenter + 
        position.x * majorAxis / viewport + 
        position.y * minorAxis / viewport,
        pos2d.z / pos2d.w, 1.0
    );
}
