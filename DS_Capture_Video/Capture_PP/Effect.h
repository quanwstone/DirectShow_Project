#pragma once
class CEffect
{
public:
	CEffect();
	~CEffect();

public:
	void Water(unsigned char *apSrc, int aiColor, int aiWaterH, int aiWidth, int aiBitwidth);
};

