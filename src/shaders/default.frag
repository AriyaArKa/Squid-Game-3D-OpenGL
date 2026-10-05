#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 VertexColor;
in vec3 GouraudColor;

uniform vec3 viewPos;
uniform vec3 materialColor;
uniform float shininess;
uniform vec3 emissive;    // Young-hee red eye beams, neon badges, safe glass glow
uniform float alphaValue; // Translucent overlays (glass, shadows, blush)
uniform bool unlit;       // Flat unshaded color
uniform bool useGouraud;  // Gouraud (per-vertex) vs Phong (per-fragment)

// 1. Directional Light
uniform bool dirLightOn;
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;

// 2. Point Light
uniform bool pointLightOn;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float pointLightConstant;
uniform float pointLightLinear;
uniform float pointLightQuadratic;
uniform float pointLightIntensity;

// 3. Spot Light
uniform bool spotLightOn;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;
uniform float spotLightIntensity;

// Legacy fallback uniform support
uniform vec3 lightDir;
uniform vec3 lightColor;

// ACES Filmic Tone Mapping Curve (Standard in high-end cinema and modern game engines)
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 baseColor = materialColor * VertexColor;

    if (unlit) {
        FragColor = vec4(baseColor + emissive, alphaValue);
        return;
    }

    if (useGouraud) {
        // Gouraud Shading: lighting was calculated per-vertex in vertex shader
        vec3 litColor = GouraudColor * baseColor + emissive;

        // Apply Squid Game tone mapping & grade
        float luminance = dot(litColor, vec3(0.2126, 0.7152, 0.0722));
        vec3 shadowTeal = vec3(0.03, 0.10, 0.12);
        vec3 highlightWarm = vec3(1.04, 0.98, 0.92);
        vec3 graded = mix(litColor + shadowTeal * (1.0 - smoothstep(0.0, 0.45, luminance)),
                          litColor * highlightWarm,
                          smoothstep(0.35, 1.0, luminance));
        vec3 finalColor = ACESFilm(graded * 1.05);
        FragColor = vec4(finalColor, alphaValue);
        return;
    }

    // Phong Shading: full per-fragment normal & reflection evaluation
    vec3 norm = normalize(Normal);
    vec3 toCam = normalize(viewPos - FragPos);

    // 1. Dual-tone Hemispheric Ambient (Soft sky cool + warm sand/stage bounce)
    float skyFactor = norm.y * 0.5 + 0.5;
    vec3 skyColor = vec3(0.42, 0.52, 0.70);
    vec3 groundColor = vec3(0.36, 0.26, 0.16);
    vec3 ambient = mix(groundColor, skyColor, skyFactor) * 0.35;

    // Subtle Eye-Direction Soft Fill Light (prevents harsh pitch-black crevices)
    float fillDiff = max(dot(norm, toCam), 0.0) * 0.10;
    vec3 fillLight = fillDiff * vec3(0.88, 0.85, 0.82);

    vec3 totalDiffuse = ambient + fillLight;
    vec3 totalSpecular = vec3(0.0);
    vec3 totalRim = vec3(0.0);

    // -------------------------------------------------------------
    // LIGHT TYPE 1: DIRECTIONAL LIGHT (Sun / Stage Key Light)
    // -------------------------------------------------------------
    vec3 actualDir = (length(dirLightDir) > 0.001) ? dirLightDir : lightDir;
    vec3 actualDirCol = (length(dirLightColor) > 0.001) ? dirLightColor : lightColor;
    bool actualDirOn = dirLightOn || (!dirLightOn && length(actualDirCol) > 0.001 && !pointLightOn && !spotLightOn);

    if (actualDirOn && length(actualDir) > 0.001) {
        vec3 toLight = normalize(-actualDir);

        // Lambertian Diffuse
        float diff = max(dot(norm, toLight), 0.0);
        totalDiffuse += diff * actualDirCol * 0.60;

        // True Phong + Blinn-Phong Specular
        vec3 reflectDir = reflect(-toLight, norm);
        float specAngle = max(dot(toCam, reflectDir), 0.0);
        float phongSpec = pow(specAngle, max(shininess * 1.2, 4.0));

        vec3 halfwayDir = normalize(toLight + toCam);
        float blinnSpec = pow(max(dot(norm, halfwayDir), 0.0), max(shininess * 0.75, 2.0));
        float finalSpec = mix(phongSpec, blinnSpec, 0.35);
        totalSpecular += finalSpec * actualDirCol * 0.26;

        // Cinematic Fresnel Rim Light
        float rimFactor = 1.0 - max(dot(toCam, norm), 0.0);
        float rim = pow(rimFactor, 3.2) * max(dot(norm, toLight) * 0.5 + 0.5, 0.0);
        totalRim += rim * mix(vec3(0.85, 0.94, 1.0), actualDirCol, 0.5) * 0.28;
    }

    // -------------------------------------------------------------
    // LIGHT TYPE 2: POINT LIGHT (Omnidirectional with Attenuation)
    // -------------------------------------------------------------
    if (pointLightOn) {
        vec3 toPt = pointLightPos - FragPos;
        float dist = length(toPt);
        if (dist > 0.0001) {
            vec3 toPtNorm = toPt / dist;
            float atten = pointLightIntensity / (pointLightConstant + pointLightLinear * dist + pointLightQuadratic * dist * dist);

            // Diffuse
            float diffPt = max(dot(norm, toPtNorm), 0.0);
            totalDiffuse += diffPt * pointLightColor * atten;

            // Specular (Blinn-Phong)
            vec3 halfwayPt = normalize(toPtNorm + toCam);
            float specPt = pow(max(dot(norm, halfwayPt), 0.0), max(shininess * 0.85, 4.0));
            totalSpecular += specPt * pointLightColor * atten * 0.40;
        }
    }

    // -------------------------------------------------------------
    // LIGHT TYPE 3: SPOT LIGHT (Cone Angle Cutoff + Attenuation)
    // -------------------------------------------------------------
    if (spotLightOn) {
        vec3 toSp = spotLightPos - FragPos;
        float dist = length(toSp);
        if (dist > 0.0001) {
            vec3 toSpNorm = toSp / dist;
            vec3 spotDirNorm = normalize(-spotLightDir);
            float theta = dot(toSpNorm, spotDirNorm);
            float epsilon = max(spotLightCutOff - spotLightOuterCutOff, 0.0001);
            float spotIntensity = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);

            if (spotIntensity > 0.0) {
                float atten = spotLightIntensity / (1.0 + 0.045 * dist + 0.0075 * dist * dist);

                // Diffuse
                float diffSp = max(dot(norm, toSpNorm), 0.0);
                totalDiffuse += diffSp * spotLightColor * spotIntensity * atten;

                // Specular
                vec3 halfwaySp = normalize(toSpNorm + toCam);
                float specSp = pow(max(dot(norm, halfwaySp), 0.0), max(shininess, 4.0));
                totalSpecular += specSp * spotLightColor * spotIntensity * atten * 0.50;
            }
        }
    }

    // Composite Lit Color
    vec3 litColor = totalDiffuse * baseColor + totalSpecular + totalRim + emissive;

    // 6. Squid Game Color Grading & Split-Toning
    float luminance = dot(litColor, vec3(0.2126, 0.7152, 0.0722));
    vec3 shadowTeal = vec3(0.03, 0.10, 0.12);
    vec3 highlightWarm = vec3(1.04, 0.98, 0.92);

    vec3 graded = mix(litColor + shadowTeal * (1.0 - smoothstep(0.0, 0.45, luminance)),
                      litColor * highlightWarm,
                      smoothstep(0.35, 1.0, luminance));

    // Vibrance enhancement
    float maxVal = max(graded.r, max(graded.g, graded.b));
    float minVal = min(graded.r, min(graded.g, graded.b));
    float sat = (maxVal - minVal) / (maxVal + 0.0001);
    graded = mix(vec3(luminance), graded, 1.0 + (1.0 - sat) * 0.18);

    // ACES Filmic Tone Reproduction
    vec3 finalColor = ACESFilm(graded * 1.05);

    FragColor = vec4(finalColor, alphaValue);
}
