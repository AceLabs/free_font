#include "stdafx.h"
#include <crtdbg.h>

#include "WinOs/WinOs.h"
#include "Xel/Xel.h"
#include "free_font.h"

#include "Callback.h"

void Init() {
	Atlas *atlas = new Atlas("C:\\_c\\c_lib\\lib\\arial.ttf");
	atlas->load(26);
	printf("atlas width %i\n", atlas->atlasWidth);


}

void Uninit() {
}

void OnTick() {
	Xel::Swap();
}

int main(int argc, char** argv) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Xel::Init();

	Xel::Window::SetCallbacks(OnResize, Init, Uninit);

	Xel::Mouse::SetCallbacks(
		OnMouseMove,
		OnMouseButton,
		OnMouseScroll);

	Xel::Keyboard::SetCallbacks(
		OnKeyDown,
		OnKeyUp);

	Xel::Window::SetTitle("Xel v2016.03.26");
	Xel::Window::SetPosition(200, 100);
	Xel::Window::SetSize(1280, 800);

	Xel::Loop(OnTick);

	std::cout << "good bye." << std::endl;	return 0;




	/*Canvas *v = new Canvas(640, 480);
	Atlas *atlas = new Atlas("C:\\_c\\c_lib\\lib\\arial.ttf");

	atlas->load(26);

	printf("atlas width %i\n", atlas->atlasWidth);

	//v->draw("The Quick Brown Fox Jumped Over the Lazy Dog!", 50, 100, atlas);
	v->draw("The Quick Brown Fox Jumped Over The Lazy Dog...", 50, 100, atlas);

	v->toFile("c:\\temp\\cv.data");

	delete atlas;
	delete v;*/

	return 1;
}
