#pragma once
#include "../Core.h"

class ShaderProg final{
public:
	ShaderProg();
	ShaderProg(cstr const& vsPath, cstr const& fsPath, cstr const& gsPath = "");
	~ShaderProg();
	void Use();
	void UseTex(const uint& texRefID, const cstr& samplerName, const int& texTarget = GL_TEXTURE_2D);
	void ResetTexUnits() const;

	///Utility funcs
	void Set1f(cstr const& uniName, const float& val);
	void Set3f(cstr const& uniName, const float& a, const float& b, const float& c);
	void Set3fv(cstr const& uniName, const glm::vec3& vec);
	void Set4fv(cstr const& uniName, const glm::vec4& vec);
	void Set1i(cstr const& uniName, const int& val);
	void SetMat4fv(cstr const& uniName, const float* const& floatPtr, const bool& transpose = false);
private:
	static std::vector<int> texTargets; //For every active tex unit
	cstr shaderPaths[3];
	static ShaderProg* currSP;
	std::unordered_map<str, int> uniLocationCache;
	static std::unordered_map<cstr, uint> shaderCache;

	uint refID;
	bool Init();
	int GetUniLocation(cstr const& uniName);
	void Link() const;
	void ParseShader(cstr const& fPath, const uint& shaderRefID) const;
};