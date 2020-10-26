#include "ShaderProg.h"

std::vector<int> ShaderProg::texTargets;
ShaderProg* ShaderProg::currSP = nullptr;
std::unordered_map<cstr, uint> ShaderProg::shaderCache;

ShaderProg::ShaderProg():
	refID(0),
	shaderPaths{"", "", ""}
{
}

ShaderProg::ShaderProg(cstr const& vsPath, cstr const& fsPath, cstr const& gsPath):
	refID(0),
	shaderPaths{vsPath, fsPath, gsPath}
{
}

ShaderProg::~ShaderProg(){
	if(!shaderCache.empty()){
		for(const auto& shader: shaderCache){
			glDeleteShader(shader.second);
		}
		shaderCache.clear();
	}
	glDeleteProgram(refID);
}

int ShaderProg::GetUniLocation(cstr const& uniName){
	if(!uniLocationCache.count(str{uniName})){ //If not cached...
		uniLocationCache[str{uniName}] = glGetUniformLocation(refID, uniName); //Query location of uni
		if(uniLocationCache[str{uniName}] == -1){
			(void)printf("%u: Failed to find uni '%s'\n", this->refID, uniName);
		}
	}
	return uniLocationCache[str{uniName}];
}

bool ShaderProg::Init(){
	if(shaderPaths[0] == ""){
		return false;
	}
	refID = glCreateProgram();
	for(short i = 0; i < sizeof(shaderPaths) / sizeof(shaderPaths[0]) - (shaderPaths[2] == ""); ++i){
		if(shaderCache.count(shaderPaths[i])){
			(void)printf("Reusing \"%s\"...\n", shaderPaths[i]);
			glAttachShader(refID, shaderCache[shaderPaths[i]]);
		} else{
			uint shaderRefID = glCreateShader(i < 2 ? (~i & 1 ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER) : GL_GEOMETRY_SHADER);
			ParseShader(shaderPaths[i], shaderRefID);
			glAttachShader(refID, shaderRefID);
			shaderCache[shaderPaths[i]] = shaderRefID;
		}
	}
	Link();
	return true;
}

void ShaderProg::Link() const{
	int infoLogLength;
	(void)puts("Linking prog...\n");
	glLinkProgram(refID); //Vars in diff shaders are linked here too

	glGetProgramiv(refID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if(infoLogLength){
		char* errorMsg = (char*)_malloca(infoLogLength * sizeof(char)); //Allocate memory on the stack dynamically
		glGetProgramInfoLog(refID, infoLogLength, &infoLogLength, errorMsg);
		(void)printf("%s\n", errorMsg);
	}
}

void ShaderProg::ParseShader(cstr const& fPath, const uint& shaderRefID) const{
	int infoLogLength;
	str srcCodeStr, line;
	std::ifstream stream(fPath);

	if(!stream.is_open()){
		return (void)printf("Failed to open and read \"%s\"\n", fPath);
	}
	while(getline(stream, line)){
		srcCodeStr += "\n" + line;
	}
	stream.close();

	(void)printf("Compiling \"%s\"...\n", fPath);
	cstr srcCodeCStr = srcCodeStr.c_str();
	glShaderSource(shaderRefID, 1, &srcCodeCStr, 0);
	glCompileShader(shaderRefID);
	glGetShaderiv(shaderRefID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if(infoLogLength){
		char* errorMsg = (char*)_malloca(infoLogLength * sizeof(char)); //Allocate memory on the stack dynamically
		glGetShaderInfoLog(shaderRefID, infoLogLength, &infoLogLength, errorMsg);
		(void)printf("Failed to compile \"%s\"!\n%s\n", fPath, errorMsg);
	}
}

void ShaderProg::Use(){
	if(!refID && !Init()){ //Init on 1st use
		(void)printf("%u: ShaderProg failed to initialise\n", this->refID);
	}
	if(!currSP || currSP->refID != refID){
		glUseProgram(refID);
		currSP = this;
	}
}

void ShaderProg::UseTex(const uint& texRefID, const cstr& samplerName, const int& texTarget){
	const int size = (int)texTargets.size();
	if(size == 32){
		return (void)puts("Exceeded 32 texs in draw call!\n");
	}
	glActiveTexture(GL_TEXTURE0 + size);
	glBindTexture(texTarget, texRefID);
	Set1i(samplerName, size); //Make sure each shader sampler uni corresponds to the correct tex unit
	texTargets.emplace_back(texTarget);
}

void ShaderProg::ResetTexUnits() const{
	for(int i = (int)texTargets.size() - 1; i >= 0; --i){
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(texTargets[i], 0);
		texTargets.pop_back();
	}
}

void ShaderProg::Set1f(cstr const& uniName, const float& val){
	glUniform1f(GetUniLocation(uniName), val);
}

void ShaderProg::Set3f(cstr const& uniName, const float& a, const float& b, const float& c){
	glUniform3f(GetUniLocation(uniName), a, b, c);
}

void ShaderProg::Set3fv(cstr const& uniName, const glm::vec3& vec){
	glUniform3fv(GetUniLocation(uniName), 1, &vec[0]);
}

void ShaderProg::Set4fv(cstr const& uniName, const glm::vec4& vec){
	glUniform4fv(GetUniLocation(uniName), 1, &vec[0]);
}

void ShaderProg::Set1i(cstr const& uniName, const int& val){
	glUniform1i(GetUniLocation(uniName), val);
}

void ShaderProg::SetMat4fv(cstr const& uniName, const float* const& floatPtr, const bool& transpose){
	glUniformMatrix4fv(GetUniLocation(uniName), 1, transpose, floatPtr);
}