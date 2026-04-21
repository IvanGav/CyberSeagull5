#pragma once

#include "CreativeToolkit.h"
#include "SelectUI.h"

namespace Cyber5eagull::EditorInteraction {

namespace BeeDemoNS = Cyber5eagull::BeeDemo;
using CreativeBrush = BeeDemoNS::CreativeBrush;

inline void fill_rect_blended(I32 x, I32 y, I32 width, I32 height, RGBA8 color) {
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

namespace Detail {
inline void apply_creative_brush_dispatch(CreativeBrush brush, V2U32 tile, Rotation2 orientation) {
	BeeDemoNS::apply_creative_brush(brush, tile, orientation, CreativeToolkit::selectedBrushFreePlacement);
}
}

B32 cameraDragActive = B32_FALSE;
B32 uiLeftCapture = B32_FALSE;
B32 hasLastDraggedTile = B32_FALSE;
V2U32 lastDraggedTile{};

B32 conveyorDragActive = B32_FALSE;
B32 conveyorDragHasIncoming = B32_FALSE;
V2U32 conveyorLastTile{};
Direction2 conveyorLastInputSide = DIRECTION2_INVALID;
B32 itemBuildMenuVisible = B32_FALSE;
B32 suppressRightDragUntilRelease = B32_FALSE;

enum class BuildMenuEntryType : U8 {
	ENTRY_BRUSH = 0,
	ENTRY_BUY_BEE,
};

struct BuildMenuEntry {
	BuildMenuEntryType type = BuildMenuEntryType::ENTRY_BRUSH;
	CreativeBrush brush = CreativeBrush::TASK_SELECT;
};

BuildMenuEntry buildMenuEntries[] = {
	{ BuildMenuEntryType::ENTRY_BRUSH, CreativeBrush::CONVEYOR },
	{ BuildMenuEntryType::ENTRY_BUY_BEE, CreativeBrush::TASK_SELECT },
	{ BuildMenuEntryType::ENTRY_BRUSH, CreativeBrush::ASSEMBLER_SMALL },
	{ BuildMenuEntryType::ENTRY_BRUSH, CreativeBrush::ASSEMBLER_LARGE },
	{ BuildMenuEntryType::ENTRY_BRUSH, CreativeBrush::ASSEMBLER_VERY_LARGE },
	{ BuildMenuEntryType::ENTRY_BRUSH, CreativeBrush::SPLITTER },
	{ BuildMenuEntryType::ENTRY_BRUSH, CreativeBrush::HIVE_SMALL },
	{ BuildMenuEntryType::ENTRY_BRUSH, CreativeBrush::HIVE_BIG },
};
static constexpr U32 BUILD_MENU_ITEM_COUNT = ARRAY_COUNT(buildMenuEntries);

struct BuildMenuLayout {
	I32 x = 0;
	I32 y = 0;
	I32 width = 0;
	I32 height = 0;
	I32 cols = 0;
	I32 rows = 0;
	I32 cellSize = 0;
	I32 beginX = 0;
	I32 beginY = 0;
};

void reset_drag_state() {
	cameraDragActive = B32_FALSE;
	uiLeftCapture = B32_FALSE;
	hasLastDraggedTile = B32_FALSE;
	conveyorDragActive = B32_FALSE;
	conveyorDragHasIncoming = B32_FALSE;
	conveyorLastInputSide = DIRECTION2_INVALID;
	suppressRightDragUntilRelease = B32_FALSE;
}

FINLINE I32 build_menu_base_item_screen_size() {
	return CreativeToolkit::item_screen_size();
}

FINLINE I32 build_menu_cell_size() {
	return max(build_menu_base_item_screen_size() + 24, 96);
}

FINLINE I32 build_menu_items_per_row() {
	I32 usableWidth = Win32::framebufferWidth - CreativeToolkit::borderPadding * 2 - CreativeToolkit::borderSize * 2;
	I32 perRow = max(usableWidth / build_menu_cell_size(), 1);
	return clamp(perRow, 1, I32(BUILD_MENU_ITEM_COUNT));
}

FINLINE I32 build_menu_row_count() {
	I32 perRow = build_menu_items_per_row();
	return I32(BUILD_MENU_ITEM_COUNT / U32(perRow)) + ((BUILD_MENU_ITEM_COUNT % U32(perRow)) > 0 ? 1 : 0);
}

FINLINE BuildMenuLayout build_menu_layout() {
	BuildMenuLayout layout{};
	layout.cols = build_menu_items_per_row();
	layout.rows = max(build_menu_row_count(), 1);
	layout.cellSize = build_menu_cell_size();
	layout.width = layout.cols * layout.cellSize + CreativeToolkit::borderSize * 2;
	layout.height = layout.rows * layout.cellSize + CreativeToolkit::borderSize * 2;
	layout.x = max(Win32::framebufferWidth - CreativeToolkit::borderPadding - layout.width, 0);
	layout.y = CreativeToolkit::borderPadding;
	layout.beginX = layout.x + CreativeToolkit::borderSize;
	layout.beginY = layout.y + CreativeToolkit::borderSize;
	return layout;
}

FINLINE B32 build_menu_contains(V2F32 mousePos) {
	if (!itemBuildMenuVisible) {
		return B32_FALSE;
	}
	BuildMenuLayout layout = build_menu_layout();
	I32 mouseX = I32(mousePos.x);
	I32 mouseY = I32(mousePos.y);
	return mouseX >= layout.x && mouseX < layout.x + layout.width && mouseY >= layout.y && mouseY < layout.y + layout.height ? B32_TRUE : B32_FALSE;
}

FINLINE I32 build_menu_index_at(V2F32 mousePos) {
	if (!build_menu_contains(mousePos)) {
		return -1;
	}
	BuildMenuLayout layout = build_menu_layout();
	I32 localX = I32(mousePos.x) - layout.beginX;
	I32 localY = I32(mousePos.y) - layout.beginY;
	if (localX < 0 || localY < 0) {
		return -1;
	}
	I32 col = localX / layout.cellSize;
	I32 row = localY / layout.cellSize;
	if (col < 0 || col >= layout.cols || row < 0 || row >= layout.rows) {
		return -1;
	}
	I32 index = col + row * layout.cols;
	return index >= 0 && U32(index) < BUILD_MENU_ITEM_COUNT ? index : -1;
}

FINLINE void close_item_build_menu() {
	Factory::recipeMenuMachine.machine = nullptr;
	itemBuildMenuVisible = B32_FALSE;
}

FINLINE void open_item_build_menu() {
	itemBuildMenuVisible = B32_TRUE;
	CreativeToolkit::close_ui();
	SelectUI::open = B32_FALSE;
}

FINLINE void toggle_item_build_menu() {
	if (itemBuildMenuVisible) {
		close_item_build_menu();
	}
	else {
		open_item_build_menu();
	}
}

FINLINE Resources::Sprite* build_menu_sprite(const BuildMenuEntry& entry) {
	if (entry.type == BuildMenuEntryType::ENTRY_BUY_BEE) {
		return &Resources::tile.beeFly;
	}
	return CreativeToolkit::brush_icon(entry.brush);
}

FINLINE B32 build_menu_selected(const BuildMenuEntry& entry) {
	if (entry.type != BuildMenuEntryType::ENTRY_BRUSH) {
		return B32_FALSE;
	}
	return CreativeToolkit::selectedBrush == entry.brush ? B32_TRUE : B32_FALSE;
}

FINLINE U32 build_menu_affordable_count(const BuildMenuEntry& entry) {
	if (entry.type == BuildMenuEntryType::ENTRY_BUY_BEE) {
		return BeeDemoNS::bee_purchase_available_count();
	}
	return BeeDemoNS::build_available_count(entry.brush);
}

FINLINE U32 build_menu_badge_count(const BuildMenuEntry& entry) {
	if (entry.type == BuildMenuEntryType::ENTRY_BUY_BEE) {
		return BeeDemoNS::total_bee_count();
	}
	if (entry.brush == CreativeBrush::CONVEYOR) {
		return Inventory::count(Inventory::ITEM_CONVEYOR);
	}
	return BeeDemoNS::build_available_count(entry.brush);
}

FINLINE BeeDemoNS::BuildCostDef build_menu_cost_def(const BuildMenuEntry& entry) {
	return entry.type == BuildMenuEntryType::ENTRY_BUY_BEE ? BeeDemoNS::bee_purchase_cost_def() : BeeDemoNS::build_cost_def(entry.brush);
}

FINLINE U32 build_menu_hotkey_number(U32 index) {
	return index + 1u;
}

FINLINE I32 decimal_digit_count(U32 value) {
	I32 digits = 1;
	while (value >= 10u) {
		value /= 10u;
		digits++;
	}
	return digits;
}

FINLINE B32 build_menu_key_to_index(Win32::Key key, U32* indexOut) {
	if (key < Win32::KEY_1 || key > Win32::KEY_9) {
		return B32_FALSE;
	}
	U32 index = U32(key - Win32::KEY_1);
	if (index >= BUILD_MENU_ITEM_COUNT) {
		return B32_FALSE;
	}
	if (indexOut) {
		*indexOut = index;
	}
	return B32_TRUE;
}

void select_build_menu_index(I32 index) {
	if (index < 0 || U32(index) >= BUILD_MENU_ITEM_COUNT) {
		return;
	}
	const BuildMenuEntry& entry = buildMenuEntries[index];
	if (entry.type == BuildMenuEntryType::ENTRY_BUY_BEE) {
		BeeDemoNS::buy_bee_with_honey();
		return;
	}
	CreativeToolkit::set_selected_brush(entry.brush, B32_FALSE);
	CreativeToolkit::selectedRotation = ROTATION2_0;
}

void render_item_build_menu() {
	if (!itemBuildMenuVisible) {
		return;
	}

	BuildMenuLayout layout = build_menu_layout();
	Graphics::box(layout.x, layout.y, layout.width, layout.height, CreativeToolkit::borderSize, CreativeToolkit::borderColor, CreativeToolkit::fillColor);
	I32 hoveredIndex = build_menu_index_at(Win32::get_mouse());
	for (U32 i = 0; i < BUILD_MENU_ITEM_COUNT; i++) {
		const BuildMenuEntry& entry = buildMenuEntries[i];
		I32 col = I32(i % U32(layout.cols));
		I32 row = I32(i / U32(layout.cols));
		I32 cellX = layout.beginX + col * layout.cellSize;
		I32 cellY = layout.beginY + row * layout.cellSize;
		U32 availableCount = build_menu_affordable_count(entry);
		U32 badgeCount = build_menu_badge_count(entry);
		B32 affordable = availableCount > 0u ? B32_TRUE : B32_FALSE;
		RGBA8 cellColor = affordable ? RGBA8{ 100, 130, 170, 60 } : RGBA8{ 130, 80, 80, 90 };
		if (I32(i) == hoveredIndex) {
			cellColor = affordable ? RGBA8{ 160, 190, 220, 90 } : RGBA8{ 170, 100, 100, 110 };
		}
		fill_rect_blended(cellX + 1, cellY + 1, layout.cellSize - 2, layout.cellSize - 2, cellColor);
		Resources::Sprite* sprite = build_menu_sprite(entry);
		if (sprite) {
			SelectUI::draw_sprite_in_cell(*sprite, cellX + 4, cellY + 4, layout.cellSize - 8);
		}
		I32 badgeW = max(layout.cellSize / 3, 24);
		I32 badgeH = max(layout.cellSize / 4, 18);
		I32 badgeX = cellX + layout.cellSize - badgeW - 4;
		I32 badgeY = cellY + 4;
		fill_rect_blended(badgeX, badgeY, badgeW, badgeH, RGBA8{ 24, 24, 24, 180 });
		Graphics::display_num(badgeCount, badgeX + 4, badgeY + 1, 16);
		I32 hotkeyW = 18;
		I32 hotkeyH = 18;
		I32 hotkeyX = cellX + 4;
		I32 hotkeyY = cellY + layout.cellSize - hotkeyH - 4;
		fill_rect_blended(hotkeyX, hotkeyY, hotkeyW, hotkeyH, RGBA8{ 24, 24, 24, 180 });
		Graphics::display_num(build_menu_hotkey_number(i), hotkeyX + 1, hotkeyY + 1, 16);
		if (build_menu_selected(entry)) {
			Graphics::border(cellX, cellY, layout.cellSize, layout.cellSize, CreativeToolkit::selectBorderSize, CreativeToolkit::selectedColor);
		}
		else if (!affordable) {
			Graphics::border(cellX, cellY, layout.cellSize, layout.cellSize, 2, RGBA8{ 180, 90, 90, 255 });
		}
	}

	if (hoveredIndex >= 0 && U32(hoveredIndex) < BUILD_MENU_ITEM_COUNT) {
		const BuildMenuEntry& hoveredEntry = buildMenuEntries[hoveredIndex];
		BeeDemoNS::BuildCostDef costDef = build_menu_cost_def(hoveredEntry);
		if (costDef.numEntries > 0u) {
			BuildMenuLayout tipLayout = build_menu_layout();
			I32 col = hoveredIndex % tipLayout.cols;
			I32 row = hoveredIndex / tipLayout.cols;
			I32 cellX = tipLayout.beginX + col * tipLayout.cellSize;
			I32 cellY = tipLayout.beginY + row * tipLayout.cellSize;
			I32 iconScale = 2;
			I32 iconSize = 16 * iconScale;
			I32 numberSize = 32;
			I32 entryHeight = max(iconSize + 8, numberSize + 4);
			I32 tipPadding = 8;
			I32 maxEntryWidth = 0;
			for (U32 i = 0; i < costDef.numEntries; i++) {
				const BeeDemoNS::BuildCostEntry& costEntry = costDef.entries[i];
				I32 entryWidth = iconSize + 8 + decimal_digit_count(costEntry.count) * numberSize;
				maxEntryWidth = max(maxEntryWidth, entryWidth);
			}
			I32 tipW = tipPadding * 2 + max(maxEntryWidth, 64);
			I32 tipH = tipPadding * 2 + I32(costDef.numEntries) * entryHeight;
			I32 tipX = cellX + (tipLayout.cellSize - tipW) / 2;
			I32 tipY = cellY + tipLayout.cellSize + 8;
			if (tipY + tipH > Win32::framebufferHeight) {
				tipY = cellY - tipH - 8;
			}
			tipX = clamp(tipX, 0, max(Win32::framebufferWidth - tipW, 0));
			tipY = clamp(tipY, 0, max(Win32::framebufferHeight - tipH, 0));
			Graphics::box(tipX, tipY, tipW, tipH, 2, CreativeToolkit::borderColor, RGBA8{ 58, 76, 108, 235 });
			for (U32 i = 0; i < costDef.numEntries; i++) {
				const BeeDemoNS::BuildCostEntry& costEntry = costDef.entries[i];
				I32 rowY = tipY + tipPadding + I32(i) * entryHeight;
				I32 iconX = tipX + tipPadding;
				I32 iconY = rowY + (entryHeight - iconSize) / 2;
				I32 numberX = iconX + iconSize + 8;
				I32 numberY = rowY + (entryHeight - numberSize) / 2;
				Graphics::blit_sprite_cutout(*Inventory::itemSprite[costEntry.item], iconX, iconY, iconScale, 0);
				Graphics::display_num(costEntry.count, numberX, numberY, numberSize);
			}
		}
	}
}

B32 handle_item_build_menu_click(V2F32 mousePos) {
	if (!build_menu_contains(mousePos)) {
		return B32_FALSE;
	}
	I32 index = build_menu_index_at(mousePos);
	if (index >= 0) {
		select_build_menu_index(index);
	}
	hasLastDraggedTile = B32_FALSE;
	conveyorDragActive = B32_FALSE;
	return B32_TRUE;
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
	if (Factory::has_machine(tile) && !Factory::has_belt(tile)) {
		conveyorDragActive = B32_FALSE;
		conveyorDragHasIncoming = B32_FALSE;
		conveyorLastInputSide = DIRECTION2_INVALID;
		return;
	}
	if (!Factory::has_machine(tile)) {
		if (!BeeDemoNS::ensure_conveyor_tile(hoveredTile, CreativeToolkit::selectedBrushFreePlacement ? B32_FALSE : B32_TRUE)) {
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

	if (!BeeDemoNS::ensure_conveyor_tile(hoveredTile, CreativeToolkit::selectedBrushFreePlacement ? B32_FALSE : B32_TRUE)) {
		return;
	}
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
	Detail::apply_creative_brush_dispatch(brush, hoveredTile, CreativeToolkit::selectedRotation);
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
	BeeDemoNS::unqueue_tile_task(hoveredTile);
}

void update_drag_interactions() {
	V2F32 rawMouseDelta = Win32::get_raw_delta_mouse();
	B32 shiftHeld = Win32::keyboardState[Win32::KEY_SHIFT] ? B32_TRUE : B32_FALSE;
	B32 leftHeld = Win32::mouseButtonState[Win32::MOUSE_BUTTON_LEFT] ? B32_TRUE : B32_FALSE;
	B32 rightHeld = Win32::mouseButtonState[Win32::MOUSE_BUTTON_RIGHT] ? B32_TRUE : B32_FALSE;
	B32 middleHeld = Win32::mouseButtonState[Win32::MOUSE_BUTTON_MIDDLE] ? B32_TRUE : B32_FALSE;

	if (!leftHeld) {
		uiLeftCapture = B32_FALSE;
	}

	if (suppressRightDragUntilRelease) {
		if (!rightHeld) {
			suppressRightDragUntilRelease = B32_FALSE;
			hasLastDraggedTile = B32_FALSE;
			conveyorDragActive = B32_FALSE;
		}
		return;
	}

	if (((shiftHeld && leftHeld) || middleHeld) && !uiLeftCapture && !CreativeToolkit::tilesheetVisible && !SelectUI::open) {
		cameraDragActive = B32_TRUE;
		hasLastDraggedTile = B32_FALSE;
		conveyorDragActive = B32_FALSE;
		camera -= rawMouseDelta;
		clamp_camera();
		return;
	}

	cameraDragActive = B32_FALSE;
	if (shiftHeld || CreativeToolkit::tilesheetVisible || SelectUI::open || uiLeftCapture || Inventory::has_selected_item()) {
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


	U32 hotkeyIndex = 0;
	if (itemBuildMenuVisible && build_menu_key_to_index(key, &hotkeyIndex)) {
		select_build_menu_index(I32(hotkeyIndex));
		return;
	}

	if (key == Win32::KEY_I || key == Win32::KEY_B) {
		Factory::recipeMenuMachine.machine = nullptr; // special case; when opening the menu, close the recipe picker
		toggle_item_build_menu();
		if (itemBuildMenuVisible) {
			CreativeToolkit::close_ui();
			SelectUI::open = B32_FALSE;
		}

		hasLastDraggedTile = B32_FALSE;
		uiLeftCapture = B32_FALSE;
		conveyorDragActive = B32_FALSE;
		return;
	}

	if (key == Win32::KEY_R && Win32::keyboardState[Win32::KEY_CTRL]) {
		BeeDemoNS::init(hiveTile);
		center_camera_on_tile(hiveTile);
		reset_drag_state();
		CreativeToolkit::set_selected_brush(CreativeBrush::TASK_SELECT, B32_FALSE);
		Inventory::clear_selected_item();
		CreativeToolkit::selectedRotation = ROTATION2_0;
		CreativeToolkit::close_ui();
		SelectUI::open = B32_FALSE;
		close_item_build_menu();
		return;
	}

	if (key == Win32::KEY_BACKTICK) {
		close_item_build_menu();
		CreativeToolkit::toggle_ui();
		if (CreativeToolkit::tilesheetVisible) {
			SelectUI::open = B32_FALSE;
		}
		hasLastDraggedTile = B32_FALSE;
		uiLeftCapture = B32_FALSE;
		conveyorDragActive = B32_FALSE;
		return;
	}

	if (key == Win32::KEY_R && !Win32::keyboardState[Win32::KEY_CTRL]) {
		if (CreativeToolkit::brush_uses_rotation(CreativeToolkit::selectedBrush)) {
			CreativeToolkit::rotate_cw();
		}
		return;
	}

	if (key == Win32::KEY_Q) {
		if (CreativeToolkit::brush_uses_rotation(CreativeToolkit::selectedBrush)) {
			CreativeToolkit::rotate_ccw();
		}
		return;
	}

	if (key == Win32::KEY_ESC) {
		CreativeToolkit::close_ui();
		SelectUI::open = B32_FALSE;
		close_item_build_menu();
		uiLeftCapture = B32_FALSE;
		CreativeToolkit::set_selected_brush(CreativeBrush::TASK_SELECT, B32_FALSE);
		Inventory::clear_selected_item();
		CreativeToolkit::selectedRotation = ROTATION2_0;
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

	V2F32 mouse = Win32::get_mouse();

	if (button == Win32::MOUSE_BUTTON_RIGHT && state.state == Win32::BUTTON_STATE_DOWN) {
		if (itemBuildMenuVisible && !build_menu_contains(mouse)) {
			close_item_build_menu();
		}

		if (!CreativeToolkit::tilesheetVisible && !SelectUI::open && !Win32::keyboardState[Win32::KEY_SHIFT]) {
			V2U32 tile{};
			if (mouse_to_tile(&tile) && BeeDemoNS::queue_conveyor_pickup(tile, 1u)) {
				suppressRightDragUntilRelease = B32_TRUE;
				hasLastDraggedTile = B32_FALSE;
				conveyorDragActive = B32_FALSE;
				return;
			}
		}

		if (!CreativeToolkit::tilesheetVisible) {
			V2U32 tile{};
			if (mouse_to_tile(&tile)) {
				Factory::Machine* machine = Factory::get_machine_from_tile(V2U{ tile.x, tile.y });
				if (Factory::machine_supports_recipe_menu(machine)) {
					Factory::Machine* lastSelectedMachine = Factory::recipeMenuMachine.machine;
					close_item_build_menu();
					CreativeToolkit::close_ui();
					Inventory::clear_selected_item();
					if (lastSelectedMachine == machine) {
						return; // don't reopen if just closed
					}
					Factory::open_recipe_menu_for_machine(machine);
					suppressRightDragUntilRelease = B32_TRUE;
					uiLeftCapture = B32_TRUE;
					hasLastDraggedTile = B32_FALSE;
					conveyorDragActive = B32_FALSE;
					return;
				}
			}
		}
	}

	if (button == Win32::MOUSE_BUTTON_LEFT && state.state == Win32::BUTTON_STATE_DOWN) {
		uiLeftCapture = Inventory::click_callback(mouse);
		if (uiLeftCapture) {
			return;
		}
		if (itemBuildMenuVisible) {
			uiLeftCapture = handle_item_build_menu_click(mouse);
			if (uiLeftCapture) {
				return;
			}
		}
		if (CreativeToolkit::tilesheetVisible) {
			uiLeftCapture = CreativeToolkit::handle_ui_click(mouse);
			return;
		}
		if (SelectUI::open) {
			uiLeftCapture = SelectUI::click_callback(mouse);
			if (uiLeftCapture) {
				return;
			} else if (Factory::recipeMenuMachine.machine != nullptr) {
				// if left clicked outside of the recipe selection ui, close the menu
				close_item_build_menu();
			}
		}
		if (Inventory::has_selected_item()) {
			V2U32 tile{};
			if (mouse_to_tile(&tile) && BeeDemoNS::queue_inventory_delivery(tile, Inventory::selected_item(), 1u)) {
				if (!Win32::keyboardState[Win32::KEY_SHIFT]) {
					Inventory::clear_selected_item();
				}
			}
			uiLeftCapture = B32_TRUE;
			return;
		}
	}

	if ((button == Win32::MOUSE_BUTTON_LEFT || button == Win32::MOUSE_BUTTON_RIGHT) && state.state == Win32::BUTTON_STATE_UP) {
		hasLastDraggedTile = B32_FALSE;
		conveyorDragActive = B32_FALSE;
		if (button == Win32::MOUSE_BUTTON_LEFT) {
			cameraDragActive = B32_FALSE;
			uiLeftCapture = B32_FALSE;
		}
		else {
			suppressRightDragUntilRelease = B32_FALSE;
		}
	}
}

}
