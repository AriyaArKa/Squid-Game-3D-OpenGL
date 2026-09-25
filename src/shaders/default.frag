#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 VertexColor;

uniform vec3 viewPos;
uniform vec3 lightDir;    // directional "sun" light, pointing FROM the light
uniform vec3 lightColor;
uniform vec3 materialColor;
uniform float shininess;
uniform vec3 emissive;    // e.g. the doll's glowing red-light eyes
uniform float alphaValue; // for translucent overlays (blush, ground shadow, laser glow) - GLCompat always sets this (1.0 default)
uniform bool unlit;       // flat color, ignoring scene lighting - GLCompat always sets this (false default)

void main() {
    vec3 baseColor = materialColor * VertexColor;

    if (unlit) {
        FragColor = vec4(baseColor + emissive, alphaValue);
        return;
    }

    vec3 norm = normalize(Normal);
    vec3 toLight = normalize(-lightDir);

    // Ambient
    vec3 ambient = 0.22 * lightColor;

    // Diffuse
    float diff = max(dot(norm, toLight), 0.0);
    vec3 diffuse = diff * lightColor;

    // Blinn-Phong specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(toLight + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), max(shininess, 1.0));
    vec3 specular = spec * lightColor * 0.5;

    vec3 result = (ambient + diffuse) * baseColor + specular + emissive;
    FragColor = vec4(result, alphaValue);
}
