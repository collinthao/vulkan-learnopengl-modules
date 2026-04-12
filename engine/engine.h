#ifndef ENGINE_H
#define ENGINE_H
#include "../renderer/vulkanRenderer.h"

class Engine
{
	public:
	Engine();
	VulkanRenderer renderer;
	
	void init();

};

#endif
