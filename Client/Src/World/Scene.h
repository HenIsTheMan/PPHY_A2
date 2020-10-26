#pragma once
#include <Engine.h>
#include "Cam.h"
#include "../PPHY/Entity.h"

class Scene final{
public:
	Scene();
	~Scene();
	bool Init();
	void Update();
	void GeoRenderPass();
	void LightingRenderPass(const uint& posTexRefID, const uint& coloursTexRefID, const uint& normalsTexRefID, const uint& specTexRefID, const uint& reflectionTexRefID);
	void BlurRender(const uint& brightTexRefID, const bool& horizontal);
	void DefaultRender(const uint& screenTexRefID, const uint& blurTexRefID);
	void ForwardRender();
private:
	Cam cam;
	ISoundEngine* soundEngine;
	ISound* music;
	ISoundEffectControl* soundFX;
	TextChief textChief;

	bool spawnEmpty;
	Entity* portal;
	float angle0;
	float angle1;
	float ballSpawnBT;
	float buttonBT;
	float textScaleFactors[3];
	glm::vec4 textColours[3];
	float timeLeft;
	int score;
	std::vector<int> scores;
	std::vector<Entity*> activeBalls;
	std::vector<Entity*> flipper0;
	std::vector<Entity*> flipper1;
	std::vector<std::vector<Entity*>> changeableThickWalls;
	std::vector<std::vector<Entity*>> cross;
	std::vector<std::vector<Entity*>> curvedWall;
	std::vector<std::vector<Entity*>> polygons;
	enum struct Screen{
		MainMenu = 0,
		Game,
		GameOver,
		Scoreboard,
		Amt
	};
	Screen screen;
	std::vector<Entity*> entityPool;
	Entity* const& FetchEntity();
	struct EntityCreationAttribs final{
		glm::vec3 pos;
		glm::vec3 collisionNormal;
		glm::vec3 facingDir;
		glm::vec3 scale;
		glm::vec4 colour;
		int diffuseTexIndex;
		float angularVel;
		int type = 0;
	};
	Entity& CreateBall(const EntityCreationAttribs& attribs);
	Entity& CreateDiode(const EntityCreationAttribs& attribs);
	Entity& CreateThinWall(const EntityCreationAttribs& attribs);
	Entity& CreatePillar(const EntityCreationAttribs& attribs);
	Entity& CreateCoin(const EntityCreationAttribs& attribs);
	std::vector<Entity*> CreateThickWall(const EntityCreationAttribs& attribs);
	std::vector<Entity*> CreateHollowPolygon(const int& sidesAmt, const float& len, const EntityCreationAttribs& attribs);
	void UpdateEntities();
	void RenderEntities(ShaderProg& SP, const bool& opaque);
	bool IsColliding(const Entity* const& actor, const Entity* const& actee) const;
	void ResolveCollision(Entity* const& actor, Entity* const& actee);

	enum struct GeoType{
		Quad,
		Cube,
		Sphere,
		Cylinder,
		CoinSpriteAni,
		Amt
	};
	Mesh* meshes[(int)GeoType::Amt];

	ShaderProg blurSP;
	ShaderProg forwardSP;
	ShaderProg geoPassSP;
	ShaderProg lightingPassSP;
	ShaderProg normalsSP;
	ShaderProg screenSP;
	ShaderProg textSP;

	std::vector<Light*> ptLights;
	std::vector<Light*> directionalLights;
	std::vector<Light*> spotlights;

	glm::mat4 view;
	glm::mat4 projection;

	float elapsedTime;
	float polyModeBT;
	float distortionBT;
	float echoBT;
	float wavesReverbBT;
	float resetSoundFXBT;

	mutable std::stack<glm::mat4> modelStack;
	glm::mat4 Translate(const glm::vec3& translate);
	glm::mat4 Rotate(const glm::vec4& rotate);
	glm::mat4 Scale(const glm::vec3& scale);
	glm::mat4 GetTopModel() const;
	void PushModel(const std::vector<glm::mat4>& vec) const;
	void PopModel() const;
};

inline static glm::vec3 RotateVecIn2D(const glm::vec3& vec, const float& angleInRad){
	return glm::vec3(vec.x * cos(angleInRad) + vec.y * -sin(angleInRad), vec.x * sin(angleInRad) + vec.y * cos(angleInRad), 0.f);
}