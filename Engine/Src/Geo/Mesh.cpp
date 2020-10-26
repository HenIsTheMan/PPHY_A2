#include "Mesh.h"
#include "../Global/GlobalFuncs.h"

Mesh::Mesh():
	type(MeshType::Amt),
	primitive(GL_TRIANGLES),
	vertices(nullptr),
	indices(nullptr),
	texMaps({}),
	modelMats({}),
	batchVAO(0),
	batchVBO(0),
	batchEBO(0),
	instancingVAO(0),
	instancingVBO(0),
	instancingEBO(0),
	VAO(0),
	VBO(0),
	EBO(0),
	model(glm::mat4(1.f))
{
}

Mesh::Mesh(const MeshType& type, const int& primitive, const std::initializer_list<std::tuple<str, TexType, uint>>& iL):
	type(type),
	primitive(primitive),
	vertices(nullptr),
	indices(nullptr),
	texMaps(iL),
	modelMats({}),
	batchVAO(0),
	batchVBO(0),
	batchEBO(0),
	instancingVAO(0),
	instancingVBO(0),
	instancingEBO(0),
	VAO(0),
	VBO(0),
	EBO(0),
	model(glm::mat4(1.f))
{
}

Mesh::Mesh(const Mesh& mesh): Mesh(){
	if(this != &mesh){
		*this = mesh;
	}
}

Mesh::Mesh(Mesh&& mesh) noexcept: Mesh(){
	if(this != &mesh){
		*this = mesh;
	}
}

Mesh& Mesh::operator=(const Mesh& mesh){
	if(this != &mesh){
		type = mesh.type;
		primitive = mesh.primitive;
		vertices = new std::vector<Vertex>(mesh.vertices->begin(), mesh.vertices->end());
		indices = new std::vector<uint>{mesh.indices->begin(), mesh.indices->end()};
		texMaps = mesh.texMaps;
		modelMats = mesh.modelMats;
		batchVAO = mesh.batchVAO;
		batchVBO = mesh.batchVBO;
		batchEBO = mesh.batchEBO;
		instancingVAO = mesh.instancingVAO;
		instancingVBO = mesh.instancingVBO;
		instancingEBO = mesh.instancingEBO;
		VAO = mesh.VAO;
		VBO = mesh.VBO;
		EBO = mesh.EBO;
		model = mesh.model;
	}
	return *this;
}

Mesh& Mesh::operator=(Mesh&& mesh) noexcept{
	if(this != &mesh){
		type = mesh.type;
		primitive = mesh.primitive;
		vertices = new std::vector<Vertex>(mesh.vertices->begin(), mesh.vertices->end());
		indices = new std::vector<uint>{mesh.indices->begin(), mesh.indices->end()};
		texMaps = mesh.texMaps;
		modelMats = mesh.modelMats;
		batchVAO = mesh.batchVAO;
		batchVBO = mesh.batchVBO;
		batchEBO = mesh.batchEBO;
		instancingVAO = mesh.instancingVAO;
		instancingVBO = mesh.instancingVBO;
		instancingEBO = mesh.instancingEBO;
		VAO = mesh.VAO;
		VBO = mesh.VBO;
		EBO = mesh.EBO;
		model = mesh.model;
	}
	return *this;
}

Mesh::~Mesh(){
	if(vertices){
		delete vertices;
		vertices = nullptr;
	}
	if(indices){
		delete indices;
		indices = nullptr;
	}
	for(const std::tuple<str, TexType, uint>& texMap: texMaps){
		glDeleteTextures(1, &std::get<uint>(texMap));
	}
	if(batchVAO){
		glDeleteVertexArrays(1, &batchVAO);
	}
	if(batchVBO){
		glDeleteBuffers(1, &batchVBO);
	}
	if(batchEBO){
		glDeleteBuffers(1, &batchEBO);
	}
	if(instancingVAO){
		glDeleteVertexArrays(1, &instancingVAO);
	}
	if(instancingVBO){
		glDeleteBuffers(1, &instancingVBO);
	}
	if(instancingEBO){
		glDeleteBuffers(1, &instancingEBO);
	}
	if(VAO){
		glDeleteVertexArrays(1, &VAO);
	}
	if(VBO){
		glDeleteBuffers(1, &VBO);
	}
	if(EBO){
		glDeleteBuffers(1, &EBO);
	}
}

