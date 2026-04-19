#pragma once

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
B32 cameraDragActive = B32_FALSE;
B32 uiLeftCapture = B32_FALSE;
B32 hasLastDraggedTile = B32_FALSE;
V2U32 lastDraggedTile{};

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

void reset_drag_state() {
	cameraDragActive = B32_FALSE;
	uiLeftCapture = B32_FALSE;
	hasLastDraggedTile = B32_FALSE;
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
	if (selectedSprite == &Resources::tile.belt.right) highlight_tileset_cell(4, 0, RGBA8{ 255, 255, 80, 255 });
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
	CreativeBrush clickedBrush = BeeDemo::creative_brush_from_tileset_cell(cellX, cellY, selectedBrush);
	selectedBrush = clickedBrush;
	tilesheetVisible = B32_FALSE;
	return B32_TRUE;
}

void apply_drag_action(CreativeBrush brush) {
	V2U32 hoveredTile{};
	if (!mouse_to_tile(&hoveredTile)) {
		return;
	}
	if (hasLastDraggedTile && TileSpace::same_tile(hoveredTile, lastDraggedTile)) {
		return;
	}
	lastDraggedTile = hoveredTile;
	hasLastDraggedTile = B32_TRUE;
	BeeDemo::apply_creative_brush(brush, hoveredTile);
}

void update_drag_interactions() {
	V2F32 rawMouseDelta = Win32::get_raw_delta_mouse();
	B32 shiftHeld = Win32::keyboardState[Win32::KEY_SHIFT] ? B32_TRUE : B32_FALSE;
	B32 leftHeld = Win32::mouseButtonState[Win32::MOUSE_BUTTON_LEFT] ? B32_TRUE : B32_FALSE;
	B32 rightHeld = Win32::mouseButtonState[Win32::MOUSE_BUTTON_RIGHT] ? B32_TRUE : B32_FALSE;

	if (!leftHeld) {
		uiLeftCapture = B32_FALSE;
	}

	if (shiftHeld && leftHeld && !uiLeftCapture && !tilesheetVisible) {
		cameraDragActive = B32_TRUE;
		hasLastDraggedTile = B32_FALSE;
		camera -= rawMouseDelta;
		clamp_camera();
		return;
	}

	cameraDragActive = B32_FALSE;
	if (shiftHeld || tilesheetVisible || uiLeftCapture) {
		if (!leftHeld && !rightHeld) {
			hasLastDraggedTile = B32_FALSE;
		}
		return;
	}

	if (leftHeld) {
		apply_drag_action(selectedBrush);
	}
	else if (rightHeld) {
		apply_drag_action(CreativeBrush::ERASE);
	}
	else {
		hasLastDraggedTile = B32_FALSE;
	}
}

void keyboard_callback(Win32::Key key, Win32::ButtonState state) {
	if (state != Win32::BUTTON_STATE_DOWN) {
		return;
	}

	if (key == Win32::KEY_R) {
		BeeDemo::init(hiveTile);
		center_camera_on_tile(hiveTile);
		reset_drag_state();
		selectedBrush = CreativeBrush::TASK_SELECT;
		return;
	}

	if (key == Win32::KEY_BACKTICK) {
		tilesheetVisible = !tilesheetVisible;
		hasLastDraggedTile = B32_FALSE;
		uiLeftCapture = B32_FALSE;
		return;
	}

	if (key == Win32::KEY_ESC) {
		tilesheetVisible = B32_FALSE;
		uiLeftCapture = B32_FALSE;
		selectedBrush = CreativeBrush::TASK_SELECT;
	}
}

void mouse_callback(Win32::MouseButton button, Win32::MouseValue state) {
	if (button == Win32::MOUSE_BUTTON_WHEEL) {
		I32 wheelDirection = state.scroll > 0.0F ? -1 : (state.scroll < 0.0F ? 1 : 0);
		if (wheelDirection != 0) {
			if (Win32::keyboardState[Win32::KEY_SHIFT]) {
				pan_camera_horizontally(wheelDirection);
			}
			else {
				zoom_camera_at_screen(-wheelDirection, Win32::get_mouse());
			}
		}
		return;
	}

	if (button == Win32::MOUSE_BUTTON_LEFT && state.state == Win32::BUTTON_STATE_DOWN) {
		V2F32 mouse = Win32::get_mouse();
		if (tilesheetVisible) {
			uiLeftCapture = handle_tilesheet_click(mouse);
			return;
		}
	}

	if ((button == Win32::MOUSE_BUTTON_LEFT || button == Win32::MOUSE_BUTTON_RIGHT) && state.state == Win32::BUTTON_STATE_UP) {
		hasLastDraggedTile = B32_FALSE;
		if (button == Win32::MOUSE_BUTTON_LEFT) {
			cameraDragActive = B32_FALSE;
			uiLeftCapture = B32_FALSE;
		}
	}
}

}
