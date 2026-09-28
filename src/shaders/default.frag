#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 VertexColor;

uniform vec3 viewPos;
uniform vec3 lightDir;    // directional "sun" light
uniform vec3 lightColor;
uniform vec3 materialColor;
uniform float shininess;
uniform vec3 emissive;    // e.g. the doll's glowing red-light eyes
uniform float alphaValue; // for translucent overlays (blush, ground shadow, laser glow)
uniform bool unlit;       // flat color, ignoring scene lighting

void main() {
    vec3 baseColor = materialColor * VertexColor;

    if (unlit) {
        FragColor = vec4(baseColor + emissive, alphaValue);
        return;
    }

    vec3 norm = normalize(Normal);
    vec3 toLight = normalize(-lightDir);
    vec3 toCam = normalize(viewPos - FragPos);

    // 1. Dual-tone hemispheric ambient lighting (Soft sky light + warm sand bounce)
    float skyFactor = norm.y * 0.5 + 0.5;
    vec3 skyColor = vec3(0.38, 0.48, 0.65);
    vec3 groundColor = vec3(0.35, 0.28, 0.20);
    vec3 ambient = mix(groundColor, skyColor, skyFactor) * 0.32;

    // 2. Direct sunlight diffuse
    float diff = max(dot(norm, toLight), 0.0);
    vec3 diffuse = diff * lightColor * 0.52;

    // 3. Subtle soft fill light from eye direction (balanced, not overexposing)
    float fillDiff = max(dot(norm, toCam), 0.0) * 0.10;
    vec3 fillLight = fillDiff * vec3(0.85, 0.82, 0.78);

    // 4. Blinn-Phong specular highlight
    vec3 halfwayDir = normalize(toLight + toCam);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), max(shininess, 1.0));
    vec3 specular = spec * lightColor * 0.18;

    vec3 litColor = (ambient + diffuse + fillLight) * baseColor + specular + emissive;

    // Rich cinematic tone curve matching Squid Game episode 1 look
    vec3 finalColor = litColor / (litColor + vec3(0.22)) * 1.02;

    FragColor = vec4(finalColor, alphaValue);
}
