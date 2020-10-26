#pragma once
#include "../Core.h"

struct Vertex final{
	glm::vec3 pos = glm::vec3(0.f);
	glm::vec4 colour = glm::vec4(.7f, .4f, .1f, 1.f);
    glm::vec2 texCoords = glm::vec2(0.f);
    glm::vec3 normal = glm::vec3(0.f);
    glm::vec3 tangent = glm::vec3(0.f);
    int diffuseTexIndex = 0;
};