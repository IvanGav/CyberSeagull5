#pragma once

#include "drillengine/DrillLib.h";
#include "Resources.h"

namespace Graphics {

void blit_sprite(Resources::Sprite& sprite, I32 x, I32 y, I32 scaleFactor) {
	I32 dstX = max(x, 0);
	I32 dstY = max(y, 0);
	I32 srcX = (x >= 0 ? 0 : -x) + sprite.x;
	I32 srcY = (y >= 0 ? 0 : -y) + sprite.y;
	I32 sizeX = min<I32>(x + sprite.width * scaleFactor, Win32::framebufferWidth) - dstX;
	I32 sizeY = min<I32>(y + sprite.height * scaleFactor, Win32::framebufferHeight) - dstY;
	for (I32 blitY = 0; blitY < sizeY; blitY++) {
		RGBA8* src = &sprite.tex->pixels[(blitY / scaleFactor + srcY) * sprite.tex->width] + srcX;
		RGBA8* dst = &Win32::framebuffer[(blitY + dstY) * Win32::framebufferWidth] + dstX;
		for (I32 blitX = 0; blitX < sizeX; blitX++) {
			dst[blitX] = src[blitX / scaleFactor ];
		}
	}
}

void blit_texture(Resources::Texture& tex, I32 x, I32 y, I32 scaleFactor) {
	Resources::Sprite s{ &tex, 0, 0, tex.width, tex.height, 1 };
	blit_sprite(s, x, y, scaleFactor);
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

voi/*d display_num(U32 text, I32 x, I32 y, I32 fontSize) {
	while(text)
}*/

}