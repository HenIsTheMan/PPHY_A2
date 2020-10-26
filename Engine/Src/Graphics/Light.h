#pragma once
#include "../Core.h"

class Light{
public:
    glm::vec3 ambient; //Affects colour and intensity of ambient component of light
    glm::vec3 diffuse; //...
    glm::vec3 spec; //...
    static glm::vec3 globalAmbient; //Affects brightness of unlit scene
    virtual ~Light() = default;
protected:
    Light():
        ambient(glm::vec3(.5f)),
        diffuse(glm::vec3(.8f)),
        spec(glm::vec3(1.f))
    {
    }
};

struct PtLight final: Light{
    PtLight(): PtLight(glm::vec3(0.f), 1.f, .09f, .032f){}
    PtLight(const glm::vec3& pos, const float& constant, const float& linear, const float& quadratic):
        pos(pos),
        constant(constant),
        linear(linear),
        quadratic(quadratic)
    {
    }
    glm::vec3 pos;
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight final: Light{
    DirectionalLight(): DirectionalLight(glm::vec3(0.f, -1.f, 0.f)){}
    DirectionalLight(const glm::vec3& dir):
        dir(dir)
    {
    }
    glm::vec3 dir;
};

struct Spotlight final: Light{
    Spotlight(): Spotlight(glm::vec3(0.f), glm::vec3(0.f), glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f))){}
    Spotlight(const glm::vec3& pos, const glm::vec3& dir, const float& cosInnerCutoff, const float& cosOuterCutoff):
        pos(pos),
        dir(dir),
        cosInnerCutoff(cosInnerCutoff),
        cosOuterCutoff(cosOuterCutoff)
    {
    }
    glm::vec3 pos;
    glm::vec3 dir;
    float cosInnerCutoff;
    float cosOuterCutoff;
};

enum struct LightType{
    Pt = 0,
    Directional,
    Spot,
    Amt
};

static Light* CreateLight(const LightType& type){
    switch(type){
        case LightType::Pt:
            return new PtLight();
        case LightType::Directional:
            return new DirectionalLight();
        case LightType::Spot:
            return new Spotlight();
        default:
            return nullptr;
    }
}