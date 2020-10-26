#pragma once
#include "../World/Scene.h"

class App final: public Singleton<App>{
	explicit App(const App&) = delete;
	explicit App(App&&) noexcept = delete;
	App& operator=(const App&) = delete;
	App& operator=(App&&) noexcept = delete;

	friend Singleton<App>;
	friend void MainProcess();
public:
	enum struct FBO{
		GeoPass = 0,
		LightingPass,
		PingPong0,
		PingPong1,
		Amt
	};
	enum struct Tex{
		Pos = 0,
		Colours,
		Normals,
		Spec,
		Reflection,
		Lit,
		Bright,
		PingPong0,
		PingPong1,
		Amt
	};

	~App();
	bool Init();
	void Update();
	void PreRender() const;
	void Render();
	void PostRender() const;
private:
	App();
	bool InitOptions() const;
	bool fullscreen;
	float elapsedTime;
	float lastFrameTime;
	float toggleFullscreenBT;
	Scene scene;
	static const GLFWvidmode* mode;
	static GLFWwindow* win;

	uint FBORefIDs[(int)FBO::Amt];
	uint texRefIDs[(int)Tex::Amt];
	uint RBORefIDs[1];
};

template <class T>
inline T& operator++(T& myEnum){
	myEnum = T((int)myEnum + 1);
	return myEnum;
}