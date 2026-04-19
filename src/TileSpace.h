#pragma once
#include "drillengine/DrillLib.h"

namespace TileSpace {

static constexpr F32 TILE_SIZE = 1.0F;
static constexpr F32 HALF_TILE_SIZE = 0.5F;

enum class NeighborDirection : U8 {
	NORTH = 0,
	EAST,
	SOUTH,
	WEST,
};


B32 same_tile(V2U32 a, V2U32 b) {
	return a.x == b.x && a.y == b.y;
}

V2F32 tile_to_world(V2U32 tile) {
	return V2F32{ F32(tile.x) * TILE_SIZE, F32(tile.y) * TILE_SIZE };
}

V2F32 tile_to_world_center(V2U32 tile) {
	return tile_to_world(tile) + V2F32{ HALF_TILE_SIZE, HALF_TILE_SIZE };
}

V2U32 world_to_tile(V2F32 world) {
	return V2U32{ U32(floorf32(world.x / TILE_SIZE)), U32(floorf32(world.y / TILE_SIZE)) };
}

V2U32 neighbor_tile(V2U32 tile, NeighborDirection direction) {
	switch (direction) {
	case NeighborDirection::NORTH: return V2U32{ tile.x, tile.y > 0 ? tile.y - 1 : 0 };
	case NeighborDirection::EAST:  return V2U32{ tile.x + 1, tile.y };
	case NeighborDirection::SOUTH: return V2U32{ tile.x, tile.y + 1 };
	case NeighborDirection::WEST:  return V2U32{ tile.x > 0 ? tile.x - 1 : 0, tile.y };
	}
	return tile;
}


V2F32 tile_local_to_world(V2U32 tile, V2F32 local01) {
	return tile_to_world(tile) + clamp(local01, V2F32{}, V2F32{ 1.0F, 1.0F });
}

}
