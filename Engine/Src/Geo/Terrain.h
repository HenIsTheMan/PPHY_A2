#pragma once
#include "Mesh.h"

class Terrain final: public Mesh{
public:
	Terrain();
	Terrain(cstr const& fPath, const float& tileH, const float& tileV);
	~Terrain() = default;

	float GetHeightAtPt(const float& x, const float& z) const;
	void Render(ShaderProg& SP, const bool& autoConfig = true) override;
private:
	cstr terrainPath;
	float tileH;
	float tileV;
	std::vector<unsigned char> data;

	bool Load();
	void Create();
};