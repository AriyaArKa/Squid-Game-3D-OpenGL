#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform float shininess;
uniform bool unlit;
uniform bool useGouraud;

// 1. Directional Light Uniforms
uniform bool dirLightOn;
uniform vec3 dirLightDir;
uniform vec3 dirLightColor;

// 2. Point Light Uniforms
uniform bool pointLightOn;
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float pointLightConstant;
uniform float pointLightLinear;
uniform float pointLightQuadratic;
uniform float pointLightIntensity;

// 3. Spot Light Uniforms
uniform bool spotLightOn;
uniform vec3 spotLightPos;
uniform vec3 spotLightDir;
uniform vec3 spotLightColor;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;
uniform float spotLightIntensity;

out vec3 FragPos;
out vec3 Normal;
out vec3 VertexColor;
out vec3 GouraudColor;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    VertexColor = aColor;
    gl_Position = projection * view * vec4(FragPos, 1.0);

    if (useGouraud && !unlit) {
        vec3 norm = normalize(Normal);
        vec3 toCam = normalize(viewPos - FragPos);

        // Hemispheric ambient lighting
        float skyFactor = norm.y * 0.5 + 0.5;
        vec3 skyColor = vec3(0.42, 0.52, 0.70);
        vec3 groundColor = vec3(0.36, 0.26, 0.16);
        vec3 totalDiffuse = mix(groundColor, skyColor, skyFactor) * 0.35;
        vec3 totalSpecular = vec3(0.0);

        // 1. Directional Light
        if (dirLightOn) {
            vec3 toDirLight = normalize(-dirLightDir);
            float diff = max(dot(norm, toDirLight), 0.0);
            totalDiffuse += diff * dirLightColor * 0.65;

            vec3 halfwayDir = normalize(toDirLight + toCam);
            float spec = pow(max(dot(norm, halfwayDir), 0.0), max(shininess * 0.75, 2.0));
            totalSpecular += spec * dirLightColor * 0.25;
        }

        // 2. Point Light (with Attenuation)
        if (pointLightOn) {
            vec3 toPt = pointLightPos - FragPos;
            float dist = length(toPt);
            if (dist > 0.0001) {
                vec3 toPtNorm = toPt / dist;
                float atten = pointLightIntensity / (pointLightConstant + pointLightLinear * dist + pointLightQuadratic * dist * dist);
                float diff = max(dot(norm, toPtNorm), 0.0);
                totalDiffuse += diff * pointLightColor * atten;

                vec3 halfwayDir = normalize(toPtNorm + toCam);
                float spec = pow(max(dot(norm, halfwayDir), 0.0), max(shininess * 0.75, 2.0));
                totalSpecular += spec * pointLightColor * atten * 0.35;
            }
        }

        // 3. Spot Light (with Cone Cutoff and Attenuation)
        if (spotLightOn) {
            vec3 toSp = spotLightPos - FragPos;
            float dist = length(toSp);
            if (dist > 0.0001) {
                vec3 toSpNorm = toSp / dist;
                float theta = dot(toSpNorm, normalize(-spotLightDir));
                float epsilon = max(spotLightCutOff - spotLightOuterCutOff, 0.0001);
                float spotFactor = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);

                if (spotFactor > 0.0) {
                    float atten = spotLightIntensity / (1.0 + 0.045 * dist + 0.0075 * dist * dist);
                    float diff = max(dot(norm, toSpNorm), 0.0);
                    totalDiffuse += diff * spotLightColor * spotFactor * atten;

                    vec3 halfwayDir = normalize(toSpNorm + toCam);
                    float spec = pow(max(dot(norm, halfwayDir), 0.0), max(shininess * 0.75, 2.0));
                    totalSpecular += spec * spotLightColor * spotFactor * atten * 0.40;
                }
            }
        }

        GouraudColor = totalDiffuse + totalSpecular;
    } else {
        GouraudColor = vec3(1.0);
    }
}
