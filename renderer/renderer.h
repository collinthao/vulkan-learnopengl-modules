#ifndef RENDERER_H
#define RENDERER_H

class IRenderer
{
	public:
	virtual ~IRenderer(){};
	virtual void drawFrame() = 0;
};

#endif
