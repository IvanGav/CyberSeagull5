#pragma once

#include "drillengine/DrillLib.h";
#include "Resources.h"

namespace Graphics {

// World background is in 16x16 sprites with a scale factor of 4, optimize for that
void blit_sprite16x4(Resources::Sprite& sprite, I32 x, I32 y, I32 animFrame) {
	I32 dstX = max(x, 0);
	I32 dstY = max(y, 0);
	I32 srcX = (x >= 0 ? 0 : -x) + (sprite.x + animFrame * sprite.width) * 4;
	I32 srcY = (y >= 0 ? 0 : -y) + sprite.y * 4;
	I32 sizeX = min<I32>(x + sprite.width * 4, Win32::framebufferWidth) - dstX;
	I32 sizeY = min<I32>(y + sprite.height * 4, Win32::framebufferHeight) - dstY;
	for (I32 blitY = 0; blitY < sizeY; blitY++) {
		RGBA8* src = &sprite.tex->pixels[((blitY + srcY) / 4) * sprite.tex->width];
		RGBA8* dst = &Win32::framebuffer[(blitY + dstY) * Win32::framebufferWidth] + dstX;
		for (I32 blitX = 0; blitX < sizeX; blitX++) {
			dst[blitX] = src[(srcX + blitX) / 4];
		}
	}
}

void blit_sprite(Resources::Sprite& sprite, I32 x, I32 y, I32 scaleFactor, I32 animFrame) {
	I32 dstX = max(x, 0);
	I32 dstY = max(y, 0);
	I32 srcX = (x >= 0 ? 0 : -x) + (sprite.x + animFrame * sprite.width) * scaleFactor;
	I32 srcY = (y >= 0 ? 0 : -y) + sprite.y * scaleFactor;
	I32 sizeX = min<I32>(x + sprite.width * scaleFactor, Win32::framebufferWidth) - dstX;
	I32 sizeY = min<I32>(y + sprite.height * scaleFactor, Win32::framebufferHeight) - dstY;
	for (I32 blitY = 0; blitY < sizeY; blitY++) {
		RGBA8* src = &sprite.tex->pixels[((blitY + srcY) / scaleFactor) * sprite.tex->width];
		RGBA8* dst = &Win32::framebuffer[(blitY + dstY) * Win32::framebufferWidth] + dstX;
		for (I32 blitX = 0; blitX < sizeX; blitX++) {
			dst[blitX] = src[(srcX + blitX) / scaleFactor ];
		}
	}
}

void blit_sprite_cutout(Resources::Sprite& sprite, I32 x, I32 y, I32 scaleFactor, I32 animFrame) {
	I32 dstX = max(x, 0);
	I32 dstY = max(y, 0);
	I32 srcX = (x >= 0 ? 0 : -x) + (sprite.x + animFrame * sprite.width) * scaleFactor;
	I32 srcY = (y >= 0 ? 0 : -y) + sprite.y * scaleFactor;
	I32 sizeX = min<I32>(x + sprite.width * scaleFactor, Win32::framebufferWidth) - dstX;
	I32 sizeY = min<I32>(y + sprite.height * scaleFactor, Win32::framebufferHeight) - dstY;
	for (I32 blitY = 0; blitY < sizeY; blitY++) {
		RGBA8* src = &sprite.tex->pixels[((blitY + srcY) / scaleFactor) * sprite.tex->width];
		RGBA8* dst = &Win32::framebuffer[(blitY + dstY) * Win32::framebufferWidth] + dstX;
		for (I32 blitX = 0; blitX < sizeX; blitX++) {
			RGBA8 srcPx = src[(srcX + blitX) / scaleFactor];
			if (srcPx.a != 0) {
				dst[blitX] = srcPx;
			}
		}
	}
}

void blit_texture(Resources::Texture& tex, I32 x, I32 y, I32 scaleFactor) {
	Resources::Sprite s{ &tex, 0, 0, tex.width, tex.height, 1 };
	blit_sprite(s, x, y, scaleFactor, 0);
}

}