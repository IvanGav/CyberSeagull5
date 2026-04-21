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

void blit_texture_cutout(Resources::Texture& tex, I32 x, I32 y, I32 scaleFactor) {
	Resources::Sprite s{ &tex, 0, 0, tex.width, tex.height, 1 };
	blit_sprite_cutout(s, x, y, scaleFactor, 0);
}

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
	if (!Win32::framebuffer || sizeX <= 0 || sizeY <= 0 || borderSize <= 0) {
		return;
	}

	I32 fbW = Win32::framebufferWidth;
	I32 fbH = Win32::framebufferHeight;
	if (fbW <= 0 || fbH <= 0) {
		return;
	}

	I32 x0 = x;
	I32 y0 = y;
	I32 x1 = x + sizeX;
	I32 y1 = y + sizeY;
	if (x1 <= 0 || y1 <= 0 || x0 >= fbW || y0 >= fbH) {
		return;
	}

	borderSize = min(borderSize, min(sizeX, sizeY) / 2);
	if (borderSize <= 0) {
		return;
	}

	I32 topY0 = max(y0, 0);
	I32 topY1 = min(y0 + borderSize, fbH);
	I32 botY0 = max(y1 - borderSize, 0);
	I32 botY1 = min(y1, fbH);
	I32 leftX0 = max(x0, 0);
	I32 leftX1 = min(x0 + borderSize, fbW);
	I32 rightX0 = max(x1 - borderSize, 0);
	I32 rightX1 = min(x1, fbW);
	I32 midY0 = max(y0 + borderSize, 0);
	I32 midY1 = min(y1 - borderSize, fbH);
	I32 spanX0 = max(x0, 0);
	I32 spanX1 = min(x1, fbW);

	for (I32 py = topY0; py < topY1; py++) {
		RGBA8* row = &Win32::framebuffer[py * fbW];
		for (I32 px = spanX0; px < spanX1; px++) {
			row[px] = color;
		}
	}
	for (I32 py = botY0; py < botY1; py++) {
		RGBA8* row = &Win32::framebuffer[py * fbW];
		for (I32 px = spanX0; px < spanX1; px++) {
			row[px] = color;
		}
	}
	for (I32 py = midY0; py < midY1; py++) {
		RGBA8* row = &Win32::framebuffer[py * fbW];
		for (I32 px = leftX0; px < leftX1; px++) {
			row[px] = color;
		}
		for (I32 px = rightX0; px < rightX1; px++) {
			row[px] = color;
		}
	}
}

// Really terrible box, but at least with a border..
// sizeX and sizeY **do** include the border
void box(I32 x, I32 y, I32 sizeX, I32 sizeY, I32 borderSize, RGBA8 borderColor, RGBA8 fillColor) {
	if (!Win32::framebuffer || sizeX <= 0 || sizeY <= 0) {
		return;
	}

	I32 fbW = Win32::framebufferWidth;
	I32 fbH = Win32::framebufferHeight;
	if (fbW <= 0 || fbH <= 0) {
		return;
	}

	I32 x0 = max(x, 0);
	I32 y0 = max(y, 0);
	I32 x1 = min(x + sizeX, fbW);
	I32 y1 = min(y + sizeY, fbH);
	if (x0 >= x1 || y0 >= y1) {
		return;
	}

	Graphics::border(x, y, sizeX, sizeY, borderSize, borderColor);

	I32 fillX0 = max(x + borderSize, 0);
	I32 fillY0 = max(y + borderSize, 0);
	I32 fillX1 = min(x + sizeX - borderSize, fbW);
	I32 fillY1 = min(y + sizeY - borderSize, fbH);
	if (fillX0 >= fillX1 || fillY0 >= fillY1) {
		return;
	}

	for (I32 py = fillY0; py < fillY1; py++) {
		RGBA8* row = &Win32::framebuffer[py * fbW];
		for (I32 px = fillX0; px < fillX1; px++) {
			row[px] = fillColor;
		}
	}
}

void fill_rect_blended(I32 x, I32 y, I32 width, I32 height, RGBA8 color) {
	I32 startX = max(x, 0);
	I32 startY = max(y, 0);
	I32 endX = min(x + width, Win32::framebufferWidth);
	I32 endY = min(y + height, Win32::framebufferHeight);
	if (startX >= endX || startY >= endY || color.a == 0) {
		return;
	}
	U32 srcA = color.a;
	U32 invA = 255u - srcA;
	for (I32 py = startY; py < endY; py++) {
		RGBA8* row = Win32::framebuffer + py * Win32::framebufferWidth;
		for (I32 px = startX; px < endX; px++) {
			RGBA8 dst = row[px];
			row[px].r = U8((U32(dst.r) * invA + U32(color.r) * srcA) / 255u);
			row[px].g = U8((U32(dst.g) * invA + U32(color.g) * srcA) / 255u);
			row[px].b = U8((U32(dst.b) * invA + U32(color.b) * srcA) / 255u);
			row[px].a = 255;
		}
	}
}

}