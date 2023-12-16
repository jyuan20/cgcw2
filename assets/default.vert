#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal; // Adding normal attribute
layout(location = 3) in vec2 texCoord; // Add texture coordinate attribute

uniform mat4 projCameraWorld;

out vec3 fragColor;
out vec3 fragNormal; // Output the normal to the fragment shader
out vec2 fragTexCoord; // Output the texture coordinate to the fragment shader

void main() {
    gl_Position = projCameraWorld * vec4(position, 1.0);
    fragColor = color;
    fragNormal = normal; // Pass the normal to the fragment shader
    fragTexCoord = texCoord;
}
