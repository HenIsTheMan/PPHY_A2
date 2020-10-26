#version 330 core
out vec4 fragColour;

in vec2 TexCoords;
uniform float exposure;
uniform sampler2D screenTexSampler;
uniform sampler2D blurTexSampler;

void main(){
	fragColour = vec4(texture(screenTexSampler, TexCoords).rgb, 1.f);
	fragColour.rgb += texture(blurTexSampler, TexCoords).rgb; //Additive blending

	//FragColor.rgb = FragColor.rgb / (FragColor.rgb + vec3(1.f)); //Reinhard tone mapping alg (evenly balances out colour values, tend to slightly favour bright areas)
    fragColour.rgb = vec3(1.f) - exp(-fragColour.rgb * exposure); //Exposure tone mapping (...)
}