#include <Core.h>
#include <Engine.h>
#include "Admin/App.h"

extern bool endLoop;

void MainProcess(){
	App* app = App::GetObjPtr(); //Singleton<App>* test = Singleton<App>::GetObjPtr()??
	while(!endLoop){
		app->Update();
		app->PreRender();
		app->Render();
		app->PostRender();
		if(glfwWindowShouldClose(App::win)){
			endLoop = true;
		}
	}
	app->Destroy();
}

int main(const int&, const char* const* const&){
	if(!InitConsole()){
		return -1;
	}
	std::thread worker(&MainProcess);
	while(!endLoop){
		if(Key(VK_ESCAPE)){
			endLoop = true;
			break;
		}
	}
	worker.join();
}