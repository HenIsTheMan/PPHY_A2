#version 330 core
out vec4 fragColour;

in myInterface{
	vec3 pos;
	vec4 colour;
	vec2 texCoords;
	vec3 normal;
    flat int diffuseTexIndex;
} fsIn;

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

#define maxAmtP 5
#define maxAmtD 5
#define maxAmtS 5

uniform float shininess;
uniform vec3 globalAmbient;
uniform int pAmt;
uniform int dAmt;
uniform int sAmt;
uniform PtLight ptLights[maxAmtP];
uniform DirectionalLight directionalLights[maxAmtD];
uniform Spotlight spotlights[maxAmtS];

uniform vec3 camPos;
uniform samplerCube cubemapSampler;

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

vec3 WorldSpacePos = fsIn.pos;
vec4 Colour;
vec3 Normal = fsIn.normal;
vec3 Spec = useSpecMap ? texture(specMap, fsIn.texCoords).rgb : vec3(0.f); //Use full white tex for full spec
vec3 Reflection = useReflectionMap ? texture(reflectionMap, fsIn.texCoords).rgb : vec3(0.f); //Use full white tex for full reflection

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
	if(!useCustomColour && !useDiffuseMap){
		Colour = fsIn.colour;
	} else{
		Colour = (useCustomColour ? customColour : vec4(1.f)) * (useDiffuseMap && (useCustomDiffuseTexIndex ? customDiffuseTexIndex : fsIn.diffuseTexIndex) >= 0 ? texture(diffuseMaps[useCustomDiffuseTexIndex ? customDiffuseTexIndex : fsIn.diffuseTexIndex], fsIn.texCoords) : vec4(1.f));
	}
	if(useEmissionMap){
		Colour.rgb += texture(emissionMap, fsIn.texCoords).rgb;
	}

    if(Normal == vec3(0.f)){
        fragColour = Colour;
    } else if(pAmt == 0 && dAmt == 0 && sAmt == 0){
        fragColour = vec4(CalcAmbient(globalAmbient), Colour.a);
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
    }
}