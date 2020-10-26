#include "Scene.h"
#include "Vendor/stb_image.h"

//#define MOVE_CAM

extern bool endLoop;
extern float leftRightMB;
extern float angularFOV;
extern float dt;
extern int winWidth;
extern int winHeight;

const glm::vec3 gravAccel(0.f, -10.f, 0.f);
constexpr float FRICTION_COEFFICIENT = .05f;
constexpr float MAG_MULTIPLIER = 5.f;
constexpr float GAME_SPD = 1.f;

static const glm::vec3 pos0 = glm::vec3(-9.5f, -29.5f, 0.f);
static const glm::vec3 pos1 = glm::vec3(5.5f, -29.5f, 0.f);
static const glm::vec3 facingDir0 = glm::normalize(glm::vec3(.2f, 1.f, 0.f));
static const glm::vec3 facingDir1 = glm::normalize(glm::vec3(-.2f, 1.f, 0.f));
static const glm::vec3 scale = glm::vec3(1.f, 5.f, 3.f);

glm::vec3 Light::globalAmbient = glm::vec3(.2f);

Scene::Scene():
	cam(glm::vec3(0.f, 0.f, 130.f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f), 0.f, 100.f),
	soundEngine(nullptr),
	music(nullptr),
	soundFX(nullptr),
	spawnEmpty(false),
	portal(nullptr),
	angle0(0.f),
	angle1(0.f),
	ballSpawnBT(0.f),
	buttonBT(0.f),
	textScaleFactors{
		1.f,
		1.f,
		1.f,
	},
	textColours{
		glm::vec4(1.f),
		glm::vec4(1.f),
		glm::vec4(1.f),
	},
	timeLeft(60.f),
	score(0),
	scores({}),
	screen(Screen::MainMenu),
	activeBalls({}),
	flipper0({}),
	flipper1({}),
	changeableThickWalls({}),
	cross({}),
	curvedWall({}),
	polygons({}),
	entityPool(std::vector<Entity*>(1000)),
	meshes{
		new Mesh(Mesh::MeshType::Quad, GL_TRIANGLES, {
			{"Imgs/BG.jpg", Mesh::TexType::Diffuse, 0},
			{"Imgs/Chalkboard.jpg", Mesh::TexType::Diffuse, 0},
		}),
		new Mesh(Mesh::MeshType::Cube, GL_TRIANGLES, {
			{"Imgs/DarkOldWood.jpg", Mesh::TexType::Diffuse, 0},
			{"Imgs/LightOldWood.jpg", Mesh::TexType::Diffuse, 0},
			{"Imgs/Foil.jpg", Mesh::TexType::Diffuse, 0},
		}),
		new Mesh(Mesh::MeshType::Sphere, GL_TRIANGLE_STRIP, {
			{"Imgs/Metal.jpg", Mesh::TexType::Diffuse, 0},
		}),
		new Mesh(Mesh::MeshType::Cylinder, GL_TRIANGLE_STRIP, {
			{"Imgs/Rubber.jpg", Mesh::TexType::Diffuse, 0},
		}),
		new SpriteAni(1, 6),
	},
	blurSP{"Shaders/Quad.vs", "Shaders/Blur.fs"},
	forwardSP{"Shaders/Forward.vs", "Shaders/Forward.fs"},
	geoPassSP{"Shaders/GeoPass.vs", "Shaders/GeoPass.fs"},
	lightingPassSP{"Shaders/Quad.vs", "Shaders/LightingPass.fs"},
	normalsSP{"Shaders/Normals.vs", "Shaders/Normals.fs", "Shaders/Normals.gs"}, //??
	screenSP{"Shaders/Quad.vs", "Shaders/Screen.fs"},
	textSP{"Shaders/Text.vs", "Shaders/Text.fs"},
	ptLights({}),
	directionalLights({}),
	spotlights({}),
	view(glm::mat4(1.f)),
	projection(glm::mat4(1.f)),
	elapsedTime(0.f),
	polyModeBT(0.f),
	distortionBT(0.f),
	echoBT(0.f),
	wavesReverbBT(0.f),
	resetSoundFXBT(0.f),
	modelStack()
{
}

