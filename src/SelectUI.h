#pragma once

#include "Win32.h"
#include "Graphics.h"
#include "Resources.h"

namespace SelectUI {
	// you can pick out of these sprites; the picked sprite will be returned as an index into the array
	// all textures have to be 16x16
	ArenaArrayList<Resources::Sprite*> selections;
	I32 itemSize = 16; // assume all items are 16x16
	I32 scale = 4; // UI scale
	I32 selectedItem = -1; // -1 means no selection
	B32 open = B32_FALSE;

	I32 borderPadding = 50; // ideal padding on all sides of the screen from the UI
	I32 borderSize = 3; // size of the UI's border
	I32 selectBorderSize = 2;

	RGBA8 selectedColor = RGBA8{ 50, 250, 50, 255 };

	void change_select_options(ArenaArrayList<Resources::Sprite*> options) {
		selections = options;
		open = B32_FALSE;
		selectedItem = -1;
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
	}

	void draw() {
		if (!open) return;
		I32 screenH = Win32::framebufferHeight;
		I32 screenW = Win32::framebufferWidth;
		// determine the UI width and height
		I32 itemScreenSize = itemSize * scale;
		I32 itemsPerRow = (screenW - borderPadding * 2 - borderSize * 2) / itemScreenSize; // items that can fit per row, on the screen
		I32 beginX = borderPadding + borderSize;
		I32 beginY = borderPadding + borderSize;
		Graphics::box(
			borderPadding, borderPadding, 
			itemsPerRow * itemScreenSize + borderSize * 2, 
			// it's horrible so: number of rows + if we have an incomplete row, +1; that's all multiplied by item size and we add a top offset
			((selections.size / itemsPerRow) + ((selections.size % itemsPerRow) > 0)) * itemScreenSize + borderSize * 2,
			borderSize,
			RGBA8 { 0,0,0,255 }, RGBA8 { 100, 160, 100, 255 }
		);
		for (U32 i = 0; i < selections.size; i++) {
			Graphics::blit_sprite_cutout(*selections[i], beginX + (i % itemsPerRow) * itemScreenSize, beginY + (i / itemsPerRow) * itemScreenSize, scale, 0);
		}
		// highlight the selected item
		if (selectedItem == -1) return;
		Graphics::border(
			borderPadding + borderSize + (selectedItem % itemsPerRow) * itemScreenSize,
			borderPadding + borderSize + (selectedItem / itemsPerRow) * itemScreenSize,
			itemScreenSize, itemScreenSize, selectBorderSize, selectedColor
		);
	}

	// return true iff clicking on the SelectUI
	B32 click_callback(V2F32 mousePos) {
		if (!open) return false;
		V2I32 mouse = { (I32) mousePos.x, (I32) mousePos.y };
		I32 screenH = Win32::framebufferHeight;
		I32 screenW = Win32::framebufferWidth;
		// determine the UI width and height
		I32 itemScreenSize = itemSize * scale;
		I32 itemsPerRow = (screenW - borderPadding * 2 - borderSize * 2) / itemScreenSize; // items per row
		I32 beginX = borderPadding + borderSize;
		I32 beginY = borderPadding + borderSize;
		
		// they now start from the top left corner of the first item
		mouse.x -= beginX;
		mouse.y -= beginY;

		I32 interactableBoxW = itemsPerRow * itemScreenSize;
		// it's horrible so: number of rows + if we have an incomplete row, +1; that's all multiplied by item size and we add a top offset
		I32 interactableBoxH = ((selections.size / itemsPerRow) + ((selections.size % itemsPerRow) > 0)) * itemScreenSize;

		if (!(mouse.x > 0 && mouse.x < interactableBoxW && mouse.y > 0 && mouse.y < interactableBoxH)) {
			return false; // clicked outside of the ui
		}

		mouse.x /= itemScreenSize;
		mouse.y /= itemScreenSize;

		I32 index = mouse.x + mouse.y * itemsPerRow;

		if (selectedItem == index) {
			selectedItem = -1; // unselect
		} else if(index < selections.size) {
			selectedItem = index; // select
		}
		return true;
	}

};