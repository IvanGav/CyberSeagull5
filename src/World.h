#pragma once

#include "drillengine/DrillLib.h"
#include "Resources.h"
#include "Graphics.h"
#include "Win32.h"
#include "Inventory.h"
#include "TileSpace.h"

namespace World {

enum TileType : U8 {
	TILE_UNDEF,
	TILE_GRASS,
	TILE_GRASS_IRON,
	TILE_GRASS_COPPER,
	TILE_GRASS_FLOWERS,
	TILE_SAND,
	TILE_BEACH,
	TILE_WATER,
	TILE_Count
};

Resources::Sprite* tileSprite[TILE_Count];

V2U size;
TileType* tiles;
const U32 MACHINE_NULL_ID = 0;
U32* tileMachineIds;
enum MachineConnectFlags {
	MACHINE_INPUT_UP = 1 << 0,
	MACHINE_INPUT_DOWN = 1 << 1,
	MACHINE_INPUT_LEFT = 1 << 2,
	MACHINE_INPUT_RIGHT = 1 << 3,
	MACHINE_OUTPUT_UP = 1 << 4,
	MACHINE_OUTPUT_DOWN = 1 << 5,
	MACHINE_OUTPUT_LEFT = 1 << 6,
	MACHINE_OUTPUT_RIGHT = 1 << 7,
};
Flags8* canMachineConnect;

TileType get_tile(U32* machineId, I32 x, I32 y) {
	if (x < 0 || y < 0 || x >= size.x || y >= size.y) {
		return TILE_UNDEF;
	}
	if (machineId) {
		*machineId = tileMachineIds[y * size.x + x];
	}
	return tiles[y * size.x + x];
}

Xoshiro256 rng;
constexpr U32 MAX_JUNK_PER_BEACH_TILE = 5;
constexpr U32 MAX_JUNK_DELAY = 40;
constexpr U32 MIN_JUNK_DELAY = 8;

// list of floating items to render
struct JunkInfo {
	V2F coord;
	Inventory::Item item;
};

// list of beach tiles currently in the world + their delay before depositing a new junk
struct BeachTileInfo {
	V2U coord;
	F32 delay;
	U32 junkNum;
	JunkInfo junkList[MAX_JUNK_PER_BEACH_TILE];
	void empty() {
		add_junk();
		junkNum = 0;
	}
	void add_junk() {
		DEBUG_ASSERT(Inventory::inv.size != 0, "Inventory must be initialized before the world lol");
		delay = F32((rng.next() % (MAX_JUNK_DELAY + MIN_JUNK_DELAY)) - MIN_JUNK_DELAY);
		if (junkNum == MAX_JUNK_PER_BEACH_TILE) { return; }
		junkList[junkNum] = JunkInfo{ 
			TileSpace::tile_to_world(coord) + rand01v2f(rng) - V2F{0.5,0.5},
			(U32)rng.next() % (Inventory::inv.size)
		};

		junkNum++;
	}
};

U32 num_beach_tiles;
BeachTileInfo* beach_tiles; //JunkInfo* junk_list;

void set_machine(Rng2I32 range, U32 machineId) {
	range = range.intersected(Rng2I32{ 0, 0, I32(size.x - 1), I32(size.y - 1) });
	for (I32 y = range.minY; y <= range.maxY; y++) {
		for (I32 x = range.minX; x <= range.maxX; x++) {
			tileMachineIds[y * size.x + x] = machineId;
			canMachineConnect[y * size.x + x] = 0;
		}
	}
}

void set_connectivity(V2U pos, Flags8 machineConnectFlags) {
	if (pos.x < size.x && pos.y < size.y && tileMachineIds[pos.y * size.x + pos.x] != MACHINE_NULL_ID) {
		canMachineConnect[pos.y * size.x + pos.x] = machineConnectFlags;
	}
}

B32 can_connect_input(V2U pos, Direction2 fromDir) {
	if (pos.x >= size.x || pos.y >= size.y) {
		return B32_FALSE;
	}
	Flags8 connectFlags = canMachineConnect[pos.y * size.x + pos.x];
	switch (fromDir) {
	case DIRECTION2_LEFT: return connectFlags & MACHINE_INPUT_LEFT;
	case DIRECTION2_RIGHT: return connectFlags & MACHINE_INPUT_RIGHT;
	case DIRECTION2_FRONT: return connectFlags & MACHINE_INPUT_UP;
	case DIRECTION2_BACK: return connectFlags & MACHINE_INPUT_DOWN;
	}
	return B32_FALSE;
}

void init(V2U extent) {
	size = extent;
	tiles = globalArena.alloc<TileType>(extent.x * extent.y);
	num_beach_tiles = 0;
	beach_tiles = globalArena.alloc<BeachTileInfo>(extent.y); // the entire left side of the map is a beach
	rng.seed_rand();
	tileMachineIds = globalArena.alloc<U32>(extent.x * extent.y);
	canMachineConnect = globalArena.alloc<Flags8>(extent.x * extent.y);
	for (U32 i = 0; i < extent.x * extent.y; i++) {
		tiles[i] = TILE_GRASS;
		tileMachineIds[i] = MACHINE_NULL_ID;
		canMachineConnect[i] = 0;
	}

	tileSprite[TILE_UNDEF] = &Resources::tile.undef;
	tileSprite[TILE_GRASS] = &Resources::tile.grass;
	tileSprite[TILE_GRASS_IRON] = &Resources::tile.grassIron;
	tileSprite[TILE_GRASS_COPPER] = &Resources::tile.grassCopper;
	tileSprite[TILE_GRASS_FLOWERS] = &Resources::tile.grassFlowers;
	tileSprite[TILE_SAND] = &Resources::tile.sand;
	tileSprite[TILE_BEACH] = &Resources::tile.beach;
	tileSprite[TILE_WATER] = &Resources::tile.water;
}

void reset_runtime_state() {
	num_beach_tiles = 0;
	for (U32 i = 0; i < size.x * size.y; i++) {
		tileMachineIds[i] = MACHINE_NULL_ID;
	}
}

void render_beach(V2F camera, I32 tileScale) {
	I32 tileSize = (F32)tileScale * 16;
	I32 camStartX = I32(floorf32(camera.x));
	I32 camStartY = I32(floorf32(camera.y));
	for (U32 i = 0; i < num_beach_tiles; i++) {
		for (U32 j = 0; j < beach_tiles[i].junkNum; j++) {
			JunkInfo junk = beach_tiles[i].junkList[j];
			I32 screenX = I32(junk.coord.x * tileSize) - camStartX;
			I32 screenY = I32(junk.coord.y * tileSize) - camStartY;
			Graphics::blit_sprite_cutout(*Inventory::itemSprite[junk.item], screenX, screenY, tileScale, 0);
		}
	}
}

void render(V2F camera, I32 tileScale) {
	I32 tileSize = tileScale * 16;
	I32 camStartX = I32(floorf32(camera.x));
	I32 camStartY = I32(floorf32(camera.y));
	I32 camEndX = I32(ceilf32(camera.x + Win32::framebufferWidth));
	I32 camEndY = I32(ceilf32(camera.y + Win32::framebufferHeight));
	I32 tileStartX = camStartX / tileSize;
	I32 tileStartY = camStartY / tileSize;
	I32 tileEndX = (camEndX + tileSize - 1) / tileSize;
	I32 tileEndY = (camEndY + tileSize - 1) / tileSize;

	// Changed this to fix black like grid zooming glitch
	for (I32 y = max(tileStartY, 0); y < min(tileEndY, I32(size.y)); y++) {
		for (I32 x = max(tileStartX, 0); x < min(tileEndX, I32(size.x)); x++) {
			I32 drawX = x * tileSize - camStartX;
			I32 drawY = y * tileSize - camStartY;

		Graphics::blit_sprite(*tileSprite[tiles[y * size.x + x]], drawX, drawY, tileScale, 0);
		
		}
	}
	render_beach(camera, tileScale);
}

// Call every frame to update the shore tiles
void beach_update(F32 dt) {
	for (U32 i = 0; i < num_beach_tiles; i++) {
		beach_tiles[i].delay -= dt;
		if (beach_tiles[i].delay <= 0.0) {
			beach_tiles[i].add_junk();
		}
	}
}

void push_beach_tile(V2U pos) {
	beach_tiles[num_beach_tiles] = BeachTileInfo{ pos };
	beach_tiles[num_beach_tiles].empty();
	num_beach_tiles++;
}


}