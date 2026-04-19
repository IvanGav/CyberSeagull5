#pragma once

#include "CreativeToolkit.h"

namespace Cyber5eagull::EditorInteraction {

using BeeDemo::CreativeBrush;

B32 cameraDragActive = B32_FALSE;
B32 uiLeftCapture = B32_FALSE;
B32 hasLastDraggedTile = B32_FALSE;
V2U32 lastDraggedTile{};

void reset_drag_state() {
	cameraDragActive = B32_FALSE;
	uiLeftCapture = B32_FALSE;
	hasLastDraggedTile = B32_FALSE;
}

void apply_drag_brush(CreativeBrush brush) {
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

void apply_task_unassign() {
	V2U32 hoveredTile{};
	if (!mouse_to_tile(&hoveredTile)) {
		return;
	}
	if (hasLastDraggedTile && TileSpace::same_tile(hoveredTile, lastDraggedTile)) {
		return;
	}
	lastDraggedTile = hoveredTile;
	hasLastDraggedTile = B32_TRUE;
	BeeDemo::unqueue_tile_task(hoveredTile);
}

void update_drag_interactions() {
	V2F32 rawMouseDelta = Win32::get_raw_delta_mouse();
	B32 shiftHeld = Win32::keyboardState[Win32::KEY_SHIFT] ? B32_TRUE : B32_FALSE;
	B32 leftHeld = Win32::mouseButtonState[Win32::MOUSE_BUTTON_LEFT] ? B32_TRUE : B32_FALSE;
	B32 rightHeld = Win32::mouseButtonState[Win32::MOUSE_BUTTON_RIGHT] ? B32_TRUE : B32_FALSE;

	if (!leftHeld) {
		uiLeftCapture = B32_FALSE;
	}

	if (shiftHeld && leftHeld && !uiLeftCapture && !CreativeToolkit::tilesheetVisible) {
		cameraDragActive = B32_TRUE;
		hasLastDraggedTile = B32_FALSE;
		camera -= rawMouseDelta;
		clamp_camera();
		return;
	}

	cameraDragActive = B32_FALSE;
	if (shiftHeld || CreativeToolkit::tilesheetVisible || uiLeftCapture) {
		if (!leftHeld && !rightHeld) {
			hasLastDraggedTile = B32_FALSE;
		}
		return;
	}

	if (leftHeld) {
		apply_drag_brush(CreativeToolkit::selectedBrush);
	}
	else if (rightHeld) {
		if (CreativeToolkit::selectedBrush == CreativeBrush::TASK_SELECT) {
			apply_task_unassign();
		}
		else {
			apply_drag_brush(CreativeBrush::ERASE);
		}
	}
	else {
		hasLastDraggedTile = B32_FALSE;
	}
}

void keyboard_callback(Win32::Key key, Win32::ButtonState state) {
	if (state != Win32::BUTTON_STATE_DOWN) {
		return;
	}

	if (key == Win32::KEY_R && Win32::keyboardState[Win32::KEY_CTRL]) {
		BeeDemo::init(hiveTile);
		center_camera_on_tile(hiveTile);
		reset_drag_state();
		CreativeToolkit::selectedBrush = CreativeBrush::TASK_SELECT;
		return;
	}

	if (key == Win32::KEY_BACKTICK) {
		CreativeToolkit::tilesheetVisible = !CreativeToolkit::tilesheetVisible;
		hasLastDraggedTile = B32_FALSE;
		uiLeftCapture = B32_FALSE;
		return;
	}

	if (key == Win32::KEY_ESC) {
		CreativeToolkit::tilesheetVisible = B32_FALSE;
		uiLeftCapture = B32_FALSE;
		CreativeToolkit::selectedBrush = CreativeBrush::TASK_SELECT;
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
		if (CreativeToolkit::tilesheetVisible) {
			uiLeftCapture = CreativeToolkit::handle_tilesheet_click(mouse);
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
