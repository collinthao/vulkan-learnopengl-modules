#include <iostream>
#include "./engine.h"

Engine::Engine(){};

void Engine::start()
{
	renderer->render(static_cast<GLFWwindow*>(windowContext->getWindow()));
}
