#version 330 core
out vec4 fragColour;

in vec2 TexCoords;

uniform vec4 textColour;
uniform sampler2D textTex;
uniform sampler2D texSampler;

void main(){
	fragColour = textColour;
	vec4 colourFromTex = texture(textTex, TexCoords);
	if(colourFromTex.rgb != vec3(0.f)){
		fragColour *= colourFromTex;
	}
	fragColour.a *= texture(texSampler, TexCoords).r;
}