void Scene::UpdateEntities(){
	if(spawnEmpty && ballSpawnBT <= elapsedTime && !Key(VK_SPACE)){
		Entity* ball = &CreateBall({
			glm::vec3(48.f, -48.f, 0.f),
			glm::vec3(0.f),
			glm::vec3(1.f, 0.f, 0.f),
			glm::vec3(1.f),
			glm::vec4(glm::vec3(.5f), 1.f),
			0,
			0.f,
		});
		Light* light = CreateLight(LightType::Spot);
		static_cast<Spotlight*>(light)->diffuse = glm::vec3(5.f);
		static_cast<Spotlight*>(light)->spec = glm::vec3(5.f);
		static_cast<Spotlight*>(light)->cosInnerCutoff = glm::cos(glm::radians(30.f));
		static_cast<Spotlight*>(light)->cosOuterCutoff = glm::cos(glm::radians(30.f));
		static_cast<Spotlight*>(light)->pos = glm::vec3(ball->pos.x, ball->pos.y, 5.f);
		static_cast<Spotlight*>(light)->dir = glm::vec3(0.f, 0.f, -1.f);
		ball->light = light;
		spotlights.emplace_back(light);
		activeBalls.emplace_back(ball);
		spawnEmpty = false;
	} else if(!spawnEmpty){
		ballSpawnBT = elapsedTime + 5.f;
	}

	static bool flipper0Up = false;
	static bool flipper1Up = false;
	if(Key(VK_LEFT)){
		if(!flipper0Up){
			soundEngine->play2D("Audio/Sounds/Whoosh.flac", false);
			flipper0Up = true;
		}
		flipper0[0]->angularVel = 80.f;
	} else{
		flipper0Up = false;
		flipper0[0]->angularVel = -80.f;
	}
	if(Key(VK_RIGHT)){
		if(!flipper1Up){
			soundEngine->play2D("Audio/Sounds/Whoosh.flac", false);
			flipper1Up = true;
		}
		flipper1[0]->angularVel = -80.f;
	} else{
		flipper1Up = false;
		flipper1[0]->angularVel = 80.f;
	}

	angle0 += flipper0[0]->angularVel * MAG_MULTIPLIER * dt;
	if(angle0 > 30.f){
		angle0 = 30.f;
	}
	if(angle0 < 0.f){
		angle0 = 0.f;
	}
	angle1 += flipper1[0]->angularVel * MAG_MULTIPLIER * dt;
	if(angle1 < -30.f){
		angle1 = -30.f;
	}
	if(angle1 > 0.f){
		angle1 = 0.f;
	}

	const size_t& mySize = entityPool.size();
	for(size_t i = 0; i < mySize; ++i){
		Entity* const& entity = entityPool[i];
		if(entity && entity->active){
			if(entity->type == Entity::EntityType::Ball){
				if(Key(VK_SPACE) && entity->pos == glm::vec3(48.f, -48.f, 0.f)){
					entity->vel = glm::vec3(0.f, PseudorandMinMax(20.f, 25.f), 0.f) * MAG_MULTIPLIER;
					entity->force = entity->mass * gravAccel * MAG_MULTIPLIER;
					spawnEmpty = true;
				}

				if(entity->pos != glm::vec3(48.f, -48.f, 0.f)){
					entity->force = entity->mass *
						(gravAccel + fabs((FRICTION_COEFFICIENT * gravAccel).y) * -(glm::length(entity->vel) ? glm::normalize(entity->vel) : glm::vec3(0.f, 1.f, 0.f))) * MAG_MULTIPLIER;
				}
				if(entity->light){
					static_cast<Spotlight*>(entity->light)->pos = glm::vec3(entity->pos.x, entity->pos.y, 5.f);
				}
			}
			entity->vel += (entity->force / entity->mass) * dt * GAME_SPD;


			bool found = false;

			///Correct pos of flipper0
			const size_t& flipper0Size = flipper0.size();
			for(size_t i = 0; i < flipper0Size; ++i){
				Entity* const& myFlipper = flipper0[i];
				if(myFlipper && myFlipper->active && myFlipper == entity){
					found = true;
					const glm::vec3 N = flipper0[0]->collisionNormal;
					const glm::vec3 NP = glm::vec3(-N.y, N.x, N.z);
					switch(myFlipper->type){
						case Entity::EntityType::Wall:
							myFlipper->pos = RotateVecIn2D(glm::vec3(scale.y, 0.f, 0.f), glm::radians(angle0)) + glm::vec3(pos0.x - scale.y, pos0.y, pos0.z);
							myFlipper->collisionNormal = myFlipper->facingDir = glm::length(facingDir0) ? glm::normalize(RotateVecIn2D(facingDir0, glm::radians(angle0))) : glm::vec3(0.f, 1.f, 0.f);
							break;
						case Entity::EntityType::WallHidden:
							myFlipper->pos = flipper0[0]->pos;
							myFlipper->collisionNormal = NP;
							break;
						case Entity::EntityType::PillarHidden1:
							myFlipper->pos = flipper0[0]->pos + NP * flipper0[0]->scale.y + N * flipper0[0]->scale.x;
							break;
						case Entity::EntityType::PillarHidden2:
							myFlipper->pos = flipper0[0]->pos + -NP * flipper0[0]->scale.y + -N * flipper0[0]->scale.x;
							break;
						case Entity::EntityType::PillarHidden3:
							myFlipper->pos = flipper0[0]->pos + -NP * flipper0[0]->scale.y + N * flipper0[0]->scale.x;
							break;
						case Entity::EntityType::PillarHidden4:
							myFlipper->pos = flipper0[0]->pos + NP * flipper0[0]->scale.y + -N * flipper0[0]->scale.x;
							break;
					}
				}
				if(found){
					break;
				}
			}

			///Correct pos of flipper1
			const size_t& flipper1Size = flipper1.size();
			for(size_t i = 0; i < flipper1Size; ++i){
				Entity* const& myFlipper = flipper1[i];
				if(myFlipper && myFlipper->active && myFlipper == entity){
					found = true;
					const glm::vec3 N = flipper1[0]->collisionNormal;
					const glm::vec3 NP = glm::vec3(-N.y, N.x, N.z);
					switch(myFlipper->type){
						case Entity::EntityType::Wall:
							myFlipper->pos = RotateVecIn2D(glm::vec3(-scale.y, 0.f, 0.f), glm::radians(angle1)) + glm::vec3(pos1.x + scale.y, pos1.y, pos1.z);
							myFlipper->collisionNormal = myFlipper->facingDir = glm::length(facingDir1) ? glm::normalize(RotateVecIn2D(facingDir1, glm::radians(angle1))) : glm::vec3(0.f, 1.f, 0.f);
							break;
						case Entity::EntityType::WallHidden:
							myFlipper->pos = flipper1[0]->pos;
							myFlipper->collisionNormal = NP;
							break;
						case Entity::EntityType::PillarHidden1:
							myFlipper->pos = flipper1[0]->pos + NP * flipper1[0]->scale.y + N * flipper1[0]->scale.x;
							break;
						case Entity::EntityType::PillarHidden2:
							myFlipper->pos = flipper1[0]->pos + -NP * flipper1[0]->scale.y + -N * flipper1[0]->scale.x;
							break;
						case Entity::EntityType::PillarHidden3:
							myFlipper->pos = flipper1[0]->pos + -NP * flipper1[0]->scale.y + N * flipper1[0]->scale.x;
							break;
						case Entity::EntityType::PillarHidden4:
							myFlipper->pos = flipper1[0]->pos + NP * flipper1[0]->scale.y + -N * flipper1[0]->scale.x;
							break;
					}
				}
				if(found){
					break;
				}
			}

			if(!found){
				entity->angularVel += entity->torque.z / entity->angularMass * dt * GAME_SPD;
				entity->facingDir = RotateVecIn2D(entity->facingDir, entity->angularVel * dt * GAME_SPD);
				entity->collisionNormal = RotateVecIn2D(entity->collisionNormal, entity->angularVel * dt * GAME_SPD);
			}
			for(size_t j = i + 1; j < mySize; ++j){
				Entity* const& instance = entityPool[j];
				if(instance && instance->active && instance != entity){
					if(entity->type != Entity::EntityType::Ball && instance->type != Entity::EntityType::Ball){
						continue;
					}
					Entity* actor = entity->type == Entity::EntityType::Ball ? entity : instance;
					Entity* actee = actor == entity ? instance : entity;
					if(IsColliding(actor, actee)){
						ResolveCollision(actor, actee);
					}
				}
			}
			entity->vel.x = std::min(100.f, std::max(-100.f, entity->vel.x));
			entity->vel.y = std::min(100.f, std::max(-100.f, entity->vel.y));
			entity->pos += entity->vel * dt * GAME_SPD;
		}
	}

	///Correct pos of hidden pillars of changeable thick walls
	const size_t& myOtherSize = changeableThickWalls.size();
	for(size_t i = 0; i < myOtherSize; ++i){
		const size_t& myOtherSize2 = changeableThickWalls[i].size();
		for(size_t j = 0; j < myOtherSize2; ++j){
			Entity* const& entity = changeableThickWalls[i][j];
			if(entity && entity->active && entity->angularVel != 0.f){
				const glm::vec3 N = changeableThickWalls[i][0]->collisionNormal;
				const glm::vec3 NP = glm::vec3(-N.y, N.x, N.z);
				switch(entity->type){
					case Entity::EntityType::PillarHidden1:
						entity->pos = changeableThickWalls[i][0]->pos + NP * changeableThickWalls[i][0]->scale.y + N * changeableThickWalls[i][0]->scale.x;
						break;
					case Entity::EntityType::PillarHidden2:
						entity->pos = changeableThickWalls[i][0]->pos + -NP * changeableThickWalls[i][0]->scale.y + -N * changeableThickWalls[i][0]->scale.x;
						break;
					case Entity::EntityType::PillarHidden3:
						entity->pos = changeableThickWalls[i][0]->pos + -NP * changeableThickWalls[i][0]->scale.y + N * changeableThickWalls[i][0]->scale.x;
						break;
					case Entity::EntityType::PillarHidden4:
						entity->pos = changeableThickWalls[i][0]->pos + NP * changeableThickWalls[i][0]->scale.y + -N * changeableThickWalls[i][0]->scale.x;
						break;
				}
			}
		}
	}

	///Correct pos of hidden pillars of cross
	const size_t& myOtherSize3 = cross.size();
	for(size_t i = 0; i < myOtherSize3; ++i){
		const size_t& myOtherSize4 = cross[i].size();
		for(size_t j = 0; j < myOtherSize4; ++j){
			Entity* const& entity = cross[i][j];
			if(entity && entity->active && entity->angularVel != 0.f){
				const glm::vec3 N = cross[i][0]->collisionNormal;
				const glm::vec3 NP = glm::vec3(-N.y, N.x, N.z);
				switch(entity->type){
					case Entity::EntityType::PillarHidden1:
						entity->pos = cross[i][0]->pos + NP * cross[i][0]->scale.y + N * cross[i][0]->scale.x;
						break;
					case Entity::EntityType::PillarHidden2:
						entity->pos = cross[i][0]->pos + -NP * cross[i][0]->scale.y + -N * cross[i][0]->scale.x;
						break;
					case Entity::EntityType::PillarHidden3:
						entity->pos = cross[i][0]->pos + -NP * cross[i][0]->scale.y + N * cross[i][0]->scale.x;
						break;
					case Entity::EntityType::PillarHidden4:
						entity->pos = cross[i][0]->pos + NP * cross[i][0]->scale.y + -N * cross[i][0]->scale.x;
						break;
				}
			}
		}
	}
}

void Scene::RenderEntities(ShaderProg& SP, const bool& opaque){
	const size_t& mySize = entityPool.size();
	for(size_t i = 0; i < mySize; ++i){
		Entity* const& entity = entityPool[i];
		if(entity && entity->active && (opaque ? (entity->colour.a == 1.f) : (entity->colour.a != 1.f))){
			PushModel({
				Translate(entity->pos),
				Rotate(glm::vec4(0.f, 0.f, 1.f, glm::degrees(atan2(entity->facingDir.y, entity->facingDir.x)))),
				Scale(entity->scale),
			});
			SP.Set1i("useCustomColour", 1);
			SP.Set4fv("customColour", entity->colour);
			SP.Set1i("useCustomDiffuseTexIndex", 1);
			SP.Set1i("customDiffuseTexIndex", entity->diffuseTexIndex);
			switch(entity->type){
				case Entity::EntityType::Ball:
					meshes[(int)GeoType::Sphere]->SetModel(GetTopModel());
					meshes[(int)GeoType::Sphere]->Render(SP);
					break;
				case Entity::EntityType::Diode:
				case Entity::EntityType::Wall:
					meshes[(int)GeoType::Cube]->SetModel(GetTopModel());
					meshes[(int)GeoType::Cube]->Render(SP);
					break;
				case Entity::EntityType::Pillar:
					meshes[(int)GeoType::Cylinder]->SetModel(GetTopModel());
					meshes[(int)GeoType::Cylinder]->Render(SP);
					break;
				case Entity::EntityType::Coin:
					meshes[(int)GeoType::CoinSpriteAni]->SetModel(GetTopModel());
					meshes[(int)GeoType::CoinSpriteAni]->Render(SP);
					break;
			}
			SP.Set1i("useCustomDiffuseTexIndex", 0);
			SP.Set1i("useCustomColour", 0);
			PopModel();
		}
	}
}

bool Scene::IsColliding(const Entity* const& actor, const Entity* const& actee) const{
	switch(actee->type){
		case Entity::EntityType::Ball: {
			const glm::vec3& relativeVel = actor->vel - actee->vel;
			const glm::vec3& displacementVec = actee->pos - actor->pos;
			return glm::dot(relativeVel, displacementVec) > 0.f && glm::dot(-displacementVec, -displacementVec) <= (actor->scale.x + actee->scale.x) * (actor->scale.y + actee->scale.y);
		}
		case Entity::EntityType::Diode: {
			const float& temp1 = glm::dot(actor->pos - actee->pos, actee->collisionNormal);
			const float& temp2 = glm::dot(actor->vel, actee->collisionNormal);
			if((temp1 > 0.f && temp2 > 0.f) || temp1 < 0.f){
				return false;
			}
			return (fabs(glm::dot(actee->pos - actor->pos, actee->collisionNormal)) < actor->scale.x + actee->scale.x) &&
				(fabs(glm::dot(actee->pos - actor->pos, glm::vec3(-actee->collisionNormal.y, actee->collisionNormal.x, actee->collisionNormal.z))) < actee->scale.y);
		}
		case Entity::EntityType::WallHidden:
		case Entity::EntityType::Wall: {
			const glm::vec3 displacement = actee->pos - actor->pos;
			glm::vec3 N = actee->collisionNormal;
			if(glm::dot(displacement, N) < 0.f){
				N = -N;
			}
			const glm::vec3 NP(-N.y, N.x, N.z);
			return glm::dot(actor->vel, N) > 0.f && glm::dot(displacement, N) < actor->scale.x + actee->scale.x && fabs(glm::dot(displacement, NP)) < actee->scale.y;
		}
		case Entity::EntityType::Coin:
		case Entity::EntityType::PillarHidden1:
		case Entity::EntityType::PillarHidden2:
		case Entity::EntityType::PillarHidden3:
		case Entity::EntityType::PillarHidden4:
		case Entity::EntityType::Pillar:
			return (glm::dot(actee->pos - actor->pos, actor->vel) > 0.f) && (glm::dot(actee->pos - actor->pos, actee->pos - actor->pos) < (actor->scale.x + actee->scale.x) * (actor->scale.y + actee->scale.y));
	}
	return false;
}

