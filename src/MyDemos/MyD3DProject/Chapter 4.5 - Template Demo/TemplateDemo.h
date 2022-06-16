#pragma once

#include "../../../Common/d3dApp.h"
#include <DirectXColors.h>

class TemplateDemo : public D3DApp
{
public :
	TemplateDemo(HINSTANCE hInstance);
	~TemplateDemo();
	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;
};

