#pragma once

#include "drillengine/DrillLib.h";
#include "Resources.h"

namespace Graphics {

void blit_image(Resources::Texture& tex, I32 x, I32 y) {
	I32 dstX = max(x, 0);
	I32 dstY = max(y, 0);
	I32 srcX = x >= 0 ? 0 : -x;
	I32 srcY = y >= 0 ? 0 : -y;
	I32 sizeX = min<I32>(x + tex.width, Win32::framebufferWidth) - dstX;
	I32 sizeY = min<I32>(y + tex.width, Win32::framebufferHeight) - dstY;
	for (I32 blitY = 0; blitY < sizeY; blitY++) {
		RGBA8* src = &tex.pixels[(blitY + srcY) * tex.width] + srcX;
		RGBA8* dst = &Win32::framebuffer[(blitY + dstY) * Win32::framebufferWidth] + dstX;
		for (I32 blitX = 0; blitX < sizeX; blitX++) {
			dst[blitX] = src[blitX];
		}
	}
}

}