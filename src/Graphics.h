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

//template<typename... Values>
//void display_text(I32 x, I32 y, I32 fontSize, StrA fmt, Values... fmt_args) {
//	MemoryArena scratch = get_scratch_arena();
//	MEMORY_ARENA_FRAME(scratch) {
//		StrA = strafmt(scratch, fmt, fmt_args...);
//		// TODO
//		abort();
//	}
//}

void display_num(U32 text, I32 x, I32 y, I32 fontSize) {
	DEBUG_ASSERT(fontSize % 16 == 0, "fontSize must be multiple of 16");
	if (text == 0) {
		blit_sprite_cutout(Resources::tile.num[0], x, y, fontSize / 16, 0);
		return;
	}
	U32 decimal_digits = (U32)floorf32(log10f32((F32)text)); // i don't like this, but... yeah
	for (U32 i = decimal_digits; text > 0; i--) {
		blit_sprite_cutout(Resources::tile.num[text%10], x + i * fontSize, y, fontSize/16, 0);
		text /= 10;
	}
}

void border(I32 x, I32 y, I32 sizeX, I32 sizeY, I32 borderSize, RGBA8 color) {
	if (x + (y + sizeY) * sizeX > Win32::framebufferWidth * Win32::framebufferHeight) {
		__debugbreak();
		return; // silently fail; TODO fix later so it properly clips
	}
	DEBUG_ASSERT(sizeX > borderSize*2 && sizeY > borderSize*2, "Size of the box must be at least as big as twice the border size");
	I32 dstX = max(x, 0);
	I32 dstY = max(y, 0);
	for (I32 boxY = 0; boxY < borderSize; boxY++) {
		RGBA8* dst = &Win32::framebuffer[(dstY + boxY) * Win32::framebufferWidth] + dstX;
		RGBA8* dst2 = &Win32::framebuffer[(dstY + boxY + (sizeY - borderSize)) * Win32::framebufferWidth] + dstX;
		for (I32 boxX = 0; boxX < sizeX; boxX++) {
			dst[boxX] = color;
			dst2[boxX] = color;
		}
	}
	for (I32 boxX = 0; boxX < borderSize; boxX++) {
		RGBA8* dst = &Win32::framebuffer[(dstY + borderSize) * Win32::framebufferWidth] + dstX + boxX;
		RGBA8* dst2 = &Win32::framebuffer[(dstY + borderSize) * Win32::framebufferWidth] + (dstX + boxX + (sizeX - borderSize));
		for (I32 boxY = 0; boxY < sizeY - borderSize; boxY++) {
			dst[boxY * Win32::framebufferWidth] = color;
			dst2[boxY * Win32::framebufferWidth] = color;
		}
	}
}

// Really terrible box, but at least with a border..
// sizeX and sizeY **do** include the border
void box(I32 x, I32 y, I32 sizeX, I32 sizeY, I32 borderSize, RGBA8 borderColor, RGBA8 fillColor) {
	if (x + (y + sizeY) * sizeX > Win32::framebufferWidth * Win32::framebufferHeight) {
		__debugbreak();
		return; // silently fail; TODO fix later so it properly clips
	}
	DEBUG_ASSERT(sizeX > 4 && sizeY > 4, "Size of the box must be at least 4 pixels");
	I32 dstX = max(x, 0);
	I32 dstY = max(y, 0);
	Graphics::border(x, y, sizeX, sizeY, borderSize, borderColor);
	for (I32 boxY = borderSize; boxY < sizeY - borderSize; boxY++) {
		RGBA8* dst = &Win32::framebuffer[(dstY + boxY) * Win32::framebufferWidth] + dstX;
		for (I32 boxX = borderSize; boxX < sizeX - borderSize; boxX++) {
			dst[boxX] = fillColor;
		}
	}
}

}