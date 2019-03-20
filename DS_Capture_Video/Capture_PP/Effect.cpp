#include "stdafx.h"
#include "Effect.h"


CEffect::CEffect()
{
}


CEffect::~CEffect()
{
}

void CEffect::Water(unsigned char *apSrc, int aiColor, int aiWaterH, int aiWidth, int aiBitwidth)
{
	unsigned char r, g, b;
	int i, j;
	int linesize = 0;

	b = (aiColor >> 16) & 0xff;
	g = (aiColor >> 8) & 0xff;
	r = (aiColor) & 0xff;

	if (aiBitwidth == 32)
	{
		linesize = aiWidth * 4;
		for (j = 0; j<aiWaterH; j++)
		{
			for (i = 0; i<aiWidth; i++)
			{
				apSrc[j*linesize + 4 * i] = apSrc[(2 * aiWaterH - j)*linesize + 4 * i] * j*apSrc[(2 * aiWaterH - j)*linesize + 4 * i + 3] / 255 / aiWaterH + (b*(aiWaterH - j) / aiWaterH);
				apSrc[j*linesize + 4 * i + 1] = apSrc[(2 * aiWaterH - j)*linesize + 4 * i + 1] * j*apSrc[(2 * aiWaterH - j)*linesize + 4 * i + 3] / 255 / aiWaterH + (g*(aiWaterH - j) / aiWaterH);
				apSrc[j*linesize + 4 * i + 2] = apSrc[(2 * aiWaterH - j)*linesize + 4 * i + 2] * j*apSrc[(2 * aiWaterH - j)*linesize + 4 * i + 3] / 255 / aiWaterH + (r*(aiWaterH - j) / aiWaterH);
				apSrc[j*linesize + 4 * i + 3] = 0xff;
			}
		}
	}
	else if (aiBitwidth == 24)
	{
		linesize = aiWidth * 3;
		for (j = 0; j<aiWaterH; j++)
		{
			for (i = 0; i<aiWidth; i++)
			{
				apSrc[j*linesize + 3 * i] = apSrc[(2 * aiWaterH - j)*linesize + 3 * i] * j / aiWaterH + (b*(aiWaterH - j) / aiWaterH);
				apSrc[j*linesize + 3 * i + 1] = apSrc[(2 * aiWaterH - j)*linesize + 3 * i + 1] * j / aiWaterH + (g*(aiWaterH - j) / aiWaterH);
				apSrc[j*linesize + 3 * i + 2] = apSrc[(2 * aiWaterH - j)*linesize + 3 * i + 2] * j / aiWaterH + (r*(aiWaterH - j) / aiWaterH);
			}
		}
	}
}