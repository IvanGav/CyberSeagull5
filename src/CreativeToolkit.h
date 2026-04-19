#pragma once

#include "Win32.h"
#include "Resources.h"
#include "Graphics.h"
#include "TileSpace.h"
#include "BeeDemo.h"

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

static constexpr I32 TILESET_PANEL_PADDING = 12;

struct RectI {
	I32 x = 0;
	I32 y = 0;
	I32 w = 0;
	I32 h = 0;
};

CreativeBrush selectedBrush = CreativeBrush::TASK_SELECT;
B32 tilesheetVisible = B32_FALSE;

B32 point_in_rect(V2F32 point, const RectI& rect) {
	return point.x >= F32(rect.x) && point.y >= F32(rect.y) && point.x < F32(rect.x + rect.w) && point.y < F32(rect.y + rect.h) ? B32_TRUE : B32_FALSE;
}

I32 tileset_picker_scale() {
	I32 maxWidth = max(Win32::framebufferWidth - 160, 64);
	I32 maxHeight = max(Win32::framebufferHeight - 120, 64);
	I32 scaleX = maxWidth / I32(Resources::tileset.width);
	I32 scaleY = maxHeight / I32(Resources::tileset.height);
	return clamp(min(scaleX, scaleY), 1, 4);
}

RectI tileset_texture_rect() {
	I32 scale = tileset_picker_scale();
	RectI rect{};
	rect.w = I32(Resources::tileset.width) * scale;
	rect.h = I32(Resources::tileset.height) * scale;
	rect.x = max((Win32::framebufferWidth - rect.w) / 2, 0);
	rect.y = max((Win32::framebufferHeight - rect.h) / 2 - 16, 0);
	return rect;
}

RectI tileset_panel_rect() {
	RectI tex = tileset_texture_rect();
	RectI panel{};
	panel.x = max(tex.x - TILESET_PANEL_PADDING, 0);
	panel.y = max(tex.y - TILESET_PANEL_PADDING, 0);
	panel.w = min(tex.w + TILESET_PANEL_PADDING * 2, Win32::framebufferWidth - panel.x);
	panel.h = min(tex.h + TILESET_PANEL_PADDING * 2, Win32::framebufferHeight - panel.y);
	return panel;
}

void draw_frame_rect(const RectI& rect, RGBA8 color, I32 thickness = 1) {
	BeeDemo::fill_rect(rect.x, rect.y, rect.w, thickness, color);
	BeeDemo::fill_rect(rect.x, rect.y + rect.h - thickness, rect.w, thickness, color);
	BeeDemo::fill_rect(rect.x, rect.y, thickness, rect.h, color);
	BeeDemo::fill_rect(rect.x + rect.w - thickness, rect.y, thickness, rect.h, color);
}

void highlight_tileset_cell(I32 cellX, I32 cellY, RGBA8 color) {
	RectI tex = tileset_texture_rect();
	I32 scale = tileset_picker_scale();
	RectI cellRect{};
	cellRect.x = tex.x + cellX * 16 * scale;
	cellRect.y = tex.y + cellY * 16 * scale;
	cellRect.w = 16 * scale;
	cellRect.h = 16 * scale;
	draw_frame_rect(cellRect, color, max(scale / 2, 1));
}

void render_tilesheet_picker() {
	BeeDemo::fill_rect_blended(0, 0, Win32::framebufferWidth, Win32::framebufferHeight, RGBA8{ 0, 0, 0, 140 });
	RectI panel = tileset_panel_rect();
	RectI tex = tileset_texture_rect();
	BeeDemo::fill_rect(panel.x, panel.y, panel.w, panel.h, RGBA8{ 26, 26, 26, 255 });
	draw_frame_rect(panel, RGBA8{ 120, 120, 120, 255 });
	Graphics::blit_texture(Resources::tileset, tex.x, tex.y, tileset_picker_scale());

	highlight_tileset_cell(0, 10, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(1, 0, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(2, 0, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(2, 1, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(2, 2, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(0, 1, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(1, 1, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(0, 2, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(4, 0, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(1, 2, RGBA8{ 120, 220, 120, 255 });
	highlight_tileset_cell(0, 4, RGBA8{ 120, 220, 120, 255 });

	Resources::Sprite* selectedSprite = BeeDemo::creative_brush_sprite(selectedBrush);
	if (selectedBrush == CreativeBrush::TASK_SELECT) highlight_tileset_cell(0, 10, RGBA8{ 255, 255, 80, 255 });
	if (selectedSprite == &Resources::tile.grass) highlight_tileset_cell(1, 0, RGBA8{ 255, 255, 80, 255 });
	if (selectedSprite == &Resources::tile.grassIron) highlight_tileset_cell(2, 0, RGBA8{ 255, 255, 80, 255 });
	if (selectedSprite == &Resources::tile.grassCopper) highlight_tileset_cell(2, 1, RGBA8{ 255, 255, 80, 255 });
	if (selectedSprite == &Resources::tile.grassFlowers) highlight_tileset_cell(2, 2, RGBA8{ 255, 255, 80, 255 });
	if (selectedSprite == &Resources::tile.sand) highlight_tileset_cell(0, 1, RGBA8{ 255, 255, 80, 255 });
	if (selectedSprite == &Resources::tile.beach) highlight_tileset_cell(1, 1, RGBA8{ 255, 255, 80, 255 });
	if (selectedSprite == &Resources::tile.water) highlight_tileset_cell(0, 2, RGBA8{ 255, 255, 80, 255 });
	if (selectedSprite == &Resources::tile.belt.leftToRight) highlight_tileset_cell(4, 0, RGBA8{ 255, 255, 80, 255 });
	if (selectedBrush == CreativeBrush::HIVE_SMALL) highlight_tileset_cell(1, 2, RGBA8{ 255, 255, 80, 255 });
	if (selectedBrush == CreativeBrush::HIVE_BIG) highlight_tileset_cell(0, 4, RGBA8{ 255, 255, 80, 255 });
}

void render_ui() {
	if (tilesheetVisible) {
		render_tilesheet_picker();
	}
}

B32 handle_tilesheet_click(V2F32 mouse) {
	RectI tex = tileset_texture_rect();
	if (!point_in_rect(mouse, tex)) {
		tilesheetVisible = B32_FALSE;
		return B32_TRUE;
	}
	I32 scale = tileset_picker_scale();
	I32 texX = (I32(mouse.x) - tex.x) / scale;
	I32 texY = (I32(mouse.y) - tex.y) / scale;
	I32 cellX = texX / 16;
	I32 cellY = texY / 16;
	selectedBrush = BeeDemo::creative_brush_from_tileset_cell(cellX, cellY, selectedBrush);
	tilesheetVisible = B32_FALSE;
	return B32_TRUE;
}

}
