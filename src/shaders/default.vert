#version 330 core
// =============================================================================
// VERTEX SHADER (Phong shading, step 1 of 2)
// Only moves each vertex into place. All lighting is done later, per pixel, in
// default.frag - the vertex shader just hands over the position, normal, color
// and texture coordinate so the rasterizer can interpolate them across triangles.
// =============================================================================
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;     // world-space position
out vec3 Normal;      // world-space normal (re-normalized per pixel in the fragment shader)
out vec3 VertexColor;
out vec2 TexCoord;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    // Inverse-transpose keeps normals perpendicular to the surface under non-uniform scaling.
    Normal = mat3(transpose(inverse(model))) * aNormal;
    VertexColor = aColor;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
