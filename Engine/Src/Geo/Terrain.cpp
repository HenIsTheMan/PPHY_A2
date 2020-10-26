#include "Terrain.h"

Terrain::Terrain(): Mesh(),
	terrainPath(""),
	tileH(0.f),
	tileV(0.f),
	data({})
{
}

Terrain::Terrain(cstr const& fPath, const float& tileH, const float& tileV): Mesh(),
	terrainPath(fPath),
	tileH(tileH),
	tileV(tileV),
	data({})
{
	Load();
}

float Terrain::GetHeightAtPt(const float& x, const float& z) const{
	if(!data.size() || x <= -.5f || x >= .5f || z <= -.5f || z >= .5f){ //??
		return 0.f;
	}
	const long long terrainSize = (long long)sqrt((double)data.size());
	const long long zCoord = (long long)((z + 0.5) * terrainSize);
	const long long xCoord = (long long)((x + 0.5) * terrainSize);
	return (float)data[zCoord * terrainSize + xCoord] / 255.f; //[0.f, 1.f]
}

void Terrain::Render(ShaderProg& SP, const bool& autoConfig){
	if(!vertices){
		Create();
	}
	Mesh::Render(SP, autoConfig);
}

bool Terrain::Load(){
	std::ifstream fStream(terrainPath, std::ios::binary);
	if(!fStream.is_open()){
		(void)printf("Failed to open and read \"%s\"\n", terrainPath);
		return false;
	}

	fStream.seekg(0, std::ios::end);
	std::streampos fsize = (long long)fStream.tellg();
	fStream.seekg(0, std::ios::beg);
	data.resize((long long)fsize);
	fStream.read((char*)&data[0], fsize);
	fStream.close();

	return true;
}

void Terrain::Create(){
	long long terrainSize = (long long)sqrt((double)data.size());
	vertices = new std::vector<Vertex>();
	indices = new std::vector<uint>();

	std::vector<std::vector<glm::vec3>> pos = std::vector<std::vector<glm::vec3>>(terrainSize, std::vector<glm::vec3>(terrainSize));
	for(long long z = 0; z < terrainSize; ++z){
		for(long long x = 0; x < terrainSize; ++x){
			float scaledHeight = (float)data[z * terrainSize + x] / 255.f; //[0.f, 1.f]
			pos[z][x] = glm::vec3(float(x) / terrainSize - .5f, scaledHeight, float(z) / terrainSize - .5f);
		}
	}

	std::vector<std::vector<glm::vec3>> normals = std::vector<std::vector<glm::vec3>>(terrainSize, std::vector<glm::vec3>(terrainSize));
	std::vector<std::vector<glm::vec3>> tempNormals[2]{std::vector<std::vector<glm::vec3>>(terrainSize, std::vector<glm::vec3>(terrainSize)),
		std::vector<std::vector<glm::vec3>>(terrainSize, std::vector<glm::vec3>(terrainSize))};
	for(long long z = 0; z < terrainSize - 1; ++z){
		for(long long x = 0; x < terrainSize - 1; ++x){
			const glm::vec3& vertexA = pos[z][x];
			const glm::vec3& vertexB = pos[z][x + 1];
			const glm::vec3& vertexC = pos[z + 1][x + 1];
			const glm::vec3& vertexD = pos[z + 1][x];
			const glm::vec3 triangleNormalA = glm::cross(vertexB - vertexA, vertexA - vertexD);
			const glm::vec3 triangleNormalB = glm::cross(vertexD - vertexC, vertexC - vertexB);
			tempNormals[0][z][x] = triangleNormalA.length() ? glm::normalize(triangleNormalA) : triangleNormalA;
			tempNormals[1][z][x] = triangleNormalB.length() ? glm::normalize(triangleNormalB) : triangleNormalB;
		}
	}
	for(long long z = 0; z < terrainSize; ++z){
		for(long long x = 0; x < terrainSize; ++x){
			const bool isFirstRow = z == 0;
			const bool isFirstColumn = x == 0;
			const bool isLastRow = z == terrainSize - 1;
			const bool isLastColumn = x == terrainSize - 1;
			glm::vec3 finalVertexNormal = glm::vec3(0.f);
			if(!isFirstRow && !isFirstColumn){ //Look for triangle to the upper-left
				finalVertexNormal += tempNormals[0][z - 1][x - 1];
			}
			if(!isFirstRow && !isLastColumn){ //Look for triangles to the upper-right
				for(short k = 0; k < 2; ++k){
					finalVertexNormal += tempNormals[k][z - 1][x];
				}
			}
			if(!isLastRow && !isLastColumn){ //Look for triangle to the bottom-right
				finalVertexNormal += tempNormals[0][z][x];
			}
			if(!isLastRow && !isFirstColumn){ //Look for triangles to the bottom-right
				for(short k = 0; k < 2; ++k){
					finalVertexNormal += tempNormals[k][z][x - 1];
				}
			}
			normals[z][x] = finalVertexNormal.length() ? glm::normalize(finalVertexNormal) : finalVertexNormal; //Store final normal of j-th vertex in i-th row //Normalize to give avg of 4 normals
			vertices->push_back({
				pos[z][x],
				glm::vec4(.7f, .4f, .1f, 1.f),
				glm::vec2(float(x) / float(terrainSize) * tileH, 1.f - float(z) / float(terrainSize) * tileV),
				normals[z][x],
				glm::vec3(0.f), //??
				0,
			});
		}
	}

	for(long long z = 0; z < terrainSize - 1; ++z){
		for(long long x = 0; x < terrainSize - 1; ++x){
			///Triangle 1
			indices->emplace_back(uint(terrainSize * z + x));
			indices->emplace_back(uint(terrainSize * (z + 1) + x));
			indices->emplace_back(uint(terrainSize * z + x + 1));

			///Triangle 2
			indices->emplace_back(uint(terrainSize * (z + 1) + x + 1));
			indices->emplace_back(uint(terrainSize * z + x + 1));
			indices->emplace_back(uint(terrainSize * (z + 1) + x));
		}
	}
}