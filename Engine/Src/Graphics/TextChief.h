#pragma once
#include "../Core.h"
#include "../Global/GlobalFuncs.h"
#include "ShaderProg.h"

///Can be improved

class TextChief final{
public:
	struct TextAttribs final{
		str text;
		float x;
		float y;
		float scaleFactor;
		glm::vec4 colour;
		uint texRefID;
	};

	TextChief();
	~TextChief();

	void RenderText(ShaderProg& SP, const TextAttribs& attribs);
private:
	struct CharMetrics final{
		uint texRefID;
		uint advance;
		glm::ivec2 bearing;
		glm::ivec2 size;
	};

	FT_Library ft;
	FT_Face face;
	std::map<char, CharMetrics> allChars;
	uint VAO;
	uint VBO;

	bool Init();
};