#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 light_space;

uniform mat4 inverse_model_matrix;

out vec3 fragPos;
out vec4 fragPosLightSpace;
out vec3 vertexNormal;

void main() {
    fragPos = vec3(model_matrix * vec4(position, 1.0));
    vertexNormal = mat3(inverse_model_matrix) * normal;
    fragPosLightSpace = light_space * vec4(fragPos, 1.0);
    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0f);
}