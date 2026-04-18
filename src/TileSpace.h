#pragma once

#include "drillengine/DrillLib.h"


namespace World {

struct TileSpace {
	V2U32 worldOrigin{};
	V2U32 tileSize{ 1, 1 };

    static V2F32 to_v2f(V2U32 v) {
		return V2F32{ F32(v.x), F32(v.y) };
	}

	V2F32 tile_to_world_origin(V2U32 tile) const {
		V2F32 sizeAsFloat = to_v2f(tileSize);
		return to_v2f(worldOrigin) + to_v2f(tile) * sizeAsFloat;
	}

	V2F32 tile_to_world_center(V2U32 tile) const {
		V2F32 sizeAsFloat = to_v2f(tileSize);
		return tile_to_world_origin(tile) + sizeAsFloat * 0.5F;
	}
};

TileSpace& tile_space() {
	static TileSpace result{ {}, { 1, 1 } };
	return result;
}

void set_tile_space(V2U32 worldOrigin, V2U32 tileSize) {
	tile_space().worldOrigin = worldOrigin;
	tile_space().tileSize = tileSize;
}

}
