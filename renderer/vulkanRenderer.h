#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H
#include "./renderer.h"

class VulkanRenderer : public IRenderer
{
	public:
	VulkanRenderer();
	void drawFrame();
}; 

#endif
