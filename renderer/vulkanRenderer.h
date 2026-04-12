#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H
#include "./renderer.h"

class VulkanRenderer : IRenderer
{
	public:
	VulkanRenderer();
	virtual void drawFrame();
}; 

#endif