void Mesh::InstancedRender(ShaderProg& SP, const bool& autoConfig){
	if(primitive < 0){
		return (void)puts("Invalid primitive!\n");
	}

	SP.Use();
	SP.SetMat4fv("model", &(model)[0][0]);
	if(autoConfig){
		SP.Set1i("instancing", 1);

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
					type != MeshType::Amt,
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

	if(!instancingVAO){
		switch(type){
			case MeshType::Quad:
				CreateQuad();
				break;
			case MeshType::Cube:
				CreateCube();
				break;
			case MeshType::Sphere:
				CreateSphere();
				break;
			case MeshType::Cylinder:
				CreateCylinder();
				break;
		}
		glGenVertexArrays(1, &instancingVAO);
		glGenBuffers(1, &instancingVBO);

		glBindVertexArray(instancingVAO);
			glBindBuffer(GL_ARRAY_BUFFER, instancingVBO);

			glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(Vertex) + modelMats.size() * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices->size() * sizeof(Vertex), &(*vertices)[0]);
			glBufferSubData(GL_ARRAY_BUFFER, vertices->size() * sizeof(Vertex), modelMats.size() * sizeof(glm::mat4), &(modelMats)[0]);

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

			size_t mySize = vertices->size() * sizeof(Vertex);
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (const void*)(mySize));
			mySize += sizeof(glm::vec4);
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (const void*)(mySize));
			mySize += sizeof(glm::vec4);
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (const void*)(mySize));
			mySize += sizeof(glm::vec4);
			glEnableVertexAttribArray(9);
			glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (const void*)(mySize));

			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);
			glVertexAttribDivisor(9, 1);

			if(indices){
				glGenBuffers(1, &instancingEBO);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instancingEBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(uint), &(*indices)[0], GL_STATIC_DRAW);
			}
	} else{
		glBindVertexArray(instancingVAO);
	}

	indices ? glDrawElementsInstanced(primitive, (int)indices->size(), GL_UNSIGNED_INT, nullptr, (int)modelMats.size()) : glDrawArraysInstanced(primitive, 0, (int)vertices->size(), (int)modelMats.size());
	glBindVertexArray(0);
	if(autoConfig){
		SP.ResetTexUnits();
	}
}

void Mesh::Render(ShaderProg& SP, const bool& autoConfig){
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
					type != MeshType::Amt,
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
		switch(type){
			case MeshType::Quad:
				CreateQuad();
				break;
			case MeshType::Cube:
				CreateCube();
				break;
			case MeshType::Sphere:
				CreateSphere();
				break;
			case MeshType::Cylinder:
				CreateCylinder();
				break;
		}
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

	indices ? glDrawElements(primitive, (int)indices->size(), GL_UNSIGNED_INT, nullptr) : glDrawArrays(primitive, 0, (int)vertices->size());
	glBindVertexArray(0);
	if(autoConfig){
		SP.ResetTexUnits();
	}
}

void Mesh::AddModelMat(const glm::mat4& modelMat){
	modelMats.emplace_back(modelMat);
}

void Mesh::AddTexMap(const std::tuple<str, TexType, uint>& texMap){
	texMaps.emplace_back(texMap);
}

void Mesh::ClearModelMats(){
	modelMats.clear();
}

