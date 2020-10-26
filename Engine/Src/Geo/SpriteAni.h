#pragma once
#include "Mesh.h"

///Can be improved

class Ani final{
	friend class SpriteAni;
private:
	Ani():
		active(false),
		ended(false),
		time(0.f),
		repeatCount(0),
		name(""),
		frames({})
	{
	}

	bool active;
	bool ended;
	float time;
	int repeatCount;
	str name;
	std::vector<int> frames;
};

class SpriteAni final: public Mesh{
public:
	SpriteAni();
	SpriteAni(const int& rows, const int& cols);
	~SpriteAni();

	void Play(const str& name, const int& repeat, const float& time);
	void Pause();
	void Resume();
	void Reset();

	void AddAni(const str& name, const int& start, const int& end);
	void AddSequenceAni(const str& name, const ::std::initializer_list<int>& frames);
	void Create();
	void Update();
	void Render(ShaderProg& SP, const bool& autoConfig = true) override;
private:
	float currTime;
	int currFrame;
	int rows;
	int cols;
	int playCount;
	str currAni;
	std::unordered_map<str, Ani*> allAnis;
};