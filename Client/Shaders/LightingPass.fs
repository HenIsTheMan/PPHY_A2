#version 330 core

///Multiple Render Targets (MRT, can get multiple fs outputs in 1 render pass)
///Layout location specifier used (location corresponds to colour attachment of currently bound FBO the frag, that the fs is currently processing, will be rendered to [fs writes to...])
layout (location = 0) out vec4 fragColour;
layout (location = 1) out vec4 brightFragColour;

struct PtLight{ //Positional light source
    vec3 ambient;
    vec3 diffuse;
    vec3 spec;
    vec3 pos;
    float constant; //Constant term //Makes sure the denominator >= 1.f
    float linear; //Linear term //Multiplied with dist to reduce light intensity in a linear fashion
    float quadratic; //Quadratic term //Multiplied with the quadrant of the dist to set quadratic decreases in light intensity //Less significant compared to linear term when dist is small
};

struct DirectionalLight{ //Directional light source
    vec3 ambient;
    vec3 diffuse;
    vec3 spec;
    vec3 dir;
};

struct Spotlight{ //Positional light that shoots light rays in 1 dir, objs within its cut-offs (a certain radius of its dir) are lit
    vec3 ambient;
    vec3 diffuse;
    vec3 spec;
    vec3 pos;
    vec3 dir;
    float cosInnerCutoff;
    float cosOuterCutoff;
};

in vec2 TexCoords;
#define maxAmtP 30
#define maxAmtD 1
#define maxAmtS 50

uniform float shininess;
uniform vec3 globalAmbient;
uniform int pAmt;
uniform int dAmt;
uniform int sAmt;
uniform PtLight ptLights[maxAmtP];
uniform DirectionalLight directionalLights[maxAmtD];
uniform Spotlight spotlights[maxAmtS];

uniform vec3 camPos;
uniform sampler2D posTex;
uniform sampler2D coloursTex;
uniform sampler2D normalsTex;
uniform sampler2D specTex;
uniform sampler2D reflectionTex;
uniform samplerCube cubemapSampler;

vec3 WorldSpacePos = texture(posTex, TexCoords).rgb;
vec4 Colour = texture(coloursTex, TexCoords);
vec3 Normal = texture(normalsTex, TexCoords).rgb;
vec3 Spec = texture(specTex, TexCoords).rgb;
vec3 Reflection = texture(reflectionTex, TexCoords).rgb;

vec3 CalcAmbient(vec3 lightAmbient){
    return lightAmbient * Colour.rgb;
}

vec3 CalcDiffuse(vec3 lightDir, vec3 lightDiffuse){
    float dImpact = max(dot(Normal, -lightDir), 0.f); //Diffuse impact of light on curr frag
    return dImpact * lightDiffuse * Colour.rgb; //Diffuse component (> 0.f && <= 1.f when angle between... (>= 0.f && < 90.f) || (> 270.f && <= 360.f)) of frag
}

vec3 CalcSpec(vec3 lightDir, vec3 lightSpec){
    vec3 viewDir = normalize(WorldSpacePos - camPos);
    vec3 halfwayDir = -normalize(lightDir + viewDir);
    float sImpact = pow(max(dot(Normal, halfwayDir), 0.f), shininess);
    return sImpact * lightSpec * Spec;
}

vec3 CalcPtLight(PtLight light){
    vec3 lightDir = normalize(WorldSpacePos - light.pos);
    float dist = length(WorldSpacePos - light.pos);
    float attenuation = 1.f / (light.constant + light.linear * dist + light.quadratic * dist * dist);
    return attenuation * (CalcAmbient(light.ambient) + CalcDiffuse(lightDir, light.diffuse) + CalcSpec(lightDir, light.spec));
}

vec3 CalcDirectionalLight(DirectionalLight light){
    vec3 lightDir = normalize(light.dir);
    return CalcAmbient(light.ambient) + CalcDiffuse(lightDir, light.diffuse) + CalcSpec(lightDir, light.spec);
}

vec3 CalcSpotlight(Spotlight light){
    vec3 lightDir = normalize(WorldSpacePos - light.pos);
    float cosTheta = dot(lightDir, normalize(light.dir));
    float epsilon = light.cosInnerCutoff - light.cosOuterCutoff;
    float lightIntensity = clamp((cosTheta - light.cosOuterCutoff) / epsilon, 0.f, 1.f);
    return CalcAmbient(light.ambient) + lightIntensity * (CalcDiffuse(lightDir, light.diffuse) + CalcSpec(lightDir, light.spec));
}

void main(){
    if(Normal == vec3(0.f)){
        fragColour = Colour;
        brightFragColour = vec4(0.f);
    } else if(pAmt == 0 && dAmt == 0 && sAmt == 0){
        fragColour = vec4(CalcAmbient(globalAmbient), Colour.a);
        brightFragColour = vec4(0.f);
    } else{
        fragColour = vec4(vec3(0.f), Colour.a);
        for(int i = 0; i < pAmt; ++i){
            fragColour.rgb += CalcPtLight(ptLights[i]);
        }
        for(int i = 0; i < dAmt; ++i){
            fragColour.rgb += CalcDirectionalLight(directionalLights[i]);
        }
        for(int i = 0; i < sAmt; ++i){
            fragColour.rgb += CalcSpotlight(spotlights[i]);
        }

        if(Reflection != 0.f){
            const float ratio = 1.f / 1.52f; //n of air / n of glass (ratio between refractive indices of both materials)
            vec3 incidentRay = normalize(WorldSpacePos - camPos);
            vec3 reflectedRay = reflect(incidentRay, Normal);
            vec3 refractedRay = refract(incidentRay, Normal, ratio);
            fragColour.rgb += texture(cubemapSampler, reflectedRay).rgb * Reflection;
        }
        float brightness = dot(fragColour.rgb, vec3(.2126f, .7152f, .0722f)); //Transform fragColour to grayscale with dot product
        brightFragColour = vec4(fragColour.rgb * vec3(float(brightness > 3.f)), 1.f); //3.f is brightness threshold (outside LDR with HDR rendering so more control over what is considered bright)
    }
}