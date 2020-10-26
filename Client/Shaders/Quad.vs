#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 texCoords;

out vec2 TexCoords;
uniform mat4 model;

void main(){
	gl_Position = model * vec4(pos.xy, 0.f, 1.f);
	TexCoords = texCoords;
}