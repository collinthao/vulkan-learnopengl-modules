#ifndef WINDOWCONTEXT_H
#define WINDOWCONTEXT_H

class IWindowContext
{
	public:
	virtual ~IWindowContext(){};
	virtual void initWindow() = 0;
	virtual void* getWindow() = 0;
};

#endif
