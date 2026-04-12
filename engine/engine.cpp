#include <iostream>
#include "./engine.h"

Engine::Engine(){};

void Engine::init()
{
	windowContext->initWindow();
	GLFWwindow * window = static_cast<GLFWwindow*>(windowContext->getWindow());
	renderer->init(window);
}
