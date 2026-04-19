#pragma once

#include "CreativeToolkit.h"

namespace Cyber5eagull::EditorInteraction {

using BeeDemo::CreativeBrush;

B32 cameraDragActive = B32_FALSE;
B32 uiLeftCapture = B32_FALSE;
B32 hasLastDraggedTile = B32_FALSE;
V2U32 lastDraggedTile{};

B32 conveyorDragActive = B32_FALSE;
B32 conveyorDragHasIncoming = B32_FALSE;
V2U32 conveyorLastTile{};
Direction2 conveyorLastInputSide = DIRECTION2_INVALID;

void reset_drag_state() {
	cameraDragActive = B32_FALSE;
	uiLeftCapture = B32_FALSE;
	hasLastDraggedTile = B32_FALSE;
	conveyorDragActive = B32_FALSE;
	conveyorDragHasIncoming = B32_FALSE;
	conveyorLastInputSide = DIRECTION2_INVALID;
}

Direction2 direction_from_to(V2U32 from, V2U32 to) {
	if (to.x == from.x + 1 && to.y == from.y) return DIRECTION2_RIGHT;
	if (from.x > 0 && to.x + 1 == from.x && to.y == from.y) return DIRECTION2_LEFT;
	if (to.y == from.y + 1 && to.x == from.x) return DIRECTION2_BACK;
	if (from.y > 0 && to.y + 1 == from.y && to.x == from.x) return DIRECTION2_FRONT;
	return DIRECTION2_INVALID;
}

void begin_conveyor_drag(V2U32 hoveredTile) {
	V2U tile{ hoveredTile };
	if (!Factory::has_machine(tile)) {
		BeeDemo::apply_creative_brush(CreativeBrush::CONVEYOR, hoveredTile, ROTATION2_0);
		if (!Factory::has_belt(tile)) {
			conveyorDragActive = B32_FALSE;
			conveyorDragHasIncoming = B32_FALSE;
			conveyorLastInputSide = DIRECTION2_INVALID;
			return;
		}
	}
	conveyorLastInputSide = DIRECTION2_INVALID;
	conveyorDragHasIncoming = B32_FALSE;
	conveyorDragActive = B32_TRUE;
	conveyorLastTile = hoveredTile;
}

void continue_conveyor_drag(V2U32 hoveredTile) {
	Direction2 newDirection = direction_from_to(conveyorLastTile, hoveredTile);
	if (newDirection == DIRECTION2_INVALID) {
		return;
	}

	V2U nextTile{ hoveredTile.x, hoveredTile.y };

	Direction2 previousInput = conveyorDragHasIncoming ? conveyorLastInputSide : Factory::opposite_direction(newDirection);
	if (previousInput == newDirection) {
		return;
	}

	V2U previousTile{ conveyorLastTile.x, conveyorLastTile.y };
	Factory::set_belt_shape(previousTile, previousInput, newDirection);

	Direction2 nextInput = Factory::opposite_direction(newDirection);
	Direction2 nextOutput = newDirection;

	if (!Factory::set_belt_shape(nextTile, nextInput, nextOutput)) {
		return;
	}

	conveyorLastTile = hoveredTile;
	conveyorLastInputSide = nextInput;
	conveyorDragHasIncoming = conveyorLastInputSide != DIRECTION2_INVALID ? B32_TRUE : B32_FALSE;
}

void apply_conveyor_drag() {
	V2U32 hoveredTile{};
	if (!mouse_to_tile(&hoveredTile)) {
		return;
	}
	if (hasLastDraggedTile && TileSpace::same_tile(hoveredTile, lastDraggedTile)) {
		return;
	}
	lastDraggedTile = hoveredTile;
	hasLastDraggedTile = B32_TRUE;

	if (!conveyorDragActive) {
		begin_conveyor_drag(hoveredTile);
		return;
	}

	continue_conveyor_drag(hoveredTile);
}

void apply_drag_brush(CreativeBrush brush) {
	if (brush == CreativeBrush::CONVEYOR) {
		apply_conveyor_drag();
		return;
	}

	V2U32 hoveredTile{};
	if (!mouse_to_tile(&hoveredTile)) {
		return;
	}
	if (hasLastDraggedTile && TileSpace::same_tile(hoveredTile, lastDraggedTile)) {
		return;
	}
	lastDraggedTile = hoveredTile;
	hasLastDraggedTile = B32_TRUE;
	BeeDemo::apply_creative_brush(brush, hoveredTile, ROTATION2_0);
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
		conveyorDragActive = B32_FALSE;
		camera -= rawMouseDelta;
		clamp_camera();
		return;
	}

	cameraDragActive = B32_FALSE;
	if (shiftHeld || CreativeToolkit::tilesheetVisible || uiLeftCapture) {
		if (!leftHeld && !rightHeld) {
			hasLastDraggedTile = B32_FALSE;
			conveyorDragActive = B32_FALSE;
		}
		return;
	}

	if (leftHeld) {
		apply_drag_brush(CreativeToolkit::selectedBrush);
	}
	else if (rightHeld) {
		conveyorDragActive = B32_FALSE;
		if (CreativeToolkit::selectedBrush == CreativeBrush::TASK_SELECT) {
			apply_task_unassign();
		}
		else {
			apply_drag_brush(CreativeBrush::ERASE);
		}
	}
	else {
		hasLastDraggedTile = B32_FALSE;
		conveyorDragActive = B32_FALSE;
	}
}

void keyboard_callback(Win32::Key key, Win32::ButtonState state) {
	if (state != Win32::BUTTON_STATE_DOWN) {
		return;
	}

	if (key == Win32::KEY_I) {
		SelectUI::open = !SelectUI::open;
		hasLastDraggedTile = B32_FALSE;
		uiLeftCapture = B32_FALSE;
		conveyorDragActive = B32_FALSE;
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
		conveyorDragActive = B32_FALSE;
		return;
	}

	if (key == Win32::KEY_ESC) {
		CreativeToolkit::tilesheetVisible = B32_FALSE;
		uiLeftCapture = B32_FALSE;
		CreativeToolkit::selectedBrush = CreativeBrush::TASK_SELECT;
		conveyorDragActive = B32_FALSE;
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
		if (SelectUI::open) {
			uiLeftCapture = SelectUI::click_callback(Win32::get_mouse());
		}
		if (CreativeToolkit::tilesheetVisible) {
			uiLeftCapture = CreativeToolkit::handle_tilesheet_click(mouse);
			return;
		}
		V2U tilePos;
		if (mouse_to_tile(&tilePos) && Factory::machine_is_belt(Factory::get_machine_from_tile(tilePos))) {
			Factory::get_machine_from_tile(tilePos)->inventory[0].count = 1;
			Factory::get_machine_from_tile(tilePos)->inventory[0].item = Inventory::ITEM_IRON_PLATE;
		}
	}

	if ((button == Win32::MOUSE_BUTTON_LEFT || button == Win32::MOUSE_BUTTON_RIGHT) && state.state == Win32::BUTTON_STATE_UP) {
		hasLastDraggedTile = B32_FALSE;
		conveyorDragActive = B32_FALSE;
		if (button == Win32::MOUSE_BUTTON_LEFT) {
			cameraDragActive = B32_FALSE;
			uiLeftCapture = B32_FALSE;
		}
	}
}

}