void Scene::ResolveCollision(Entity* const& actor, Entity* const& actee){
	float COR;
	switch(actee->type){
		case Entity::EntityType::Ball: {
			const float& m1 = actor->mass;
			const float& m2 = actee->mass;
			const glm::vec3& u1 = actor->vel;
			const glm::vec3& u2 = actee->vel;

			const glm::vec3 relativePos = actor->pos - actee->pos;
			const glm::vec3 myVec = glm::dot(u1 - u2, relativePos) / glm::dot(relativePos, relativePos) * relativePos;
			COR = .7f;
			actor->vel = COR * (u1 - (2 * m2 / (m1 + m2)) * myVec);
			actee->vel = COR * (u2 - (2 * m1 / (m2 + m1)) * -myVec);
			break;
		}
		case Entity::EntityType::Diode: {
			COR = 1.f;
			glm::vec3 normal = actee->collisionNormal;
			if(glm::dot(normal, actee->pos - actor->pos) < 0.f){
				normal = -normal;
			}
			actor->vel = COR * (actor->vel - glm::dot(2.f * actor->vel, actee->collisionNormal) * actee->collisionNormal);
			break;
		}
		case Entity::EntityType::WallHidden:
		case Entity::EntityType::Wall: {
			if(actee->type == Entity::EntityType::WallHidden || (actee->type == Entity::EntityType::Wall && actee->diffuseTexIndex == 2)){
				bool found;

				found = false;
				const size_t& mySize = changeableThickWalls.size();
				for(size_t i = 0; i < mySize; ++i){
					const size_t& myOtherSize = changeableThickWalls[i].size();
					for(size_t j = 0; j < myOtherSize; ++j){
						if(actee == changeableThickWalls[i][j]){
							found = true;
							break;
						}
					}
					if(found){
						if(changeableThickWalls[i][0]->colour == actor->colour){
							++score;
						} else if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
							--score;
							changeableThickWalls[i][0]->colour = actor->colour;
						}
						break;
					}
				}

				found = false;
				const size_t& mySize2 = curvedWall.size();
				for(size_t i = 0; i < mySize2; ++i){
					const size_t& myOtherSize2 = curvedWall[i].size();
					for(size_t j = 0; j < myOtherSize2; ++j){
						if(actee == curvedWall[i][j]){
							found = true;
							break;
						}
					}
					if(found){
						break;
					}
				}
				if(found){
					if(curvedWall[0][0]->colour == actor->colour){
						++score;
					} else{
						if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
							--score;
						}
						for(size_t i = 0; i < mySize2; ++i){
							const size_t& myOtherSize2 = curvedWall[i].size();
							for(size_t j = 0; j < myOtherSize2; ++j){
								if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
									curvedWall[i][j]->colour = actor->colour;
								}
							}
						}
					}
				}

				found = false;
				const size_t& mySize3 = cross.size();
				for(size_t i = 0; i < mySize3; ++i){
					const size_t& myOtherSize3 = cross[i].size();
					for(size_t j = 0; j < myOtherSize3; ++j){
						if(actee == cross[i][j]){
							found = true;
							break;
						}
					}
					if(found){
						break;
					}
				}
				if(found){
					if(cross[0][0]->colour == actor->colour){
						++score;
					} else{
						if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
							--score;
						}
						for(size_t i = 0; i < mySize3; ++i){
							const size_t& myOtherSize3 = cross[i].size();
							for(size_t j = 0; j < myOtherSize3; ++j){
								if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
									cross[i][j]->colour = actor->colour;
								}
							}
						}
					}
				}
			}

			if(actee->type != Entity::EntityType::WallHidden){
				bool found = false;
				const size_t& mySize = polygons.size();
				for(size_t i = 0; i < mySize; ++i){
					const size_t& myOtherSize = polygons[i].size();
					for(size_t j = 0; j < myOtherSize; ++j){
						if(actee == polygons[i][j]){
							found = true;
							break;
						}
					}
					if(found){
						if(polygons[i][0]->colour == actor->colour){
							++score;
						} else{
							if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
								--score;
							}
							const size_t& myOtherSize2 = polygons[i].size();
							for(size_t j = 0; j < myOtherSize2; ++j){
								if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
									polygons[i][j]->colour = actor->colour;
								}
							}
						}
						break;
					}
				}
			}

			if(!actee->diffuseTexIndex){
				soundEngine->play2D("Audio/Sounds/Wood.wav", false);
			} else{
				soundEngine->play2D("Audio/Sounds/BumpA.wav", false);
			}

			if(actee->colour == glm::vec4(1.f, 0.f, 0.f, .7f) || actee->colour == glm::vec4(0.f, 0.f, 1.f, .7f)){ //If part of flipper...
				actor->colour = glm::vec4(glm::vec3(actee->colour) * 5.f, 1.f);
				if(actor->light){
					static_cast<Spotlight*>(actor->light)->diffuse = glm::vec3(actee->colour);
				}
				score += 2;
			}

			COR = (glm::length(actor->vel) > 50.f ? .7f : 1.f);
			const glm::vec3 displacement = actee->pos - actor->pos;
			glm::vec3 N = actee->collisionNormal;
			if(glm::dot(displacement, N) < 0.f){
				N = -N;
			}
			const glm::vec3 NP(-N.y, N.x, N.z);
			float momentArm = glm::length(actor->pos - (actee->pos + actee->scale.y * NP) - (actor->scale.x + actee->scale.x) * actee->collisionNormal) / 10.f;

			if(glm::dot(actor->vel, N) <= 0.f){
				actor->vel = COR * (actor->vel + fabs(actee->angularVel) * momentArm * -N);
			} else if(glm::dot(actor->vel, actee->collisionNormal) <= 0.f){
				actor->vel = COR * (actor->vel - glm::dot(2.f * actor->vel, N) * N + fabs(actee->angularVel) * momentArm * -N);
			} else{
				actor->vel = COR * (actor->vel - glm::dot(2.f * actor->vel, N) * N - fabs(actee->angularVel) * momentArm * -N);
			}

			break;
		}
		case Entity::EntityType::PillarHidden1:
		case Entity::EntityType::PillarHidden2:
		case Entity::EntityType::PillarHidden3:
		case Entity::EntityType::PillarHidden4:
		case Entity::EntityType::Pillar: {
			if(portal && actee == portal){
				score -= 50;
				soundEngine->play2D("Audio/Sounds/Teleport.wav", false);
				for(size_t j = 0; j < activeBalls.size(); ++j){
					Entity* const& ball = activeBalls[j];
					if(actor == ball){
						for(size_t k = 0; k < spotlights.size(); ++k){
							if(actor->light && actor->light == spotlights[k]){
								delete spotlights[k];
								spotlights.erase(spotlights.begin() + k);
								break;
							}
						}
						ball->active = false;
						activeBalls.erase(activeBalls.begin() + j);
						break;
					}
				}
				return;
			}

			if(actee->type == Entity::EntityType::Pillar){
				if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
					actee->colour = actor->colour;
				}

				bool found = false;
				const size_t& mySize = polygons.size();
				for(size_t i = 0; i < mySize; ++i){
					const size_t& myOtherSize = polygons[i].size();
					for(size_t j = 0; j < myOtherSize; ++j){
						if(actee == polygons[i][j]){
							found = true;
							break;
						}
					}
					if(found){
						if(polygons[i][0]->colour == actor->colour){
							++score;
						} else{
							if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
								--score;
							}
							const size_t& myOtherSize2 = polygons[i].size();
							for(size_t j = 0; j < myOtherSize2; ++j){
								if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
									polygons[i][j]->colour = actor->colour;
								}
							}
						}
						break;
					}
				}
			} else{
				bool found;

				found = false;
				const size_t& mySize = changeableThickWalls.size();
				for(size_t i = 0; i < mySize; ++i){
					const size_t& myOtherSize = changeableThickWalls[i].size();
					for(size_t j = 0; j < myOtherSize; ++j){
						if(actee == changeableThickWalls[i][j]){
							found = true;
							break;
						}
					}
					if(found){
						if(changeableThickWalls[i][0]->colour == actor->colour){
							++score;
						} else if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
							--score;
							changeableThickWalls[i][0]->colour = actor->colour;
						}
						break;
					}
				}

				found = false;
				const size_t& mySize2 = curvedWall.size();
				for(size_t i = 0; i < mySize2; ++i){
					const size_t& myOtherSize2 = curvedWall[i].size();
					for(size_t j = 0; j < myOtherSize2; ++j){
						if(actee == curvedWall[i][j]){
							found = true;
							break;
						}
					}
					if(found){
						break;
					}
				}
				if(found){
					if(curvedWall[0][0]->colour == actor->colour){
						++score;
					} else{
						if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
							--score;
						}
						for(size_t i = 0; i < mySize2; ++i){
							const size_t& myOtherSize2 = curvedWall[i].size();
							for(size_t j = 0; j < myOtherSize2; ++j){
								if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
									curvedWall[i][j]->colour = actor->colour;
								}
							}
						}
					}
				}

				found = false;
				const size_t& mySize3 = cross.size();
				for(size_t i = 0; i < mySize3; ++i){
					const size_t& myOtherSize3 = cross[i].size();
					for(size_t j = 0; j < myOtherSize3; ++j){
						if(actee == cross[i][j]){
							found = true;
							break;
						}
					}
					if(found){
						break;
					}
				}
				if(found){
					if(cross[0][0]->colour == actor->colour){
						++score;
					} else{
						if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
							--score;
						}
						for(size_t i = 0; i < mySize3; ++i){
							const size_t& myOtherSize3 = cross[i].size();
							for(size_t j = 0; j < myOtherSize3; ++j){
								if(actor->colour != glm::vec4(glm::vec3(.5f), 1.f)){
									cross[i][j]->colour = actor->colour;
								}
							}
						}
					}
				}
			}

			soundEngine->play2D("Audio/Sounds/BumpB.wav", false);
			if(actee->colour == glm::vec4(1.f, 0.f, 0.f, .7f) || actee->colour == glm::vec4(0.f, 0.f, 1.f, .7f)){ //If part of flipper...
				actor->colour = glm::vec4(glm::vec3(actee->colour) * 5.f, 1.f);
				if(actor->light){
					static_cast<Spotlight*>(actor->light)->diffuse = glm::vec3(actee->colour);
				}
				score += 2;
			}

			COR = (glm::length(actor->vel) > 50.f ? .7f : 1.f);
			glm::vec3 displacement = actee->pos - actor->pos;
			glm::vec3 N = glm::length(actor->pos - actee->pos) ? glm::normalize(actor->pos - actee->pos) : glm::vec3(0.f, 1.f, 0.f);
			if(glm::dot(displacement, N) < 0.f){
				N = -N;
			}
			glm::vec3 NP(-N.y, N.x, N.z);
			float momentArm = glm::length(actor->pos - (actee->pos + actee->scale.y * NP) - (actor->scale.x + actee->scale.x) * actee->collisionNormal) / 10.f;

			if(glm::dot(actor->vel, N) <= 0.f){
				actor->vel = COR * (actor->vel + fabs(actee->angularVel) * momentArm * -N);
			} else if(glm::dot(actor->vel, actee->collisionNormal) <= 0.f){
				actor->vel = COR * (actor->vel - glm::dot(2.f * actor->vel, N) * N + fabs(actee->angularVel) * momentArm * -N);
			} else{
				actor->vel = COR * (actor->vel - glm::dot(2.f * actor->vel, N) * N - fabs(actee->angularVel) * momentArm * -N);
			}

			break;
		}
		case Entity::EntityType::Coin:
			soundEngine->play2D("Audio/Sounds/Collect.wav", false);
			actee->active = false;
			score += 200;
			break;
	}
}

