#pragma once

#include "drillengine/DrillLib.h";
#include "Graphics.h"
#include "Win32.h"

namespace Inventory {
	typedef U32 Res;
	U32 res_font_size = 16;
	U32 x_off = 10;
	U32 y_off = 10;

	ArenaArrayList<Res> inv; // inv[Res] = how many of that resource is in the invenotry

	// Call to create space for `item_count` resources in the inventory (0..inv_count)
	void init_inv(U32 item_count) {
		inv.clear();
		inv.resize(item_count);
	}

	void draw_inv() {
		for (U32 i = 0; i < inv.size; i++) {
			Graphics::display_text(0, (res_font_size + 2) * i, res_font_size, "%"a, Inventory::inv[i]);
		}
	}
};


//namespace TileSpace {
//	V2F32 tile_to_world_origin(V2U32 tile, V2U32 scale) const {
//		V2F32 sizeAsFloat = to_v2f(tileSize);
//		return to_v2f(worldOrigin) + to_v2f(tile) * sizeAsFloat;
//	}
//
//	V2F32 tile_to_world_center(V2U32 tile) const {
//		V2F32 sizeAsFloat = to_v2f(tileSize);
//		return tile_to_world_origin(tile) + sizeAsFloat * 0.5F;
//	}
//};