#version 330 core
// =============================================================================
// FRAGMENT SHADER (Phong shading, step 2 of 2)
// The lighting equation is evaluated for EVERY PIXEL using the interpolated,
// re-normalized normal:
//     color = ambient + sum over lights( diffuse + specular ) + emission
// with Blinn-Phong specular highlights. Three light types are supported:
//   (a) one directional light, (b) up to 5 point lights, (c) one spot light.
// =============================================================================
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 VertexColor;
in vec2 TexCoord;

uniform vec3 viewPos;

// Material
uniform vec3 materialColor;
uniform float shininess;
uniform float materialSpecularStrength;
uniform vec3 emissive;    // glowing parts: neon rails, doll's red eyes, lamp faces
uniform float alphaValue; // translucent parts: glass panes, blush, light halos
uniform bool unlit;       // flat color, no lighting (glows and halos)

// Texture mapping
uniform bool useTexture;
uniform sampler2D diffuseTexture;
uniform vec2 texRepeat;   // how many times the image repeats across the surface

// Ambient (hemisphere: sky color from above, ground bounce from below)
uniform vec3 ambientSky;
uniform vec3 ambientGround;

// (a) Directional light
uniform bool dirLightOn;
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;
uniform float dirAmbientIntensity;
uniform float dirDiffuseIntensity;
uniform float dirSpecularIntensity;

// (b) Point lights (slot 0 = main point light, others = fill lamps)
#define MAX_POINT_LIGHTS 5
struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
    float intensity;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

// (c) Spot light
uniform bool spotLightOn;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;
uniform float spotLightIntensity;

// Distance haze
uniform bool fogOn;
uniform vec3 fogColor;
uniform float fogDensity;

// Soft filmic curve: keeps bright neon and highlights from clipping to flat white.
vec3 toneMap(vec3 c) {
    c = max(c, vec3(0.0));
    vec3 mapped = (c * (2.51 * c + 0.03)) / (c * (2.43 * c + 0.59) + 0.14);
    mapped = clamp(mapped, 0.0, 1.0);
    float luminance = dot(mapped, vec3(0.2126, 0.7152, 0.0722));
    return clamp(mix(vec3(luminance), mapped, 1.10), 0.0, 1.0);
}

vec3 applyFog(vec3 color) {
    if (!fogOn) return color;
    float d = length(viewPos - FragPos) * fogDensity;
    float visibility = exp(-d * d);
    return mix(fogColor, color, visibility);
}

// Blinn-Phong diffuse + specular contribution of one light arriving from direction L.
void addLight(vec3 N, vec3 V, vec3 L, vec3 lightColor, float diffuseScale, float specScale,
              float specExp, inout vec3 diffuse, inout vec3 specular) {
    float diff = max(dot(N, L), 0.0);                     // Lambert: cos(angle to light)
    diffuse += diff * lightColor * diffuseScale;

    vec3 H = normalize(L + V);                            // halfway vector
    float spec = pow(max(dot(N, H), 0.0), specExp);       // shininess controls highlight size
    specular += spec * lightColor * specScale * materialSpecularStrength;
}

void main() {
    vec3 baseColor = materialColor * VertexColor;
    if (useTexture) {
        baseColor *= texture(diffuseTexture, TexCoord * texRepeat).rgb;
    }
    baseColor = clamp(baseColor, 0.0, 1.0);

    if (unlit) {
        FragColor = vec4(applyFog(toneMap(baseColor + emissive)), alphaValue);
        return;
    }

    vec3 N = normalize(Normal);                 // per-pixel normal (the heart of Phong shading)
    vec3 V = normalize(viewPos - FragPos);      // direction to the camera
    float specExp = max(shininess, 32.0);

    // Ambient: blend ground and sky colors by how much the surface faces up.
    float upFactor = N.y * 0.5 + 0.5;
    vec3 diffuse = mix(ambientGround, ambientSky, upFactor) * dirAmbientIntensity;
    diffuse += max(dot(N, V), 0.0) * 0.10 * vec3(0.88, 0.85, 0.82); // soft camera fill
    vec3 specular = vec3(0.0);
    vec3 rim = vec3(0.0);

    // (a) Directional light: same direction everywhere, no distance falloff.
    if (dirLightOn && length(dirLightDir) > 0.001) {
        vec3 L = normalize(-dirLightDir);
        addLight(N, V, L, dirLightColor, dirDiffuseIntensity, dirSpecularIntensity, specExp, diffuse, specular);

        // Subtle rim light on silhouettes
        float rimFactor = 1.0 - max(dot(V, N), 0.0);
        rim += pow(rimFactor, 3.5) * max(dot(N, L) * 0.5 + 0.5, 0.0) * dirLightColor * 0.20;
    }

    // (b) Point lights: shine in all directions, get weaker with distance.
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; ++i) {
        vec3 toLight = pointLights[i].position - FragPos;
        float dist = length(toLight);
        if (dist < 0.0001) continue;
        float atten = pointLights[i].intensity /
                      (pointLights[i].constant + pointLights[i].linear * dist +
                       pointLights[i].quadratic * dist * dist);
        addLight(N, V, toLight / dist, pointLights[i].color, atten, atten, specExp, diffuse, specular);
    }

    // (c) Spot light: a point light limited to a cone, with a soft edge.
    if (spotLightOn) {
        vec3 toLight = spotLightPos - FragPos;
        float dist = length(toLight);
        if (dist > 0.0001) {
            vec3 L = toLight / dist;
            float theta = dot(L, normalize(-spotLightDir));            // cos(angle from cone axis)
            float epsilon = max(spotLightCutOff - spotLightOuterCutOff, 0.0001);
            float cone = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);
            if (cone > 0.0) {
                float atten = spotLightIntensity / (1.0 + 0.045 * dist + 0.0075 * dist * dist);
                addLight(N, V, L, spotLightColor, cone * atten, cone * atten * 1.1, specExp, diffuse, specular);
            }
        }
    }

    vec3 litColor = diffuse * baseColor + specular * vec3(1.0, 0.98, 0.94) + rim + emissive;
    FragColor = vec4(applyFog(toneMap(litColor)), alphaValue);
}
