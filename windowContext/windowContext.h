#ifndef WINDOWCONTEXT_H
#define WINDOWCONTEXT_H

class IWindowContext
{
	private:
	virtual void initWindow() = 0;

	public:
	virtual ~IWindowContext(){};
	virtual void* getWindow() = 0;
};

#endif
