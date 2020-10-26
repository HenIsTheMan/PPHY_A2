#version 330 core
layout (location = 0) out vec3 pos;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec4 albedoSpec;

out myInterface{
	vec4 colour;
	vec3 normal;
} fsIn;

void main(){
    pos = fsIn.pos;
	normal = fsIn.normal; //??
	albedoSpec = fsIn.colour;
}

//Use forward rendering/...??