void Mesh::RemoveTexMap(str const& texPath){
	const size_t& size = texMaps.size();
	for(size_t i = 0; i < size; ++i){
		if(std::get<str>(texMaps[i]) == texPath){
			glDeleteTextures(1, &std::get<uint>(texMaps[i]));
			texMaps.erase(texMaps.begin() + i);
			break; //Prevents vec subscript from going out of range due to size
		}
	}
}

void Mesh::SetModel(const glm::mat4& model){
	this->model = model;
}

void Mesh::CreateQuad(){
	if(!vertices){
		const glm::vec3 pos[4]{glm::vec3(-1.f, 1.f, 0.f), glm::vec3(-1.f, -1.f, 0.f), glm::vec3(1.f, -1.f, 0.f), glm::vec3(1.f, 1.f, 0.f)};
		const glm::vec2 UVs[4]{glm::vec2(0.f, 1.f), glm::vec2(0.f, 0.f), glm::vec2(1.f, 0.f), glm::vec2(1.f, 1.f)};

		///T and B lie on the same plane as normal map surface and align with tex axes U and V so calc them with vertices (to get edges of...) and texCoords (since in tangent space) of primitives
		glm::vec3 tangent[2];
		for(short i = 0; i < 2; ++i){
			const glm::vec3 edges[2]{pos[!i ? 1 : 3] - pos[2 * i], pos[2 * !i] - pos[2 * i]};
			const glm::vec2 deltaUVs[2]{UVs[!i ? 1 : 3] - UVs[2 * i], UVs[2 * !i] - UVs[2 * i]};
			const float reciprocal = 1.f / (deltaUVs[0].x * deltaUVs[1].y - deltaUVs[1].x * deltaUVs[0].y);

			tangent[i].x = reciprocal * (deltaUVs[1].y * edges[0].x - deltaUVs[0].y * edges[1].x);
			tangent[i].y = reciprocal * (deltaUVs[1].y * edges[0].x - deltaUVs[0].y * edges[1].x);
			tangent[i].z = reciprocal * (deltaUVs[1].y * edges[0].x - deltaUVs[0].y * edges[1].x);
		}

		vertices = new std::vector<Vertex>(4);
		for(short i = 0; i < 4; ++i){
			(*vertices)[i] = {
				pos[i],
				glm::vec4(.7f, .4f, .1f, 1.f),
				UVs[i],
				glm::vec3(0.f, 0.f, 1.f),
				tangent[!(i % 3)],
			};
		}

		if(indices){
			delete indices;
			indices = nullptr;
		}
		indices = new std::vector<uint>{0, 1, 2, 0, 2, 3};
	}
}

