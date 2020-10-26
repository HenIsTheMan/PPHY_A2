#pragma once
#include <Core.h>
#include <Engine.h>

class Entity final{
	friend class Scene;
private:
	enum class EntityType{
		Ball = 0,
		Diode,
		Wall,
		Pillar,
		WallHidden,
		PillarHidden1,
		PillarHidden2,
		PillarHidden3,
		PillarHidden4,
		Coin,
		Amt
	};

	Entity();

	EntityType type;
	bool active;
	float life;
	float maxLife;
	glm::vec4 colour;
	int diffuseTexIndex;
	glm::vec3 scale;
	glm::vec3 collisionNormal;
	Light* light;

	glm::vec3 pos;
	glm::vec3 vel;
	float mass;
	glm::vec3 force;

	glm::vec3 facingDir;
	float angularVel;
	float angularMass;
	glm::vec3 torque;
};