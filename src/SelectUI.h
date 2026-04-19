#pragma once

#include "Win32.h"
#include "Graphics.h"
#include "Resources.h"

namespace SelectUI {
	// you can pick out of these sprites; the picked sprite will be returned as an index into the array
	ArenaArrayList<Resources::Sprite*> selections;
	void(*callback)(U32) = nullptr;
	I32 itemSize = 16; // assume all items are 16x16
	I32 scale = 4; // UI scale
	I32 selectedItem = -1; // -1 means no selection
	B32 open = B32_FALSE;

	I32 borderPadding = 50;
	I32 borderSize = 3;
	I32 selectBorderSize = 2;

	RGBA8 selectedColor = RGBA8{ 50, 250, 50, 255 };
	RGBA8 borderColor = RGBA8{ 0, 0, 0, 255 };
	RGBA8 fillColor = RGBA8{ 100, 160, 100, 255 };

	B32 popupMode = B32_FALSE;
	V2I popupAnchor{ 0, 0 };
	I32 popupGap = 10;

	struct Layout {
		I32 x = 0;
		I32 y = 0;
		I32 width = 0;
		I32 height = 0;
		I32 cols = 0;
		I32 rows = 0;
		I32 itemScreenSize = 0;
		I32 beginX = 0;
		I32 beginY = 0;
	};

	FINLINE I32 item_screen_size() {
		return max(itemSize * scale, 1);
	}

	FINLINE I32 items_per_row() {
		if (popupMode) {
			return clamp(I32(selections.size), 1, 4);
		}
		I32 usableWidth = Win32::framebufferWidth - borderPadding * 2 - borderSize * 2;
		return max(usableWidth / item_screen_size(), 1);
	}

	FINLINE I32 row_count() {
		I32 perRow = items_per_row();
		return I32(selections.size / U32(perRow)) + ((selections.size % U32(perRow)) > 0 ? 1 : 0);
	}

	FINLINE Layout compute_layout() {
		Layout layout{};
		layout.itemScreenSize = item_screen_size();
		layout.cols = items_per_row();
		layout.rows = max(row_count(), 1);
		layout.width = layout.cols * layout.itemScreenSize + borderSize * 2;
		layout.height = layout.rows * layout.itemScreenSize + borderSize * 2;
		if (popupMode) {
			layout.x = popupAnchor.x - layout.width / 2;
			layout.y = popupAnchor.y - layout.height - popupGap;
			layout.x = clamp(layout.x, 0, max(Win32::framebufferWidth - layout.width, 0));
			layout.y = clamp(layout.y, 0, max(Win32::framebufferHeight - layout.height, 0));
		}
		else {
			layout.x = borderPadding;
			layout.y = borderPadding;
		}
		layout.beginX = layout.x + borderSize;
		layout.beginY = layout.y + borderSize;
		return layout;
	}

	FINLINE I32 sprite_scale_to_fit_cell(Resources::Sprite& sprite, I32 cellSizePx) {
		I32 width = max(I32(sprite.width), 1);
		I32 height = max(I32(sprite.height), 1);
		I32 scaleX = max(cellSizePx / width, 1);
		I32 scaleY = max(cellSizePx / height, 1);
		return max(min(scaleX, scaleY), 1);
	}

	FINLINE void draw_sprite_in_cell(Resources::Sprite& sprite, I32 cellX, I32 cellY, I32 cellSizePx) {
		I32 drawScale = sprite_scale_to_fit_cell(sprite, cellSizePx);
		I32 drawWidth = I32(sprite.width) * drawScale;
		I32 drawHeight = I32(sprite.height) * drawScale;
		I32 drawX = cellX + (cellSizePx - drawWidth) / 2;
		I32 drawY = cellY + (cellSizePx - drawHeight) / 2;
		Graphics::blit_sprite_cutout(sprite, drawX, drawY, drawScale, 0);
	}

	void set_popup_anchor(V2I anchor) {
		popupMode = B32_TRUE;
		popupAnchor = anchor;
	}

	void clear_popup_anchor() {
		popupMode = B32_FALSE;
	}

	void set_selected_index(I32 index) {
		selectedItem = index >= 0 && U32(index) < selections.size ? index : -1;
	}

	void change_select_options(ArenaArrayList<Resources::Sprite*> options, void(*set_callback)(U32) = nullptr) {
		selections = options;
		open = B32_FALSE;
		selectedItem = -1;
		callback = set_callback;
	}

	void debug_selections() {
		selections.clear();
		selections.push_back(&Resources::tile.icon.belt);
		selections.push_back(&Resources::tile.icon.assembler);
		selections.push_back(&Resources::tile.icon.hive);

		selections.push_back(&Resources::tile.item.honey);
		selections.push_back(&Resources::tile.item.kittyCat);
		selections.push_back(&Resources::tile.item.camera);
		selections.push_back(&Resources::tile.item.ironOre);

		selections.push_back(&Resources::tile.rock.topLeft);
		selections.push_back(&Resources::tile.rock.top);
		selections.push_back(&Resources::tile.rock.topRight);
		selections.push_back(&Resources::tile.rock.left);
		selections.push_back(&Resources::tile.rock.full);
		selections.push_back(&Resources::tile.rock.right);

		selectedItem = -1;
		open = B32_FALSE;
		popupMode = B32_FALSE;
	}

	void draw() {
		if (!open) return;
		Layout layout = compute_layout();

		Graphics::box(
			layout.x,
			layout.y,
			layout.width,
			layout.height,
			borderSize,
			borderColor,
			fillColor
		);

		for (U32 i = 0; i < selections.size; i++) {
			Resources::Sprite* sprite = selections[i];
			if (!sprite) continue;
			draw_sprite_in_cell(*sprite,
				layout.beginX + I32(i % U32(layout.cols)) * layout.itemScreenSize,
				layout.beginY + I32(i / U32(layout.cols)) * layout.itemScreenSize,
				layout.itemScreenSize);
		}

		if (selectedItem == -1) return;
		Graphics::border(
			layout.beginX + (selectedItem % layout.cols) * layout.itemScreenSize,
			layout.beginY + (selectedItem / layout.cols) * layout.itemScreenSize,
			layout.itemScreenSize, layout.itemScreenSize, selectBorderSize, selectedColor
		);
	}

	// return true iff clicking on the SelectUI
	B32 click_callback(V2F32 mousePos) {
		if (!open) return B32_FALSE;
		Layout layout = compute_layout();
		V2I32 mouse = { (I32)mousePos.x - layout.beginX, (I32)mousePos.y - layout.beginY };

		I32 interactableBoxW = layout.cols * layout.itemScreenSize;
		I32 interactableBoxH = layout.rows * layout.itemScreenSize;
		if (!(mouse.x >= 0 && mouse.x < interactableBoxW && mouse.y >= 0 && mouse.y < interactableBoxH)) {
			return B32_FALSE;
		}

		mouse.x /= layout.itemScreenSize;
		mouse.y /= layout.itemScreenSize;
		I32 index = mouse.x + mouse.y * layout.cols;
		if (index < 0 || U32(index) >= selections.size) {
			return B32_TRUE;
		}

		selectedItem = index;
		if (callback != nullptr) {
			callback(U32(index));
		}
		open = B32_FALSE;
		popupMode = B32_FALSE;
		return B32_TRUE;
	}

};
