#version 430

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec3 fragColor;

uniform sampler2D textureSampler;
uniform bool applyLighting; // Uniform to toggle point lighting for specific objects

// Uniforms for the directional light
uniform vec3 dirLightDirection;
uniform vec3 dirLightColor;
uniform vec3 ambientColor;

// Uniforms for the point lights
uniform vec3 lightPos1;
uniform vec3 lightColor1;
uniform vec3 lightPos2;
uniform vec3 lightColor2;
uniform vec3 lightPos3;
uniform vec3 lightColor3;

out vec4 outColor;


void main() {
    vec4 texColor = texture(textureSampler, fragTexCoord);
    vec3 norm = normalize(fragNormal);
    vec3 lightResult = vec3(0.0);

    // Ambient lighting
    vec3 ambient = ambientColor * texColor.rgb;

    // Directional lighting
    vec3 lightDir = normalize(-dirLightDirection);
    float diffDir = max(dot(norm, lightDir), 0.0);
    vec3 diffuseDir = diffDir * dirLightColor;

    // Add the result of directional lighting to the final result
    lightResult += diffuseDir;

    if (applyLighting) {
        // Point lighting is enabled for this object

        // Point light 1
        vec3 lightDir1 = normalize(lightPos1 - fragPosition);
        float diff1 = max(dot(norm, lightDir1), 0.0);
        vec3 diffuse1 = diff1 * lightColor1;

        // Point light 2
        vec3 lightDir2 = normalize(lightPos2 - fragPosition);
        float diff2 = max(dot(norm, lightDir2), 0.0);
        vec3 diffuse2 = diff2 * lightColor2;

        // Point light 2
        vec3 lightDir3 = normalize(lightPos3 - fragPosition);
        float diff3 = max(dot(norm, lightDir3), 0.0);
        vec3 diffuse3 = diff3 * lightColor3;

        // Add the point lighting results
        lightResult += (diffuse1 + diffuse2 + diffuse3);
    }

    // Combine the ambient light with the calculated lighting and apply it to the texture color
    vec3 finalColor = (ambient + lightResult) * texColor.rgb * fragColor;
    outColor = vec4(finalColor, texColor.a);
}