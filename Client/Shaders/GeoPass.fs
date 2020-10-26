#version 330 core
layout (location = 0) out vec3 pos;
layout (location = 1) out vec4 colour;
layout (location = 2) out vec3 normal;
layout (location = 3) out vec3 spec;
layout (location = 4) out vec3 reflection;

in myInterface{
	vec3 pos;
	vec4 colour;
	vec2 texCoords;
	vec3 normal;
	flat int diffuseTexIndex;
} fsIn;

///Can be set by client
uniform bool useCustomDiffuseTexIndex;
uniform bool useCustomColour;
uniform int customDiffuseTexIndex;
uniform vec4 customColour;

uniform bool useDiffuseMap;
uniform bool useSpecMap;
uniform bool useEmissionMap;
uniform bool useReflectionMap;

uniform sampler2D diffuseMaps[29];
uniform sampler2D specMap;
uniform sampler2D emissionMap;
uniform sampler2D reflectionMap;

void main(){
    pos = fsIn.pos;

	if(!useCustomColour && !useDiffuseMap){
		colour = fsIn.colour;
	} else{
		colour = (useCustomColour ? customColour : vec4(1.f)) * (useDiffuseMap && (useCustomDiffuseTexIndex ? customDiffuseTexIndex : fsIn.diffuseTexIndex) >= 0 ? texture(diffuseMaps[useCustomDiffuseTexIndex ? customDiffuseTexIndex : fsIn.diffuseTexIndex], fsIn.texCoords) : vec4(1.f));
	}
	if(useEmissionMap){
		colour.rgb += texture(emissionMap, fsIn.texCoords).rgb;
	}

	normal = fsIn.normal;
	spec = useSpecMap ? texture(specMap, fsIn.texCoords).rgb : vec3(0.f); //Use full white tex for full spec
	reflection = useReflectionMap ? texture(reflectionMap, fsIn.texCoords).rgb : vec3(0.f); //Use full white tex for full reflection
}