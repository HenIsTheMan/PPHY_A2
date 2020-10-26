#include "SpriteAni.h"
#include "../Engine.h"

///Can be improved

extern float dt;

SpriteAni::SpriteAni(): SpriteAni(0, 0){}

SpriteAni::SpriteAni(const int& rows, const int& cols):
	currTime(0.f),
	currFrame(0),
	rows(rows),
	cols(cols),
	playCount(0),
	currAni(""),
	allAnis({})
{
}

SpriteAni::~SpriteAni(){
	const auto& end = allAnis.end();
	for(auto iter = allAnis.begin(); iter != end; ++iter){
		if(iter->second){
			delete iter->second;
			iter->second = nullptr;
		}
	}
}

void SpriteAni::Play(const str& name, const int& repeat, const float& time){
	if(allAnis[name]){
		currAni = name;
		allAnis[name]->repeatCount = repeat;
		allAnis[name]->time = time;
		allAnis[name]->active = true;
	}
}

void SpriteAni::Resume(){
	allAnis[currAni]->active = true;
}

void SpriteAni::Pause(){
	allAnis[currAni]->active = false;
}

void SpriteAni::Reset(){
	currFrame = allAnis[currAni]->frames[0];
	playCount = 0;
}

void SpriteAni::AddAni(const str& name, const int& start, const int& end){
	Ani* ani = new Ani();
	for(int i = start; i < end; ++i){
		ani->frames.emplace_back(i);
	}
	allAnis[name] = ani; //Link ani to aniList
	if(currAni == ""){ //Set the curr ani if it does not exist
		currAni = name;
	}
	allAnis[name]->active = false;
}

void SpriteAni::AddSequenceAni(const str& name, const ::std::initializer_list<int>& frames){
	Ani* ani = new Ani();
	for(const int& frame: frames){
		ani->frames.emplace_back(frame);
	}
	allAnis[name] = ani; //...
	if(currAni == ""){ //...
		currAni = name;
	}
	allAnis[name]->active = false;
}

void SpriteAni::Create(){
	vertices = new std::vector<Vertex>();
	indices = new std::vector<uint>();

	const float width = 1.f / (float)cols;
	const float height = 1.f / (float)rows;
	int offset = 0;
	short myArr[6]{0, 1, 2, 0, 2, 3};
	std::vector<std::pair<glm::vec3, glm::vec2>> temp;

	for(int i = 0; i < rows; ++i){
		for(int j = 0; j < cols; ++j){
			const float U = j * width;
			const float V = 1.f - height - i * height;
			temp.push_back({glm::vec3(-1.f, -1.f, 0.f), glm::vec2(U, V)});
			temp.push_back({glm::vec3(1.f, -1.f, 0.f), glm::vec2(U + width, V)});
			temp.push_back({glm::vec3(1.f, 1.f, 0.f), glm::vec2(U + width, V + height)});
			temp.push_back({glm::vec3(-1.f, 1.f, 0.f), glm::vec2(U, V + height)});

			for(short k = 0; k < 6; ++k){
				indices->emplace_back(offset + myArr[k]);
			}
			offset += 4;
		}
	}

	const size_t& size = temp.size();
	for(size_t i = 0; i < size; ++i){
		vertices->push_back({
			temp[i].first,
			glm::vec4(.7f, .4f, .1f, 1.f),
			temp[i].second,
			glm::vec3(0.f, 0.f, 1.f),
		});
	}
}

void SpriteAni::Update(){
	if(allAnis[currAni]->active){ //Check if the curr Ani is active
		currTime += dt;
		int numFrame = (int)allAnis[currAni]->frames.size();
		float frameTime = allAnis[currAni]->time / numFrame;

		currFrame = allAnis[currAni]->frames[std::min((int)allAnis[currAni]->frames.size() - 1, int(currTime / frameTime))]; //Set curr frame based on curr time
		if(currTime >= allAnis[currAni]->time){ //If curr time >= total animated time...
			if(playCount < allAnis[currAni]->repeatCount){
				///Increase play count and repeat
				++playCount;
				currTime = 0;
				currFrame = allAnis[currAni]->frames[0];
			} else{ //If repeat count is 0 || play count == repeat count...
				allAnis[currAni]->active = false;
				allAnis[currAni]->ended = true;
			}
			if(allAnis[currAni]->repeatCount == -1){ //If ani is infinite...
				currTime = 0.f;
				currFrame = allAnis[currAni]->frames[0];
				allAnis[currAni]->active = true;
				allAnis[currAni]->ended = false;
			}
		}
	}
}

void SpriteAni::Render(ShaderProg& SP, const bool& autoConfig){
	if(primitive < 0){
		return (void)puts("Invalid primitive!\n");
	}

	SP.Use();
	SP.SetMat4fv("model", &(model)[0][0]);
	if(autoConfig){
		SP.Set1i("instancing", 0);

		SP.Set1i("useDiffuseMap", 0);
		SP.Set1i("useSpecMap", 0);
		SP.Set1i("useEmissionMap", 0);
		//SP.Set1i("useReflectionMap", 0);
		SP.Set1i("useBumpMap", 0);

		short diffuseCount = 0;
		for(std::tuple<str, TexType, uint>& texMap: texMaps){
			if(!std::get<uint>(texMap)){
				SetUpTex({
					std::get<str>(texMap),
					true,
					GL_TEXTURE_2D,
					GL_REPEAT,
					GL_LINEAR_MIPMAP_LINEAR,
					GL_LINEAR,
				}, std::get<uint>(texMap));
			}

			switch(std::get<TexType>(texMap)){
				case TexType::Diffuse:
					SP.Set1i("useDiffuseMap", 1);
					SP.UseTex(std::get<uint>(texMap), ("diffuseMaps[" + std::to_string(diffuseCount++) + ']').c_str());
					break;
				case TexType::Spec:
					SP.Set1i("useSpecMap", 1);
					SP.UseTex(std::get<uint>(texMap), "specMap");
					break;
				case TexType::Emission:
					SP.Set1i("useEmissionMap", 1);
					SP.UseTex(std::get<uint>(texMap), "emissionMap");
					break;
				case TexType::Reflection:
					SP.Set1i("useReflectionMap", 1);
					SP.UseTex(std::get<uint>(texMap), "reflectionMap");
					break;
				case TexType::Bump:
					SP.Set1i("useBumpMap", 1);
					SP.UseTex(std::get<uint>(texMap), "bumpMap");
					break;
			}
		}
	}

	if(!VAO){
		Create();
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(Vertex), &(*vertices)[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, colour));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tangent));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, diffuseTexIndex));

		if(indices){
			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(uint), &(*indices)[0], GL_STATIC_DRAW);
		}
	} else{
		glBindVertexArray(VAO);
	}
	
	glDrawElements(primitive, 6, GL_UNSIGNED_INT, (const void*)(long long(this->currFrame) * 6 * sizeof(GLuint))); //more??
	glBindVertexArray(0);
	if(autoConfig){
		SP.ResetTexUnits();
	}
}