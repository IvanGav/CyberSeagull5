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

	FINLINE I32 item_screen_size() {
		return max(itemSize * scale, 1);
	}

	FINLINE I32 items_per_row() {
		I32 usableWidth = Win32::framebufferWidth - borderPadding * 2 - borderSize * 2;
		return max(usableWidth / item_screen_size(), 1);
	}

	FINLINE I32 row_count() {
		I32 perRow = items_per_row();
		return I32(selections.size / U32(perRow)) + ((selections.size % U32(perRow)) > 0 ? 1 : 0);
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
	}

	void draw() {
		if (!open) return;
		I32 itemScreenSize = item_screen_size();
		I32 perRow = items_per_row();
		I32 rows = max(row_count(), 1);
		I32 beginX = borderPadding + borderSize;
		I32 beginY = borderPadding + borderSize;

		Graphics::box(
			borderPadding,
			borderPadding,
			perRow * itemScreenSize + borderSize * 2,
			rows * itemScreenSize + borderSize * 2,
			borderSize,
			borderColor,
			fillColor
		);

		for (U32 i = 0; i < selections.size; i++) {
			Resources::Sprite* sprite = selections[i];
			if (!sprite) continue;
			draw_sprite_in_cell(*sprite,
				beginX + I32(i % U32(perRow)) * itemScreenSize,
				beginY + I32(i / U32(perRow)) * itemScreenSize,
				itemScreenSize);
		}

		if (selectedItem == -1) return;
		Graphics::border(
			borderPadding + borderSize + (selectedItem % perRow) * itemScreenSize,
			borderPadding + borderSize + (selectedItem / perRow) * itemScreenSize,
			itemScreenSize, itemScreenSize, selectBorderSize, selectedColor
		);
	}

	// return true iff clicking on the SelectUI
	B32 click_callback(V2F32 mousePos) {
		if (!open) return B32_FALSE;
		V2I32 mouse = { (I32)mousePos.x, (I32)mousePos.y };
		I32 itemScreenSize = item_screen_size();
		I32 perRow = items_per_row();
		I32 beginX = borderPadding + borderSize;
		I32 beginY = borderPadding + borderSize;

		mouse.x -= beginX;
		mouse.y -= beginY;

		I32 interactableBoxW = perRow * itemScreenSize;
		I32 interactableBoxH = max(row_count(), 1) * itemScreenSize;
		if (!(mouse.x >= 0 && mouse.x < interactableBoxW && mouse.y >= 0 && mouse.y < interactableBoxH)) {
			return B32_FALSE;
		}

		mouse.x /= itemScreenSize;
		mouse.y /= itemScreenSize;
		I32 index = mouse.x + mouse.y * perRow;

		if (selectedItem == index) {
			selectedItem = -1; // unselect
		} else if(index < selections.size) {
			selectedItem = index; // select
			if(callback != nullptr) callback(index);
		}
		open = false;
		return true;
	}

};