void Mesh::CreateCube(){
	if(!vertices){
		vertices = new std::vector<Vertex>(24);

		for(short i = 0; i < 4; ++i){
			(*vertices)[i].normal = glm::vec3(0.f, 1.f, 0.f);
		}
		(*vertices)[0].pos = glm::vec3(1.f);
		(*vertices)[0].texCoords = glm::vec2(1.f, 0.f);
		(*vertices)[1].pos = glm::vec3(1.f, 1.f, -1.f);
		(*vertices)[1].texCoords = glm::vec2(1.f);
		(*vertices)[2].pos = glm::vec3(-1.f, 1.f, -1.f);
		(*vertices)[2].texCoords = glm::vec2(0.f, 1.f);
		(*vertices)[3].pos = glm::vec3(-1.f, 1.f, 1.f);
		(*vertices)[3].texCoords = glm::vec2(0.f);

		for(short i = 4; i < 8; ++i){
			(*vertices)[i].normal = glm::vec3(1.f, 0.f, 0.f);
		}
		(*vertices)[4].pos = glm::vec3(1.f, -1.f, 1.f);
		(*vertices)[4].texCoords = glm::vec2(0.f);
		(*vertices)[5].pos = glm::vec3(1.f, -1.f, -1.f);
		(*vertices)[5].texCoords = glm::vec2(1.f, 0.f);
		(*vertices)[6].pos = glm::vec3(1.f, 1.f, -1.f);
		(*vertices)[6].texCoords = glm::vec2(1.f);
		(*vertices)[7].pos = glm::vec3(1.f);
		(*vertices)[7].texCoords = glm::vec2(0.f, 1.f);

		for(short i = 8; i < 12; ++i){
			(*vertices)[i].normal = glm::vec3(0.f, 0.f, 1.f);
		}
		(*vertices)[8].pos = glm::vec3(1.f);
		(*vertices)[8].texCoords = glm::vec2(1.f);
		(*vertices)[9].pos = glm::vec3(-1.f, 1.f, 1.f);
		(*vertices)[9].texCoords = glm::vec2(0.f, 1.f);
		(*vertices)[10].pos = glm::vec3(-1.f, -1.f, 1.f);
		(*vertices)[10].texCoords = glm::vec2(0.f);
		(*vertices)[11].pos = glm::vec3(1.f, -1.f, 1.f);
		(*vertices)[11].texCoords = glm::vec2(1.f, 0.f);

		for(short i = 12; i < 16; ++i){
			(*vertices)[i].normal = glm::vec3(0.f, 0.f, -1.f);
		}
		(*vertices)[12].pos = glm::vec3(1.f, -1.f, -1.f);
		(*vertices)[12].texCoords = glm::vec2(0.f);
		(*vertices)[13].pos = glm::vec3(-1.f);
		(*vertices)[13].texCoords = glm::vec2(1.f, 0.f);
		(*vertices)[14].pos = glm::vec3(-1.f, 1.f, -1.f);
		(*vertices)[14].texCoords = glm::vec2(1.f);
		(*vertices)[15].pos = glm::vec3(1.f, 1.f, -1.f);
		(*vertices)[15].texCoords = glm::vec2(0.f, 1.f);

		for(short i = 16; i < 20; ++i){
			(*vertices)[i].normal = glm::vec3(-1.f, 0.f, 0.f);
		}
		(*vertices)[16].pos = glm::vec3(-1.f);
		(*vertices)[16].texCoords = glm::vec2(0.f);
		(*vertices)[17].pos = glm::vec3(-1.f, -1.f, 1.f);
		(*vertices)[17].texCoords = glm::vec2(1.f, 0.f);
		(*vertices)[18].pos = glm::vec3(-1.f, 1.f, 1.f);
		(*vertices)[18].texCoords = glm::vec2(1.f);
		(*vertices)[19].pos = glm::vec3(-1.f, 1.f, -1.f);
		(*vertices)[19].texCoords = glm::vec2(0.f, 1.f);

		for(short i = 20; i < 24; ++i){
			(*vertices)[i].normal = glm::vec3(0.f, -1.f, 0.f);
		}
		(*vertices)[20].pos = glm::vec3(-1.f);
		(*vertices)[20].texCoords = glm::vec2(0.f);
		(*vertices)[21].pos = glm::vec3(1.f, -1.f, -1.f);
		(*vertices)[21].texCoords = glm::vec2(1.f, 0.f);
		(*vertices)[22].pos = glm::vec3(1.f, -1.f, 1.f);
		(*vertices)[22].texCoords = glm::vec2(1.f);
		(*vertices)[23].pos = glm::vec3(-1.f, -1.f, 1.f);
		(*vertices)[23].texCoords = glm::vec2(0.f, 1.f);

		if(indices){
			delete indices;
			indices = nullptr;
		}
		indices = new std::vector<uint>(36);
		const short myArr[6]{0, 1, 2, 2, 3, 0};
		for(long long i = 0; i < 6; ++i){
			for(long long j = 0; j < 6; ++j){
				(*indices)[i * 6 + j] = uint(i * 4 + myArr[j]);
			}
		}
	}
}

