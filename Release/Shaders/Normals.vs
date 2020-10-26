#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 colour;
layout (location = 3) in vec3 normal;

out myInterface{
	vec4 colour;
	vec3 normal;
} vsOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	gl_Position = projection * view * model * vec4(vsOut.pos, 1.f);
	vsOut.colour = colour;

	mat3 normalMat = mat3(transpose(inverse(view * model)));
	vsOut.normal = normalize(vec3(projection * vec4(normalMat * normal, 0.f)));
}