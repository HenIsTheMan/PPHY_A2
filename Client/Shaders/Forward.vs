#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 colour;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 normal;
layout (location = 4) in vec3 tangent;
layout (location = 5) in int diffuseTexIndex;
layout (location = 6) in mat4 modelMat;

out myInterface{
	vec3 pos;
	vec4 colour;
	vec2 texCoords;
	vec3 normal;
	flat int diffuseTexIndex;
} vsOut;

uniform mat4 PV;
uniform mat4 model;

uniform bool useBumpMap;
uniform sampler2D bumpMap;

uniform bool instancing;

void main(){
	vsOut.pos = vec3((instancing ? model * modelMat : model) * vec4(pos, 1.f));
	vsOut.colour = colour;
	vsOut.texCoords = texCoords;
	vsOut.normal = normalize(mat3(transpose(inverse(instancing ? model * modelMat : model))) * (useBumpMap ? texture(bumpMap, texCoords).rgb : normal));
	vsOut.diffuseTexIndex = diffuseTexIndex;
	gl_Position = PV * vec4(vsOut.pos, 1.f);
}