void Mesh::CreateSphere(){
	if(!vertices){
		const uint stackAmt = 50;
		const uint sliceAmt = 50;
		vertices = new std::vector<Vertex>();
		if(indices){
			delete indices;
			indices = nullptr;
		}
		indices = new std::vector<uint>();

		const float stackAngle = 180.f / stackAmt;
		const float sliceAngle = 360.f / sliceAmt;
		for(unsigned stack = 0; stack < stackAmt + 1; ++stack){
			const float phi = -90.f + stack * stackAngle;
			for(unsigned slice = 0; slice < sliceAmt + 1; ++slice){
				const float theta = slice * sliceAngle;
				const glm::vec3 pos(cos(glm::radians(phi)) * cos(glm::radians(theta)), sin(glm::radians(phi)), cos(glm::radians(phi)) * sin(glm::radians(theta)));
				vertices->push_back({
					pos,
					glm::vec4(.7f, .4f, .1f, 1.f),
					glm::vec2((float)stack / float(stackAmt), (float)slice / float(sliceAmt)),
					glm::normalize(pos),
					});

				indices->emplace_back(stack * (sliceAmt + 1) + slice);
				indices->emplace_back((stack + 1) * (sliceAmt + 1) + slice);
			}
		}
	}
}

void Mesh::CreateCylinder(){
	if(!vertices){
		Vertex v;
		const uint sliceAmt = 50;
		const float sliceAngle = -360.f / sliceAmt;
		vertices = new std::vector<Vertex>();
		if(indices){
			delete indices;
			indices = nullptr;
		}
		indices = new std::vector<uint>();

		v.normal = v.pos = glm::vec3(0.f, 0.f, -1.f);
		v.texCoords = glm::vec3(.5f);
		vertices->emplace_back(v);

		for(unsigned slice = 0; slice < sliceAmt + 1; ++slice){
			const float theta = slice * sliceAngle;

			v.pos = glm::vec3(cos(glm::radians(theta)), sin(glm::radians(theta)), -1.f);
			v.texCoords = glm::vec2((v.pos.x + 1.f) / 2.f, (v.pos.z + 1.f) / 2.f);
			vertices->emplace_back(v);

			indices->emplace_back(0);
			indices->emplace_back(slice + 1);
		}

		for(unsigned slice = 0; slice < sliceAmt + 1; ++slice){
			const float theta = slice * sliceAngle;

			v.pos = glm::vec3(cos(glm::radians(theta)), sin(glm::radians(theta)), -1.f);
			v.texCoords = glm::vec2(float(slice) / float(sliceAmt), 0.f);
			v.normal = glm::normalize(glm::vec3(v.pos.x, v.pos.y, 0.f));
			vertices->emplace_back(v);

			v.pos = glm::vec3(cos(glm::radians(theta)), sin(glm::radians(theta)), 1.f);
			v.texCoords = glm::vec2(float(slice) / float(sliceAmt), 1.f);
			v.normal = glm::normalize(glm::vec3(v.pos.x, v.pos.y, 0.f));
			vertices->emplace_back(v);

			indices->emplace_back(slice * 2 + sliceAmt + 2);
			indices->emplace_back(slice * 2 + 1 + sliceAmt + 2);
		}

		v.normal = v.pos = glm::vec3(0.f, 0.f, 1.f);
		v.texCoords = glm::vec3(.5f);
		vertices->emplace_back(v);

		for(unsigned slice = 0; slice < sliceAmt + 1; ++slice){
			const float theta = slice * sliceAngle;

			v.pos = glm::vec3(cos(glm::radians(theta)), sin(glm::radians(theta)), 1.f);
			v.texCoords = glm::vec2((v.pos.x + 1.f) / 2.f, (v.pos.z + 1.f) / 2.f);
			vertices->emplace_back(v);

			indices->emplace_back(slice + 1 + sliceAmt * 3 + 3 + 1); //+1 at the end as +1 vertex before for loop
			indices->emplace_back(0 + sliceAmt * 3 + 3 + 1); //...
		}
	}
}