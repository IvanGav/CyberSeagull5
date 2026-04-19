#pragma once

#include "Win32.h"
#include "Resources.h"
#include "Graphics.h"
#include "TileSpace.h"
#include "BeeDemo.h"
#include "SelectUI.h"

namespace Cyber5eagull {
extern F64 lastFrameTime;
extern F32 dt;
extern V2F camera;
extern I32 worldTileScale;
extern V2U32 hiveTile;
I32 world_tile_pixels();
F32 world_tile_pixels_f32();
void clamp_camera();
V2F32 screen_to_world(V2F32 screenPosition);
void center_camera_on_tile(V2U32 tile);
void zoom_camera_at_screen(I32 zoomDelta, V2F32 screenAnchor);
void pan_camera_horizontally(I32 direction);
B32 mouse_to_tile(V2U32* tileOut);
}

namespace Cyber5eagull::CreativeToolkit {

using BeeDemo::CreativeBrush;

ArenaArrayList<Resources::Sprite*> selections;
static constexpr U32 BRUSH_COUNT = 17u;
CreativeBrush brushOrder[BRUSH_COUNT] = {
	CreativeBrush::TASK_SELECT,
	CreativeBrush::ERASE,
	CreativeBrush::GRASS,
	CreativeBrush::IRON,
	CreativeBrush::COPPER,
	CreativeBrush::FLOWERS,
	CreativeBrush::SAND,
	CreativeBrush::BEACH,
	CreativeBrush::WATER,
	CreativeBrush::MOUNTAIN,
	CreativeBrush::CONVEYOR,
	CreativeBrush::ASSEMBLER_SMALL,
	CreativeBrush::ASSEMBLER_LARGE,
	CreativeBrush::ASSEMBLER_VERY_LARGE,
	CreativeBrush::SPLITTER,
	CreativeBrush::HIVE_SMALL,
	CreativeBrush::HIVE_BIG,
};

I32 itemSize = 16;
I32 scale = 4;
I32 selectedItem = 0;
B32 tilesheetVisible = B32_FALSE;

I32 borderPadding = 50;
I32 borderSize = 3;
I32 selectBorderSize = 2;

RGBA8 selectedColor = RGBA8{ 255, 255, 80, 255 };
RGBA8 borderColor = RGBA8{ 0, 0, 0, 255 };
RGBA8 fillColor = RGBA8{ 100, 130, 170, 255 };

CreativeBrush selectedBrush = CreativeBrush::TASK_SELECT;
B32 selectedBrushFreePlacement = B32_FALSE;
Rotation2 selectedRotation = ROTATION2_0;

FINLINE Resources::Sprite* brush_icon(CreativeBrush brush) {
	if (brush == CreativeBrush::ERASE) {
		return &Resources::tile.undef;
	}
	return BeeDemo::creative_brush_sprite(brush);
}

FINLINE B32 brush_uses_rotation(CreativeBrush brush) {
	switch (brush) {
	case CreativeBrush::ASSEMBLER_SMALL:
	case CreativeBrush::ASSEMBLER_LARGE:
	case CreativeBrush::SPLITTER:
		return B32_TRUE;
	default:
		return B32_FALSE;
	}
}

FINLINE void set_selected_brush(CreativeBrush brush, B32 freePlacement = B32_FALSE) {
	selectedBrush = brush;
	selectedBrushFreePlacement = freePlacement;
	selectedRotation = ROTATION2_0;
	for (U32 i = 0; i < BRUSH_COUNT; i++) {
		if (brushOrder[i] == brush) {
			selectedItem = I32(i);
			break;
		}
	}
}

FINLINE void rotate_cw() {
	switch (selectedRotation) {
	case ROTATION2_0: selectedRotation = ROTATION2_90; break;
	case ROTATION2_90: selectedRotation = ROTATION2_180; break;
	case ROTATION2_180: selectedRotation = ROTATION2_270; break;
	default: selectedRotation = ROTATION2_0; break;
	}
}

FINLINE void rotate_ccw() {
	switch (selectedRotation) {
	case ROTATION2_0: selectedRotation = ROTATION2_270; break;
	case ROTATION2_90: selectedRotation = ROTATION2_0; break;
	case ROTATION2_180: selectedRotation = ROTATION2_90; break;
	default: selectedRotation = ROTATION2_180; break;
	}
}

FINLINE I32 item_screen_size() {
	return max(itemSize * scale, 1);
}

FINLINE I32 items_per_row() {
	I32 usableWidth = Win32::framebufferWidth - borderPadding * 2 - borderSize * 2;
	return max(usableWidth / item_screen_size(), 1);
}

FINLINE I32 row_count() {
	I32 perRow = items_per_row();
	return I32(BRUSH_COUNT / U32(perRow)) + ((BRUSH_COUNT % U32(perRow)) > 0 ? 1 : 0);
}

FINLINE void close_ui() {
	tilesheetVisible = B32_FALSE;
}

FINLINE void open_ui() {
	SelectUI::open = B32_FALSE;
	tilesheetVisible = B32_TRUE;
}

FINLINE void toggle_ui() {
	if (tilesheetVisible) {
		close_ui();
	}
	else {
		open_ui();
	}
}

FINLINE Resources::Sprite* preview_sprite(CreativeBrush brush, Rotation2 orientation) {
	switch (brush) {
	case CreativeBrush::TASK_SELECT:
	case CreativeBrush::ERASE:
		return nullptr;
	case CreativeBrush::CONVEYOR:
		switch (orientation) {
		case ROTATION2_90: return &Resources::tile.belt.downToUp;
		case ROTATION2_180: return &Resources::tile.belt.rightToLeft;
		case ROTATION2_270: return &Resources::tile.belt.upToDown;
		default: return &Resources::tile.belt.leftToRight;
		}
	case CreativeBrush::ASSEMBLER_SMALL:
		switch (orientation) {
		default: return &Resources::tile.furnace;
		}
	case CreativeBrush::ASSEMBLER_LARGE:
		switch (orientation) {
		case ROTATION2_90: return &Resources::tile.assembler.leftOff;
		case ROTATION2_180: return &Resources::tile.assembler.upOff;
		case ROTATION2_270: return &Resources::tile.assembler.rightOff;
		default: return &Resources::tile.assembler.downOff;
		}
	case CreativeBrush::HIVE_SMALL:
		return &Resources::tile.hive;
	case CreativeBrush::HIVE_BIG:
		return &Resources::tile.hiveLarge;
	default:
		return BeeDemo::creative_brush_sprite(brush);
	}
}

FINLINE V2U preview_footprint_tiles(CreativeBrush brush) {
	switch (brush) {
	case CreativeBrush::ASSEMBLER_SMALL:
		return V2U{ 1, 1 };
	case CreativeBrush::ASSEMBLER_LARGE:
	case CreativeBrush::HIVE_BIG:
		return V2U{ 2, 2 };
	case CreativeBrush::ASSEMBLER_VERY_LARGE:
		return V2U{ 3,2 };
	default:
		return V2U{ 1, 1 };
	}
}

FINLINE void init_ui() {
	selections.clear();
	for (U32 i = 0; i < BRUSH_COUNT; i++) {
		selections.push_back(brush_icon(brushOrder[i]));
	}
	selectedItem = 0;
	selectedBrush = brushOrder[0];
	selectedBrushFreePlacement = B32_FALSE;
	selectedRotation = ROTATION2_0;
	tilesheetVisible = B32_FALSE;
}

FINLINE void draw_rotation_badge(I32 panelX, I32 panelY, I32 panelW) {
	if (!brush_uses_rotation(selectedBrush)) {
		return;
	}

	RGBA8 badgeColor = RGBA8{ 220, 220, 80, 255 };
	switch (selectedRotation) {
	case ROTATION2_0:   badgeColor = RGBA8{ 220, 220, 80, 255 }; break;
	case ROTATION2_90:  badgeColor = RGBA8{ 80, 220, 220, 255 }; break;
	case ROTATION2_180: badgeColor = RGBA8{ 220, 120, 80, 255 }; break;
	case ROTATION2_270: badgeColor = RGBA8{ 180, 80, 220, 255 }; break;
	}

	I32 size = 10;
	Graphics::box(panelX + panelW - size - 4, panelY + 4, size, size, 1, RGBA8{ 0, 0, 0, 255 }, badgeColor);
}

void render_ui() {
	if (!tilesheetVisible) return;
	I32 itemScreenSize = item_screen_size();
	I32 perRow = items_per_row();
	I32 rows = max(row_count(), 1);
	I32 panelW = perRow * itemScreenSize + borderSize * 2;
	I32 panelH = rows * itemScreenSize + borderSize * 2;
	I32 panelX = max(Win32::framebufferWidth - borderPadding - panelW, 0);
	I32 panelY = borderPadding;
	I32 beginX = panelX + borderSize;
	I32 beginY = panelY + borderSize;

	Graphics::box(panelX, panelY, panelW, panelH, borderSize, borderColor, fillColor);
	for (U32 i = 0; i < BRUSH_COUNT; i++) {
		Resources::Sprite* sprite = selections[i];
		if (!sprite) continue;
		I32 x = beginX + I32(i % U32(perRow)) * itemScreenSize;
		I32 y = beginY + I32(i / U32(perRow)) * itemScreenSize;
		SelectUI::draw_sprite_in_cell(*sprite, x, y, itemScreenSize);
		if (brushOrder[i] == CreativeBrush::ERASE) {
			Graphics::border(x, y, itemScreenSize, itemScreenSize, 2, RGBA8{ 180, 80, 80, 255 });
		}
	}

	Graphics::border(
		beginX + (selectedItem % perRow) * itemScreenSize,
		beginY + (selectedItem / perRow) * itemScreenSize,
		itemScreenSize, itemScreenSize, selectBorderSize, selectedColor
	);
	draw_rotation_badge(panelX, panelY, panelW);
}

B32 handle_ui_click(V2F32 mousePos) {
	if (!tilesheetVisible) return B32_FALSE;
	V2I32 mouse = { (I32)mousePos.x, (I32)mousePos.y };
	I32 itemScreenSize = item_screen_size();
	I32 perRow = items_per_row();
	I32 rows = max(row_count(), 1);
	I32 panelW = perRow * itemScreenSize + borderSize * 2;
	I32 panelX = max(Win32::framebufferWidth - borderPadding - panelW, 0);
	I32 panelY = borderPadding;
	I32 beginX = panelX + borderSize;
	I32 beginY = panelY + borderSize;

	mouse.x -= beginX;
	mouse.y -= beginY;

	I32 interactableBoxW = perRow * itemScreenSize;
	I32 interactableBoxH = rows * itemScreenSize;
	if (!(mouse.x >= 0 && mouse.x < interactableBoxW && mouse.y >= 0 && mouse.y < interactableBoxH)) {
		return B32_FALSE;
	}

	mouse.x /= itemScreenSize;
	mouse.y /= itemScreenSize;
	I32 index = mouse.x + mouse.y * perRow;
	if (index < 0 || U32(index) >= BRUSH_COUNT) {
		return B32_TRUE;
	}

	set_selected_brush(brushOrder[U32(index)], B32_TRUE);
	close_ui();
	return B32_TRUE;
}

FINLINE void blit_sprite_cutout_blended(Resources::Sprite& sprite, I32 x, I32 y, I32 scaleFactor, I32 animFrame, U8 alpha) {
	if (alpha == 0) {
		return;
	}
	I32 dstX = max(x, 0);
	I32 dstY = max(y, 0);
	I32 srcX = (x >= 0 ? 0 : -x) + (I32(sprite.x) + animFrame * I32(sprite.width)) * scaleFactor;
	I32 srcY = (y >= 0 ? 0 : -y) + I32(sprite.y) * scaleFactor;
	I32 sizeX = min<I32>(x + I32(sprite.width) * scaleFactor, Win32::framebufferWidth) - dstX;
	I32 sizeY = min<I32>(y + I32(sprite.height) * scaleFactor, Win32::framebufferHeight) - dstY;
	if (sizeX <= 0 || sizeY <= 0) {
		return;
	}

	for (I32 blitY = 0; blitY < sizeY; blitY++) {
		RGBA8* src = &sprite.tex->pixels[((blitY + srcY) / scaleFactor) * sprite.tex->width];
		RGBA8* dst = &Win32::framebuffer[(blitY + dstY) * Win32::framebufferWidth] + dstX;
		for (I32 blitX = 0; blitX < sizeX; blitX++) {
			RGBA8 srcPx = src[(srcX + blitX) / scaleFactor];
			if (srcPx.a == 0) {
				continue;
			}
			U32 srcA = (U32(srcPx.a) * U32(alpha)) / 255u;
			if (srcA == 0) {
				continue;
			}
			U32 invA = 255u - srcA;
			RGBA8 dstPx = dst[blitX];
			dst[blitX].r = U8((U32(dstPx.r) * invA + U32(srcPx.r) * srcA) / 255u);
			dst[blitX].g = U8((U32(dstPx.g) * invA + U32(srcPx.g) * srcA) / 255u);
			dst[blitX].b = U8((U32(dstPx.b) * invA + U32(srcPx.b) * srcA) / 255u);
			dst[blitX].a = 255;
		}
	}
}

FINLINE void draw_preview_border(I32 x, I32 y, I32 width, I32 height, RGBA8 color) {
	BeeDemo::fill_rect(x, y, width, 1, color);
	BeeDemo::fill_rect(x, y + height - 1, width, 1, color);
	BeeDemo::fill_rect(x, y, 1, height, color);
	BeeDemo::fill_rect(x + width - 1, y, 1, height, color);
}

FINLINE void draw_rotation_marker(I32 x, I32 y, I32 width, I32 height, Rotation2 orientation) {
	I32 thickness = max(min(width, height) / 10, 2);
	I32 markerLen = max(min(width, height) / 3, thickness + 2);
	RGBA8 color = RGBA8{ 255, 235, 90, 210 };
	BeeDemo::fill_rect_blended(x + thickness, y + thickness, width - thickness * 2, height - thickness * 2, RGBA8{ 255, 255, 255, 10 });
	switch (orientation) {
	case ROTATION2_0:
		BeeDemo::fill_rect_blended(x + width / 2 - thickness / 2, y + height - markerLen, thickness, markerLen - thickness, color);
		BeeDemo::fill_rect_blended(x + width / 2 - markerLen / 2, y + height - thickness, markerLen, thickness, color);
		break;
	case ROTATION2_90:
		BeeDemo::fill_rect_blended(x, y + height / 2 - markerLen / 2, thickness, markerLen, color);
		BeeDemo::fill_rect_blended(x, y + height / 2 - thickness / 2, markerLen, thickness, color);
		break;
	case ROTATION2_180:
		BeeDemo::fill_rect_blended(x + width / 2 - thickness / 2, y, thickness, markerLen, color);
		BeeDemo::fill_rect_blended(x + width / 2 - markerLen / 2, y, markerLen, thickness, color);
		break;
	case ROTATION2_270:
		BeeDemo::fill_rect_blended(x + width - markerLen, y + height / 2 - thickness / 2, markerLen, thickness, color);
		BeeDemo::fill_rect_blended(x + width - thickness, y + height / 2 - markerLen / 2, thickness, markerLen, color);
		break;
	}
}

void render_world_preview(V2F32 currentCamera, I32 currentWorldTileScale, F64 currentFrameTime) {
	if (tilesheetVisible || SelectUI::open) {
		return;
	}
	if (selectedBrush == CreativeBrush::TASK_SELECT) {
		return;
	}

	V2U32 hoveredTile{};
	if (!mouse_to_tile(&hoveredTile)) {
		return;
	}

	V2U footprint = preview_footprint_tiles(selectedBrush);
	I32 tilePixels = 16 * currentWorldTileScale;
	V2F32 screenTopLeftF = TileSpace::tile_to_world(hoveredTile) * F32(tilePixels) - currentCamera;
	I32 screenX = I32(roundf32(screenTopLeftF.x));
	I32 screenY = I32(roundf32(screenTopLeftF.y));
	I32 previewWidth = I32(footprint.x) * tilePixels;
	I32 previewHeight = I32(footprint.y) * tilePixels;

	RGBA8 fillTint = RGBA8{ 255, 255, 255, 36 };
	RGBA8 borderTint = RGBA8{ 255, 255, 160, 180 };
	if (selectedBrush == CreativeBrush::ERASE) {
		fillTint = RGBA8{ 220, 80, 80, 48 };
		borderTint = RGBA8{ 255, 120, 120, 220 };
	}
	else if (brush_uses_rotation(selectedBrush)) {
		fillTint = RGBA8{ 120, 180, 255, 40 };
		borderTint = RGBA8{ 160, 220, 255, 220 };
	}

	BeeDemo::fill_rect_blended(screenX, screenY, previewWidth, previewHeight, fillTint);
	for (U32 y = 1; y < footprint.y; y++) {
		BeeDemo::fill_rect(screenX, screenY + I32(y) * tilePixels, previewWidth, 1, RGBA8{ 255, 255, 255, 120 });
	}
	for (U32 x = 1; x < footprint.x; x++) {
		BeeDemo::fill_rect(screenX + I32(x) * tilePixels, screenY, 1, previewHeight, RGBA8{ 255, 255, 255, 120 });
	}
	Resources::Sprite* sprite = preview_sprite(selectedBrush, selectedRotation);
	if (sprite) {
		I32 animFrame = 0;
		if (sprite->animFrames > 1) {
			animFrame = I32(currentFrameTime * 6.0) % I32(sprite->animFrames);
		}
		I32 spriteWidthPx = I32(sprite->width) * currentWorldTileScale;
		I32 spriteHeightPx = I32(sprite->height) * currentWorldTileScale;
		I32 drawX = screenX + (previewWidth - spriteWidthPx) / 2;
		I32 drawY = screenY + previewHeight - spriteHeightPx;
		blit_sprite_cutout_blended(*sprite, drawX, drawY, currentWorldTileScale, animFrame, 140);
	}
	draw_preview_border(screenX, screenY, previewWidth, previewHeight, borderTint);
	if (brush_uses_rotation(selectedBrush)) {
		draw_rotation_marker(screenX, screenY, previewWidth, previewHeight, selectedRotation);
	}
}

}