void Scene::Update(){
	elapsedTime += dt;
	if(winHeight){ //Avoid division by 0 when win is minimised
		cam.SetDefaultAspectRatio(float(winWidth) / float(winHeight));
		cam.ResetAspectRatio();
	}

	GLint polyMode;
	glGetIntegerv(GL_POLYGON_MODE, &polyMode);
	if(Key(KEY_2) && polyModeBT <= elapsedTime){
		glPolygonMode(GL_FRONT_AND_BACK, polyMode + (polyMode == GL_FILL ? -2 : 1));
		polyModeBT = elapsedTime + .5f;
	}

	POINT mousePos;
	if(GetCursorPos(&mousePos)){
		HWND hwnd = ::GetActiveWindow();
		(void)ScreenToClient(hwnd, &mousePos);
	} else{
		(void)puts("Failed to get mouse pos relative to screen!");
	}

	switch(screen){
		case Screen::MainMenu:
		case Screen::GameOver: {
			cam.SetPos(glm::vec3(0.f, 0.f, 130.f));
			cam.SetTarget(glm::vec3(0.f));
			cam.SetUp(glm::vec3(0.f, 1.f, 0.f));
			view = cam.LookAt();
			projection = glm::ortho(-float(winWidth) / 2.f, float(winWidth) / 2.f, -float(winHeight) / 2.f, float(winHeight) / 2.f, .1f, 9999.f);

			if(mousePos.x >= 25.f && mousePos.x <= (screen == Screen::MainMenu ? 100.f : 230.f) && mousePos.y >= winHeight - 160.f && mousePos.y <= winHeight - 125.f){
				if(textScaleFactors[0] != 1.1f){
					soundEngine->play2D("Audio/Sounds/Pop.flac", false);
					textScaleFactors[0] = 1.1f;
					textColours[0] = glm::vec4(1.f, 1.f, 0.f, 1.f);
				}
				if(leftRightMB > 0.f && buttonBT <= elapsedTime){
					soundEngine->play2D("Audio/Sounds/Select.wav", false);
					screen = Screen::Game;

					size_t i;
					const size_t& mySize = entityPool.size();
					for(i = 0; i < mySize; ++i){
						entityPool[i]->active = false;
					}
					const size_t& myOtherSize = spotlights.size();
					for(i = 0; i < myOtherSize; ++i){
						delete spotlights[i];
						spotlights[i] = nullptr;
					}
					spotlights.clear();
					activeBalls.clear();

					Entity* ball = &CreateBall({
						glm::vec3(48.f, -48.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(1.f),
						glm::vec4(glm::vec3(.5f), 1.f),
						0,
						0.f,
					});
					Light* light = CreateLight(LightType::Spot);
					static_cast<Spotlight*>(light)->diffuse = glm::vec3(5.f);
					static_cast<Spotlight*>(light)->spec = glm::vec3(5.f);
					static_cast<Spotlight*>(light)->cosInnerCutoff = glm::cos(glm::radians(30.f));
					static_cast<Spotlight*>(light)->cosOuterCutoff = glm::cos(glm::radians(30.f));
					static_cast<Spotlight*>(light)->pos = glm::vec3(ball->pos.x, ball->pos.y, 5.f);
					static_cast<Spotlight*>(light)->dir = glm::vec3(0.f, 0.f, -1.f);
					ball->light = light;
					spotlights.emplace_back(light);
					activeBalls.emplace_back(ball);
					spawnEmpty = false;

					(void)CreateCoin({
						glm::vec3(2.f, 40.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(4.f, 4.f, 3.f),
						glm::vec4(glm::vec3(4.f), .99f),
						0,
						0.f,
					});
					(void)CreateCoin({
						glm::vec3(-17.f, 40.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(4.f, 4.f, 3.f),
						glm::vec4(glm::vec3(4.f), .99f),
						0,
						0.f,
					});
					(void)CreateCoin({
						glm::vec3(-40.f, -1.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(4.f, 4.f, 3.f),
						glm::vec4(glm::vec3(4.f), .99f),
						0,
						0.f,
					});
					(void)CreateCoin({
						glm::vec3(-42.f, 42.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(4.f, 4.f, 3.f),
						glm::vec4(glm::vec3(4.f), .99f),
						0,
						0.f,
					});
					(void)CreateCoin({
						glm::vec3(20.f, 31.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(4.f, 4.f, 3.f),
						glm::vec4(glm::vec3(4.f), .99f),
						0,
						0.f,
					});

					///Diode
					(void)CreateDiode({
						glm::vec3(48.f, 29.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(1.f, 1.f, 3.f),
						glm::vec4(5.f, 5.f, 0.f, .3f),
						-1,
						0.f,
					});

					///Flipper0
					flipper0 = CreateThickWall({
						pos0,
						facingDir0,
						facingDir0,
						scale,
						glm::vec4(1.f, 0.f, 0.f, .7f),
						-1,
						0.f,
					});
					light = CreateLight(LightType::Pt);
					flipper0[0]->light = light;
					static_cast<PtLight*>(light)->pos = pos0;
					static_cast<PtLight*>(light)->diffuse = glm::vec3(5.f, 0.f, 0.f);
					ptLights.emplace_back(light);

					///Flipper1
					flipper1 = CreateThickWall({
						pos1,
						facingDir1,
						facingDir1,
						scale,
						glm::vec4(0.f, 0.f, 1.f, .7f),
						-1,
						0.f,
					});
					light = CreateLight(LightType::Pt);
					flipper1[0]->light = light;
					static_cast<PtLight*>(light)->pos = pos1;
					static_cast<PtLight*>(light)->diffuse = glm::vec3(0.f, 0.f, 5.f);
					ptLights.emplace_back(light);

					(void)CreateThinWall({
						glm::vec3(0.f, 50.f, 0.f),
						glm::vec3(0.f, -1.f, 0.f),
						glm::vec3(0.f, -1.f, 0.f),
						glm::vec3(1.f, 50.f, 5.f),
						glm::vec4(glm::vec3(1.2f), 1.f),
						0,
						0.f,
					});
					(void)CreateThinWall({
						glm::vec3(0.f, -50.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(1.f, 50.f, 5.f),
						glm::vec4(glm::vec3(1.2f), 1.f),
						0,
						0.f,
					});
					(void)CreateThinWall({
						glm::vec3(-50.f, 0.f, 0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(1.f, 50.f, 5.f),
						glm::vec4(glm::vec3(1.2f), 1.f),
						0,
						0.f,
					});
					(void)CreateThinWall({
						glm::vec3(50.f, 0.f, 0.f),
						glm::vec3(-1.f, 0.f, 0.f),
						glm::vec3(-1.f, 0.f, 0.f),
						glm::vec3(1.f, 50.f, 5.f),
						glm::vec4(glm::vec3(1.2f), 1.f),
						0,
						0.f,
					});

					(void)CreateThickWall({
						glm::vec3(46.f, -10.f, 0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(1.f, 40.f, 3.f),
						glm::vec4(glm::vec3(.6f), 1.f),
						0,
						0.f,
					});
					(void)CreateThinWall({
						glm::vec3(46.f, 46.f, 0.f),
						glm::normalize(glm::vec3(-1.f, -1.f, 0.f)),
						glm::vec3(-1.f, -1.f, 0.f),
						glm::vec3(1.f, 5.f, 3.f),
						glm::vec4(glm::vec3(.6f), 1.f),
						0,
						0.f,
					});
					(void)CreateThinWall({
						glm::vec3(28.f, -25.f, 0.f),
						glm::normalize(glm::vec3(-.2f, 1.f, 0.f)),
						glm::vec3(-.2f, 1.f, 0.f),
						glm::vec3(1.f, 18.f, 3.f),
						glm::vec4(glm::vec3(.6f), 1.f),
						0,
						0.f,
					});
					(void)CreateThinWall({
						glm::vec3(-32.f, -25.f, 0.f),
						glm::normalize(glm::vec3(.2f, 1.f, 0.f)),
						glm::vec3(.2f, 1.f, 0.f),
						glm::vec3(1.f, 18.f, 3.f),
						glm::vec4(glm::vec3(.6f), 1.f),
						0,
						0.f,
					});

					cross.emplace_back(CreateThickWall({
						glm::vec3(-6.f, 25.f, 0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(.5f, 10.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						2,
						-5.f,
					}));
					cross.emplace_back(CreateThickWall({
						glm::vec3(-6.f, 25.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(.5f, 10.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						2,
						-5.f,
					}));

					changeableThickWalls.emplace_back(CreateThickWall({
						glm::vec3(34.f, 37.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(1.f, 5.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						2,
						-8.f,
					}));
					changeableThickWalls.emplace_back(CreateThickWall({
						glm::vec3(-25.f, -1.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(0.f, 1.f, 0.f),
						glm::vec3(1.f, 8.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						2,
						8.f,
					}));
					changeableThickWalls.emplace_back(CreateThickWall({
						glm::vec3(-42.f, 25.f, 0.f),
						glm::normalize(glm::vec3(1.f, -1.f, 0.f)),
						glm::vec3(1.f, -1.f, 0.f),
						glm::vec3(.5f, 4.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						2,
						0.f,
					}));
					changeableThickWalls.emplace_back(CreateThickWall({
						glm::vec3(10.f, 32.f, 0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(.5f, 8.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						2,
						0.f,
					}));

					(void)CreatePillar({
						glm::vec3(-7.f, 43.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(2.f, 2.f, 3.f),
						glm::vec4(glm::vec3(4.f), 1.f),
						0,
						0.f,
					});
					(void)CreatePillar({
						glm::vec3(33.f, 17.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(3.f),
						glm::vec4(glm::vec3(4.f), 1.f),
						0,
						0.f,
					});
					(void)CreatePillar({
						glm::vec3(20.f, 20.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(3.f),
						glm::vec4(glm::vec3(4.f), 1.f),
						0,
						0.f,
					});
					(void)CreatePillar({
						glm::vec3(17.f, 42.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(1.5f, 1.5f, 3.f),
						glm::vec4(glm::vec3(4.f), 1.f),
						0,
						0.f,
					});

					polygons.emplace_back(CreateHollowPolygon(3, 5.f, {
						glm::vec3(-30.f, 36.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 4.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						-1,
						0.f,
					}));
					polygons.emplace_back(CreateHollowPolygon(4, 6.5f, {
						glm::vec3(17.f, 4.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 5.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						-1,
						0.f,
					}));
					polygons.emplace_back(CreateHollowPolygon(5, 3.f, {
						glm::vec3(32.f, -12.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 2.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						-1,
						0.f,
					}));
					polygons.emplace_back(CreateHollowPolygon(10, 4.f, {
						glm::vec3(35.f, 4.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 1.f, 3.f),
						glm::vec4(glm::vec3(2.f), 1.f),
						-1,
						0.f,
					}));

					///Portal
					portal = &CreatePillar({
						glm::vec3(-2.f, -40.f, 0.f),
						glm::vec3(0.f),
						glm::vec3(1.f, 0.f, 0.f),
						glm::vec3(7.f, 7.f, .2f),
						glm::vec4(10.f, 5.f, 0.f, 1.f),
						-1,
						0.f,
					});

					///Curved wall
					const float angularOffset = -10.f;
					for(i = 1; i <= 20; ++i){
						const float angle = glm::radians(angularOffset * float(i)) - 15.f;
						curvedWall.emplace_back(CreateThickWall({
							glm::vec3(-30.f, 12.f, 0.f) + 9.f * glm::vec3(glm::cos(angle), glm::sin(angle), 0.f),
							glm::vec3(glm::cos(angle), glm::sin(angle), 0.f),
							glm::vec3(glm::cos(angle), glm::sin(angle), 0.f),
							glm::vec3(1.f, 2.f, 3.f),
							glm::vec4(glm::vec3(2.f), 1.f),
							2,
							0.f,
						}));
					}

					timeLeft = 60.f;
					score = 0;
					buttonBT = elapsedTime + .3f;
				}
			} else{
				textScaleFactors[0] = 1.f;
				textColours[0] = glm::vec4(1.f);
			}
			if(mousePos.x >= 25.f && mousePos.x <= 230.f && mousePos.y >= winHeight - 110.f && mousePos.y <= winHeight - 75.f){
				if(textScaleFactors[1] != 1.1f){
					soundEngine->play2D("Audio/Sounds/Pop.flac", false);
					textScaleFactors[1] = 1.1f;
					textColours[1] = glm::vec4(1.f, 1.f, 0.f, 1.f);
				}
				if(leftRightMB > 0.f && buttonBT <= elapsedTime){
					soundEngine->play2D("Audio/Sounds/Select.wav", false);
					screen = Screen::Scoreboard;
					buttonBT = elapsedTime + .3f;
				}
			} else{
				textScaleFactors[1] = 1.f;
				textColours[1] = glm::vec4(1.f);
			}
			if(mousePos.x >= 25.f && mousePos.x <= 100.f && mousePos.y >= winHeight - 60.f && mousePos.y <= winHeight - 25.f){
				if(textScaleFactors[2] != 1.1f){
					soundEngine->play2D("Audio/Sounds/Pop.flac", false);
					textScaleFactors[2] = 1.1f;
					textColours[2] = glm::vec4(1.f, 1.f, 0.f, 1.f);
				}
				if(leftRightMB > 0.f && buttonBT <= elapsedTime){
					soundEngine->play2D("Audio/Sounds/Select.wav", false);
					endLoop = true;
					buttonBT = elapsedTime + .3f;
				}
			} else{
				textScaleFactors[2] = 1.f;
				textColours[2] = glm::vec4(1.f);
			}

			break;
		}
		case Screen::Game: {
			timeLeft -= dt;
			if(timeLeft <= 0.f){
				screen = Screen::GameOver;
				const size_t& mySize = scores.size();
				if(mySize == 5){ //Max no. of scores saved
					std::sort(scores.begin(), scores.end(), std::greater<int>());
					if(score > scores.back()){
						scores.pop_back();
						scores.emplace_back(score);
					}
					std::sort(scores.begin(), scores.end(), std::greater<int>());
				} else{
					scores.emplace_back(score);
					std::sort(scores.begin(), scores.end(), std::greater<int>());
				}
			}

			#ifdef MOVE_CAM
				cam.Update(GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_W, GLFW_KEY_S);
			#else
				cam.SetPos(glm::vec3(0.f, -80.f, 150.f));
				cam.SetTarget(glm::vec3(0.f, -1.f, 0.f));
				cam.SetUp(glm::vec3(0.f, 1.f, 0.f));
			#endif
			view = cam.LookAt();
			projection = glm::perspective(glm::radians(angularFOV), cam.GetAspectRatio(), .1f, 9999.f);

			static_cast<SpriteAni*>(meshes[(int)GeoType::CoinSpriteAni])->Update();
			UpdateEntities();
			if(score < 0){
				score = 0;
			}

			break;
		}
		case Screen::Scoreboard:
			cam.SetPos(glm::vec3(0.f, 0.f, 130.f));
			cam.SetTarget(glm::vec3(0.f));
			cam.SetUp(glm::vec3(0.f, 1.f, 0.f));
			view = cam.LookAt();
			projection = glm::perspective(glm::radians(angularFOV), cam.GetAspectRatio(), .1f, 9999.f);
			if(mousePos.x >= 25.f && mousePos.x <= 110.f && mousePos.y >= winHeight - 60.f && mousePos.y <= winHeight - 25.f){
				if(textScaleFactors[2] != 1.1f){
					soundEngine->play2D("Audio/Sounds/Pop.flac", false);
					textScaleFactors[2] = 1.1f;
					textColours[2] = glm::vec4(1.f, 1.f, 0.f, 1.f);
				}
				if(leftRightMB > 0.f && buttonBT <= elapsedTime){
					soundEngine->play2D("Audio/Sounds/Select.wav", false);
					screen = Screen::MainMenu;
					buttonBT = elapsedTime + .3f;
				}
			} else{
				textScaleFactors[2] = 1.f;
				textColours[2] = glm::vec4(1.f);
			}
			break;
		default:
			view = cam.LookAt();
			projection = glm::perspective(glm::radians(angularFOV), cam.GetAspectRatio(), .1f, 9999.f);

			if(soundFX){
				if(Key(KEY_I) && distortionBT <= elapsedTime){
					soundFX->isDistortionSoundEffectEnabled() ? soundFX->disableDistortionSoundEffect() : (void)soundFX->enableDistortionSoundEffect();
					distortionBT = elapsedTime + .5f;
				}
				if(Key(KEY_O) && echoBT <= elapsedTime){
					soundFX->isEchoSoundEffectEnabled() ? soundFX->disableEchoSoundEffect() : (void)soundFX->enableEchoSoundEffect();
					echoBT = elapsedTime + .5f;
				}
				if(Key(KEY_P) && wavesReverbBT <= elapsedTime){
					soundFX->isWavesReverbSoundEffectEnabled() ? soundFX->disableWavesReverbSoundEffect() : (void)soundFX->enableWavesReverbSoundEffect();
					wavesReverbBT = elapsedTime + .5f;
				}
				if(Key(KEY_L) && resetSoundFXBT <= elapsedTime){
					soundFX->disableAllEffects();
					resetSoundFXBT = elapsedTime + .5f;
				}
			}
	}
	const glm::vec3& camPos = cam.GetPos();
	const glm::vec3& camFront = cam.CalcFront();
	soundEngine->setListenerPosition(vec3df(camPos.x, camPos.y, camPos.z), vec3df(camFront.x, camFront.y, camFront.z));
}

void Scene::GeoRenderPass(){
	geoPassSP.Use();
	geoPassSP.SetMat4fv("PV", &(projection * view)[0][0]);

	switch(screen){
		case Screen::Scoreboard:
			geoPassSP.Set1i("useCustomDiffuseTexIndex", 1);
			geoPassSP.Set1i("customDiffuseTexIndex", 1);
		case Screen::MainMenu:
		case Screen::GameOver:
			PushModel({
				Scale(glm::vec3(float(winWidth) / 2.f, float(winHeight) / 2.f, 1.f)),
			});
			meshes[(int)GeoType::Quad]->SetModel(GetTopModel());
			meshes[(int)GeoType::Quad]->Render(geoPassSP);
			PopModel();
			geoPassSP.Set1i("useCustomDiffuseTexIndex", 0);
			break;
		case Screen::Game:
			PushModel({
				Translate(glm::vec3(0.f, 0.f, -5.f)),
				Rotate(glm::vec4(0.f, 1.f, 0.f, 0.f)),
				Scale(glm::vec3(50.f, 50.f, 1.f)),
			});
				geoPassSP.Set1i("useCustomColour", 1);
				geoPassSP.Set4fv("customColour", glm::vec4(glm::vec3(1.f), .3f));
				geoPassSP.Set1i("useCustomDiffuseTexIndex", 1);
				geoPassSP.Set1i("customDiffuseTexIndex", 1);
				meshes[(int)GeoType::Cube]->SetModel(GetTopModel());
				meshes[(int)GeoType::Cube]->Render(geoPassSP);
				geoPassSP.Set1i("useCustomDiffuseTexIndex", 0);
				geoPassSP.Set1i("useCustomColour", 0);
			PopModel();
			RenderEntities(geoPassSP, true);
			break;
	}

	if(music){
		music->setIsPaused(false);
	}
}

Entity& Scene::CreateBall(const EntityCreationAttribs& attribs){
	Entity* const& entity = FetchEntity();
	entity->type = Entity::EntityType::Ball;
	entity->active = true;
	entity->life = 0.f;
	entity->maxLife = 0.f;
	entity->colour = attribs.colour;
	entity->diffuseTexIndex = attribs.diffuseTexIndex;
	entity->scale = attribs.scale;
	entity->collisionNormal = attribs.collisionNormal;

	entity->pos = attribs.pos;
	entity->vel = glm::vec3(0.f);
	entity->mass = 5.f;
	entity->force = glm::vec3(0.f);

	entity->facingDir = attribs.facingDir;
	entity->angularVel = attribs.angularVel;
	entity->angularMass = .4f * entity->mass * entity->scale.x * entity->scale.y;
	entity->torque = glm::vec3(0.f);

	return *entity;
}

Entity& Scene::CreateDiode(const EntityCreationAttribs& attribs){
	Entity* const& entity = FetchEntity();
	entity->type = Entity::EntityType::Diode;
	entity->active = true;
	entity->life = 0.f;
	entity->maxLife = 0.f;
	entity->colour = attribs.colour;
	entity->diffuseTexIndex = attribs.diffuseTexIndex;
	entity->scale = attribs.scale;
	entity->collisionNormal = attribs.collisionNormal;

	entity->pos = attribs.pos;
	entity->vel = glm::vec3(0.f);
	entity->mass = 5.f;
	entity->force = glm::vec3(0.f);

	entity->facingDir = attribs.facingDir;
	entity->angularVel = attribs.angularVel;
	entity->angularMass = entity->mass / 12.f * (entity->scale.x * entity->scale.x + entity->scale.y * entity->scale.y);
	entity->torque = glm::vec3(0.f);

	return *entity;
}

Entity& Scene::CreateThinWall(const EntityCreationAttribs& attribs){
	Entity* const& entity = FetchEntity();
	entity->type = attribs.type ? Entity::EntityType::WallHidden : Entity::EntityType::Wall;
	entity->active = true;
	entity->life = 0.f;
	entity->maxLife = 0.f;
	entity->colour = attribs.colour;
	entity->diffuseTexIndex = attribs.diffuseTexIndex;
	entity->scale = attribs.scale;
	entity->collisionNormal = attribs.collisionNormal;

	entity->pos = attribs.pos;
	entity->vel = glm::vec3(0.f);
	entity->mass = 5.f;
	entity->force = glm::vec3(0.f);

	entity->facingDir = attribs.facingDir;
	entity->angularVel = attribs.angularVel;
	entity->angularMass = entity->mass / 12.f * (entity->scale.x * entity->scale.x + entity->scale.y * entity->scale.y);
	entity->torque = glm::vec3(0.f);

	return *entity;
}

Entity& Scene::CreatePillar(const EntityCreationAttribs& attribs){
	Entity* const& entity = FetchEntity();
	switch(attribs.type){
		case 0:
			entity->type = Entity::EntityType::Pillar;
			break;
		case 1:
			entity->type = Entity::EntityType::PillarHidden1;
			break;
		case 2:
			entity->type = Entity::EntityType::PillarHidden2;
			break;
		case 3:
			entity->type = Entity::EntityType::PillarHidden3;
			break;
		case 4:
			entity->type = Entity::EntityType::PillarHidden4;
			break;
	}
	entity->active = true;
	entity->life = 0.f;
	entity->maxLife = 0.f;
	entity->colour = attribs.colour;
	entity->diffuseTexIndex = attribs.diffuseTexIndex;
	entity->scale = attribs.scale;
	entity->collisionNormal = attribs.collisionNormal;

	entity->pos = attribs.pos;
	entity->vel = glm::vec3(0.f);
	entity->mass = 5.f;
	entity->force = glm::vec3(0.f);

	entity->facingDir = attribs.facingDir;
	entity->angularVel = attribs.angularVel;
	entity->angularMass = .5f * entity->mass * entity->scale.x * entity->scale.y;
	entity->torque = glm::vec3(0.f);

	return *entity;
}

Entity& Scene::CreateCoin(const EntityCreationAttribs& attribs){
	Entity* const& entity = FetchEntity();
	entity->type = Entity::EntityType::Coin;
	entity->active = true;
	entity->life = 0.f;
	entity->maxLife = 0.f;
	entity->colour = attribs.colour;
	entity->diffuseTexIndex = attribs.diffuseTexIndex;
	entity->scale = attribs.scale;
	entity->collisionNormal = attribs.collisionNormal;

	entity->pos = attribs.pos;
	entity->vel = glm::vec3(0.f);
	entity->mass = 5.f;
	entity->force = glm::vec3(0.f);

	entity->facingDir = attribs.facingDir;
	entity->angularVel = attribs.angularVel;
	entity->angularMass = .5f * entity->mass * entity->scale.x * entity->scale.y;
	entity->torque = glm::vec3(0.f);

	return *entity;
}

std::vector<Entity*> Scene::CreateThickWall(const EntityCreationAttribs& attribs){
	std::vector<Entity*> entities;
	const glm::vec3 NP = glm::vec3(-attribs.collisionNormal.y, attribs.collisionNormal.x, attribs.collisionNormal.z);
	entities.emplace_back(&CreateThinWall({
		attribs.pos,
		attribs.collisionNormal,
		attribs.facingDir,
		attribs.scale,
		attribs.colour,
		attribs.diffuseTexIndex,
		attribs.angularVel,
	}));
	entities.emplace_back(&CreateThinWall({
		attribs.pos,
		NP,
		glm::vec3(-attribs.facingDir.y, attribs.facingDir.x, attribs.facingDir.z),
		glm::vec3(attribs.scale.y, attribs.scale.x, attribs.scale.z),
		attribs.colour,
		-1,
		attribs.angularVel,
		1,
	}));
	entities.emplace_back(&CreatePillar({
		attribs.pos + NP * attribs.scale.y + attribs.collisionNormal * attribs.scale.x,
		glm::vec3(0.f),
		attribs.facingDir,
		glm::vec3(glm::vec2(.005f), attribs.scale.z),
		attribs.colour,
		-1,
		attribs.angularVel,
		1,
	}));
	entities.emplace_back(&CreatePillar({
		attribs.pos + -NP * attribs.scale.y + -attribs.collisionNormal * attribs.scale.x,
		glm::vec3(0.f),
		attribs.facingDir,
		glm::vec3(glm::vec2(.005f), attribs.scale.z),
		attribs.colour,
		-1,
		attribs.angularVel,
		2,
	}));
	entities.emplace_back(&CreatePillar({
		attribs.pos + -NP * attribs.scale.y + attribs.collisionNormal * attribs.scale.x,
		glm::vec3(0.f),
		attribs.facingDir,
		glm::vec3(glm::vec2(.005f), attribs.scale.z),
		attribs.colour,
		-1,
		attribs.angularVel,
		3,
	}));
	entities.emplace_back(&CreatePillar({
		attribs.pos + NP * attribs.scale.y + -attribs.collisionNormal * attribs.scale.x,
		glm::vec3(0.f),
		attribs.facingDir,
		glm::vec3(glm::vec2(.005f), attribs.scale.z),
		attribs.colour,
		-1,
		attribs.angularVel,
		4,
	}));
	return entities;
}

std::vector<Entity*> Scene::CreateHollowPolygon(const int& sidesAmt, const float& len, const EntityCreationAttribs& attribs){
	std::vector<Entity*> entities;
	const float angle = 360.f / float(sidesAmt);
	const float radius = len * cosf(glm::radians(angle * .5f));

	for(int i = 0; i < sidesAmt; ++i){
		entities.emplace_back(&CreateThinWall({
			attribs.pos + glm::vec3(radius * cosf(glm::radians(i * angle)), radius * sinf(glm::radians(i * angle)), 0.f),
			glm::vec3(cosf(glm::radians(i * angle)), sinf(glm::radians(i * angle)), 0.f),
			glm::vec3(cosf(glm::radians(i * angle)), sinf(glm::radians(i * angle)), 0.f),
			attribs.scale,
			attribs.colour,
			attribs.diffuseTexIndex,
			attribs.angularVel,
		}));
		entities.emplace_back(&CreatePillar({
			attribs.pos + glm::vec3(len * cosf(glm::radians(i * angle + 180.f / float(sidesAmt))), len * sinf(glm::radians(i * angle + 180.f / float(sidesAmt))), 0.f),
			glm::vec3(0.f),
			glm::vec3(1.f, 0.f, 0.f),
			glm::vec3(glm::vec2(attribs.scale.x), attribs.scale.z),
			attribs.colour,
			attribs.diffuseTexIndex,
			attribs.angularVel,
		}));
	}

	return entities;
}

Scene::~Scene(){
	///Create save
	str line;
	try{
		std::fstream stream("Data/scores.dat", std::ios::out);
		if(stream.is_open()){
			const size_t& mySize = scores.size();
			for(size_t i = 0; i < mySize; ++i){
				stream << (!i ? "" : "\n") + std::to_string(scores[i]);
			}
			stream.close();
		} else{
			throw("Failed to save scores!");
		}
	} catch(cstr const& errorMsg){
		(void)puts(errorMsg);
	}

	const size_t& pSize = ptLights.size();
	const size_t& dSize = directionalLights.size();
	const size_t& sSize = spotlights.size();
	for(size_t i = 0; i < pSize; ++i){
		delete ptLights[i];
		ptLights[i] = nullptr;
	}
	for(size_t i = 0; i < dSize; ++i){
		delete directionalLights[i];
		directionalLights[i] = nullptr;
	}
	for(size_t i = 0; i < sSize; ++i){
		delete spotlights[i];
		spotlights[i] = nullptr;
	}

	for(Entity*& entity: entityPool){
		if(entity){
			delete entity;
			entity = nullptr;
		}
	}
	for(int i = 0; i < (int)GeoType::Amt; ++i){
		if(meshes[i]){
			delete meshes[i];
			meshes[i] = nullptr;
		}
	}
	if(music){
		music->drop();
	}
	if(soundEngine){
		soundEngine->drop();
	}
}

bool Scene::Init(){
	soundEngine = createIrrKlangDevice(ESOD_AUTO_DETECT, ESEO_MULTI_THREADED | ESEO_LOAD_PLUGINS | ESEO_USE_3D_BUFFERS | ESEO_PRINT_DEBUG_INFO_TO_DEBUGGER);
	if(!soundEngine){
		(void)puts("Failed to init soundEngine!\n");
	}

	music = soundEngine->play3D("Audio/Music/Theme.mp3", vec3df(0.f, 0.f, 0.f), true, true, true, ESM_AUTO_DETECT, true);
	if(music){
		music->setMinDistance(50.f);
		music->setVolume(3);

		soundFX = music->getSoundEffectControl();
		if(!soundFX){
			(void)puts("No soundFX support!\n");
		}
	} else{
		(void)puts("Failed to init music!\n");
	}

	///Load save
	cstr const& fPath = "Data/scores.dat";
	str line;
	std::ifstream stream(fPath, std::ios::in);
	if(stream.is_open()){
		while(getline(stream, line)){
			try{
				scores.emplace_back(stoi(line));
			} catch(const std::invalid_argument& e){
				(void)puts(e.what());
			}
		}
		stream.close();
	}
	if(scores.size() > 1){
		std::sort(scores.begin(), scores.end(), std::greater<int>());
	}

	short i;
	const size_t& mySize = entityPool.size();
	for(i = 0; i < mySize; ++i){
		entityPool[i] = new Entity();
	}

	meshes[(int)GeoType::CoinSpriteAni]->AddTexMap({"Imgs/Coin.png", Mesh::TexType::Diffuse, 0});
	static_cast<SpriteAni*>(meshes[(int)GeoType::CoinSpriteAni])->AddAni("CoinSpriteAni", 0, 6);
	static_cast<SpriteAni*>(meshes[(int)GeoType::CoinSpriteAni])->Play("CoinSpriteAni", -1, .5f);

	///Lights
	for(i = 0; i < 20; ++i){
		Light* light = CreateLight(LightType::Pt);
		if(i < 5){
			static_cast<PtLight*>(light)->pos = glm::vec3(20.f * float(i) - 50.f, 48.f, -1.f);
		} else if(i < 10){
			static_cast<PtLight*>(light)->pos = glm::vec3(20.f * float(i - 5) - 50.f, -48.f, -1.f);
		} else if(i < 15){
			static_cast<PtLight*>(light)->pos = glm::vec3(48.f, 20.f * float(i - 10) - 50.f, -1.f);
		} else if(i < 20){
			static_cast<PtLight*>(light)->pos = glm::vec3(-48.f, 20.f * float(i - 15) - 50.f, -1.f);
		}
		ptLights.emplace_back(light);
	}

	return true;
}

void Scene::LightingRenderPass(const uint& posTexRefID, const uint& coloursTexRefID, const uint& normalsTexRefID, const uint& specTexRefID, const uint& reflectionTexRefID){
	lightingPassSP.Use();

	const int& pAmt = (int)ptLights.size();
	const int& dAmt = (int)directionalLights.size();
	const int& sAmt = (int)spotlights.size();
	lightingPassSP.Set1f("shininess", 32.f); //More light scattering if lower
	lightingPassSP.Set3fv("globalAmbient", Light::globalAmbient);

	lightingPassSP.Set3fv("camPos", cam.GetPos());
	lightingPassSP.UseTex(posTexRefID, "posTex");
	lightingPassSP.UseTex(coloursTexRefID, "coloursTex");
	lightingPassSP.UseTex(normalsTexRefID, "normalsTex");
	lightingPassSP.UseTex(specTexRefID, "specTex");
	lightingPassSP.UseTex(reflectionTexRefID, "reflectionTex");

	lightingPassSP.Set1i("pAmt", pAmt);
	lightingPassSP.Set1i("dAmt", dAmt);
	lightingPassSP.Set1i("sAmt", sAmt);

	if(screen == Screen::Game){
		for(int i = 0; i < pAmt; ++i){
			const PtLight* const& ptLight = static_cast<PtLight*>(ptLights[i]);
			lightingPassSP.Set3fv(("ptLights[" + std::to_string(i) + "].ambient").c_str(), ptLight->ambient);
			lightingPassSP.Set3fv(("ptLights[" + std::to_string(i) + "].diffuse").c_str(), ptLight->diffuse);
			lightingPassSP.Set3fv(("ptLights[" + std::to_string(i) + "].spec").c_str(), ptLight->spec);
			lightingPassSP.Set3fv(("ptLights[" + std::to_string(i) + "].pos").c_str(), ptLight->pos);
			lightingPassSP.Set1f(("ptLights[" + std::to_string(i) + "].constant").c_str(), ptLight->constant);
			lightingPassSP.Set1f(("ptLights[" + std::to_string(i) + "].linear").c_str(), ptLight->linear);
			lightingPassSP.Set1f(("ptLights[" + std::to_string(i) + "].quadratic").c_str(), ptLight->quadratic);
		}
		for(int i = 0; i < dAmt; ++i){
			const DirectionalLight* const& directionalLight = static_cast<DirectionalLight*>(directionalLights[i]);
			lightingPassSP.Set3fv(("directionalLights[" + std::to_string(i) + "].ambient").c_str(), directionalLight->ambient);
			lightingPassSP.Set3fv(("directionalLights[" + std::to_string(i) + "].diffuse").c_str(), directionalLight->diffuse);
			lightingPassSP.Set3fv(("directionalLights[" + std::to_string(i) + "].spec").c_str(), directionalLight->spec);
			lightingPassSP.Set3fv(("directionalLights[" + std::to_string(i) + "].dir").c_str(), directionalLight->dir);
		}
		for(int i = 0; i < sAmt; ++i){
			const Spotlight* const& spotlight = static_cast<Spotlight*>(spotlights[i]);
			lightingPassSP.Set3fv(("spotlights[" + std::to_string(i) + "].ambient").c_str(), spotlight->ambient);
			lightingPassSP.Set3fv(("spotlights[" + std::to_string(i) + "].diffuse").c_str(), spotlight->diffuse);
			lightingPassSP.Set3fv(("spotlights[" + std::to_string(i) + "].spec").c_str(), spotlight->spec);
			lightingPassSP.Set3fv(("spotlights[" + std::to_string(i) + "].pos").c_str(), spotlight->pos);
			lightingPassSP.Set3fv(("spotlights[" + std::to_string(i) + "].dir").c_str(), spotlight->dir);
			lightingPassSP.Set1f(("spotlights[" + std::to_string(i) + "].cosInnerCutoff").c_str(), spotlight->cosInnerCutoff);
			lightingPassSP.Set1f(("spotlights[" + std::to_string(i) + "].cosOuterCutoff").c_str(), spotlight->cosOuterCutoff);
		}
	} else{
		lightingPassSP.Set1i("pAmt", 0);
		lightingPassSP.Set1i("dAmt", 0);
		lightingPassSP.Set1i("sAmt", 0);
	}

	meshes[(int)GeoType::Quad]->SetModel(GetTopModel());
	meshes[(int)GeoType::Quad]->Render(lightingPassSP, false);
	lightingPassSP.ResetTexUnits();
}

void Scene::BlurRender(const uint& brightTexRefID, const bool& horizontal){
	blurSP.Use();
	blurSP.Set1i("horizontal", horizontal);
	blurSP.UseTex(brightTexRefID, "texSampler");
	meshes[(int)GeoType::Quad]->SetModel(GetTopModel());
	meshes[(int)GeoType::Quad]->Render(blurSP, false);
	blurSP.ResetTexUnits();
}

void Scene::DefaultRender(const uint& screenTexRefID, const uint& blurTexRefID){
	screenSP.Use();
	screenSP.Set1f("exposure", .7f - (float(spotlights.size()) * .05f));
	screenSP.UseTex(screenTexRefID, "screenTexSampler");
	screenSP.UseTex(blurTexRefID, "blurTexSampler");
	meshes[(int)GeoType::Quad]->SetModel(GetTopModel());
	meshes[(int)GeoType::Quad]->Render(screenSP, false);
	screenSP.ResetTexUnits();
}

void Scene::ForwardRender(){
	forwardSP.Use();
	forwardSP.Set1i("pAmt", 0);
	forwardSP.Set1i("dAmt", 0);
	forwardSP.Set1i("sAmt", 0);
	forwardSP.Set1f("shininess", 32.f);
	forwardSP.Set3fv("globalAmbient", Light::globalAmbient);
	forwardSP.Set3fv("camPos", cam.GetPos());

	forwardSP.SetMat4fv("PV", &(projection * view)[0][0]);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	switch(screen){
		case Screen::MainMenu:
			glDepthFunc(GL_NOTEQUAL);
			textChief.RenderText(textSP, {
				"Play",
				25.f,
				125.f,
				textScaleFactors[0],
				textColours[0],
				0,
			});
			textChief.RenderText(textSP, {
				"Scoreboard",
				25.f,
				75.f,
				textScaleFactors[1],
				textColours[1],
				0,
			});
			textChief.RenderText(textSP, {
				"Exit",
				25.f,
				25.f,
				textScaleFactors[2],
				textColours[2],
				0,
			});
			textChief.RenderText(textSP, {
				"Pinball Blitz",
				float(winWidth) / 2.f,
				float(winHeight) / 2.f,
				2.f,
				glm::vec4(1.f, .5f, 0.f, 1.f),
				0,
			});
			glDepthFunc(GL_LESS);
			break;
		case Screen::Game:
			RenderEntities(forwardSP, false);

			textChief.RenderText(textSP, {
				"Time Left: " + std::to_string(timeLeft).substr(0, 5),
				25.f,
				25.f,
				1.f,
				glm::vec4(1.f, 1.f, 0.f, 1.f),
				0,
				});
			textChief.RenderText(textSP, {
				"Score: " + std::to_string(score),
				25.f,
				75.f,
				1.f,
				glm::vec4(1.f, 1.f, 0.f, 1.f),
				0,
				});
			textChief.RenderText(textSP, {
				"FPS: " + std::to_string(1.f / dt).substr(0, 2),
				25.f,
				125.f,
				1.f,
				glm::vec4(1.f, 1.f, 0.f, 1.f),
				0,
			});
			break;
		case Screen::GameOver: {
			glDepthFunc(GL_NOTEQUAL);
			textChief.RenderText(textSP, {
				"Play Again",
				25.f,
				125.f,
				textScaleFactors[0],
				textColours[0],
				0,
			});
			textChief.RenderText(textSP, {
				"Scoreboard",
				25.f,
				75.f,
				textScaleFactors[1],
				textColours[1],
				0,
			});
			textChief.RenderText(textSP, {
				"Exit",
				25.f,
				25.f,
				textScaleFactors[2],
				textColours[2],
				0,
			});
			textChief.RenderText(textSP, {
				"Game Over",
				float(winWidth) / 2.f,
				float(winHeight) / 2.f + 100.f,
				2.f,
				glm::vec4(1.f, .5f, 0.f, 1.f),
				0,
			});
			textChief.RenderText(textSP, {
				"Final Score: " + std::to_string(score),
				float(winWidth) / 2.f,
				float(winHeight) / 2.f,
				2.f,
				glm::vec4(1.f, .5f, 0.f, 1.f),
				0,
			});
			if(scores.size() == 1 || (score == scores.front() && score != scores[1])){
				textChief.RenderText(textSP, {
					"New High Score!",
					float(winWidth) / 2.f,
					float(winHeight) / 2.f - 100.f,
					2.f,
					glm::vec4(1.f, .5f, 0.f, 1.f),
					0,
				});
			}
			glDepthFunc(GL_LESS);
			break;
		}
		case Screen::Scoreboard: {
			glDepthFunc(GL_NOTEQUAL);
			textChief.RenderText(textSP, {
				"Back",
				25.f,
				25.f,
				textScaleFactors[2],
				textColours[2],
				0,
			});

			float currOffset = 350.f;
			textChief.RenderText(textSP, {
				"Scoreboard",
				float(winWidth) / 2.f,
				float(winHeight) / 2.f + currOffset,
				1.f,
				glm::vec4(1.f, .5f, 0.f, 1.f),
				0,
			});
			const size_t& mySize = scores.size();
			for(size_t i = 0; i < mySize; ++i){
				currOffset -= 100.f;
				textChief.RenderText(textSP, {
					std::to_string(scores[i]),
					float(winWidth) / 2.f,
					float(winHeight) / 2.f + currOffset,
					1.f,
					glm::vec4(1.f, .5f, 0.f, 1.f),
					0,
				});
			}
			glDepthFunc(GL_LESS);
			break;
		}
	}

	glBlendFunc(GL_ONE, GL_ZERO);
}

Entity* const& Scene::FetchEntity(){
	for(Entity* const& entity: entityPool){
		if(!entity->active){
			return entity;
		}
	}
	entityPool.emplace_back(new Entity());
	(void)puts("1 entity was added to entityPool!\n");
	return entityPool.back();
}

glm::mat4 Scene::Translate(const glm::vec3& translate){
	return glm::translate(glm::mat4(1.f), translate);
}

glm::mat4 Scene::Rotate(const glm::vec4& rotate){
	return glm::rotate(glm::mat4(1.f), glm::radians(rotate.w), glm::vec3(rotate));
}

glm::mat4 Scene::Scale(const glm::vec3& scale){
	return glm::scale(glm::mat4(1.f), scale);
}

glm::mat4 Scene::GetTopModel() const{
	return modelStack.empty() ? glm::mat4(1.f) : modelStack.top();
}

void Scene::PushModel(const std::vector<glm::mat4>& vec) const{
	modelStack.push(modelStack.empty() ? glm::mat4(1.f) : modelStack.top());
	const size_t& size = vec.size();
	for(size_t i = 0; i < size; ++i){
		modelStack.top() *= vec[i];
	}
}

void Scene::PopModel() const{
	if(!modelStack.empty()){
		modelStack.pop();
	}
}