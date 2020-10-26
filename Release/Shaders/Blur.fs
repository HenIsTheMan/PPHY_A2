#version 330 core
out vec4 fragColour;

in vec2 TexCoords;
uniform sampler2D texSampler;

uniform bool horizontal;
uniform float weights[5] = float[](.227027f, .1945946f, .1216216f, .054054f, .016216f); //Take a few samples of Gaussian weights and assign each to horizontal or vert samples around the curr frag

void main(){ //Gaussian blur filter taking 5 tex samples in each dir (take more tex samples along a larger radius or blur more times [controlled in CPU] to improve)
    vec2 texelSize = 1.f / textureSize(texSampler, 0); //Size of 1 texel
    vec3 result = texture(texSampler, TexCoords).rgb * weights[0]; //Contribution of curr frag
    for(int i = 1; i < 5; ++i){
        vec2 offset = horizontal ? vec2(texelSize.x * i, 0.f) : vec2(0.f, texelSize.y * i);
        result += texture(texSampler, TexCoords + offset).rgb * weights[i];
        result += texture(texSampler, TexCoords - offset).rgb * weights[i];
    }
    fragColour = vec4(result, 1.f);
}