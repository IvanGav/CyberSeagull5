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
	TILE_MOUNTAIN,
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
constexpr U32 MAX_JUNK_DELAY = 250;
constexpr U32 MIN_JUNK_DELAY = 30;

// list of floating items to render
struct JunkInfo {
	V2F coord;
	Inventory::ItemType item;
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
		delay = F32((rng.next() % (MAX_JUNK_DELAY-MIN_JUNK_DELAY)) + MIN_JUNK_DELAY);
		if (junkNum == MAX_JUNK_PER_BEACH_TILE) { return; }

		static const Inventory::ItemType allowedShoreItems[] = {
			Inventory::ITEM_IRON_ORE,
			Inventory::ITEM_COPPER_ORE,
			Inventory::ITEM_GULL,
			Inventory::ITEM_FEATHER,
			Inventory::ITEM_GEAR,
			Inventory::ITEM_URANIUM,
		};

		Inventory::ItemType shoreItem = allowedShoreItems[rng.next() % ARRAY_COUNT(allowedShoreItems)];
		junkList[junkNum] = JunkInfo{ 
			TileSpace::tile_to_world(coord) + rand01v2f(rng) - V2F{0.5,0.5},
			shoreItem
		};

		junkNum++;
	}
	Inventory::ItemType pop_junk() {
		DEBUG_ASSERT(junkNum > 0);
		junkNum--;
		return junkList[junkNum].item;
	}
};

U32 num_beach_tiles;
BeachTileInfo* beach_tiles;

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

Flags8 get_connectivity_flags(V2U pos) {
	if (pos.x >= size.x || pos.y >= size.y) {
		return 0;
	}
	Flags8 connectFlags = canMachineConnect[pos.y * size.x + pos.x];
	return connectFlags;
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
	beach_tiles = globalArena.alloc<BeachTileInfo>(extent.x * extent.y);
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
	tileSprite[TILE_MOUNTAIN] = &Resources::tile.mountain;
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

FINLINE B32 tile_is_mountain(I32 x, I32 y) {
	if (x < 0 || y < 0 || x >= I32(size.x) || y >= I32(size.y)) {
		return B32_FALSE;
	}
	return tiles[y * size.x + x] == TILE_MOUNTAIN ? B32_TRUE : B32_FALSE;
}

Resources::Sprite* mountain_sprite_for_tile(I32 x, I32 y) {
	B32 mountainAbove = tile_is_mountain(x, y - 1);
	B32 mountainLeft = tile_is_mountain(x - 1, y);
	B32 mountainRight = tile_is_mountain(x + 1, y);

	if (!mountainAbove) {
		if (!mountainLeft && !mountainRight) {
			return &Resources::tile.rock.top;
		}
		if (!mountainLeft) {
			return &Resources::tile.rock.topLeft;
		}
		if (!mountainRight) {
			return &Resources::tile.rock.topRight;
		}
		return &Resources::tile.rock.top;
	}

	if (!mountainLeft && !mountainRight) {
		return &Resources::tile.rock.full;
	}
	if (!mountainLeft) {
		return &Resources::tile.rock.left;
	}
	if (!mountainRight) {
		return &Resources::tile.rock.right;
	}
	return &Resources::tile.rock.full;
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
			TileType tile = tiles[y * size.x + x];
			Resources::Sprite* sprite = tile == TILE_MOUNTAIN ? mountain_sprite_for_tile(x, y) : tileSprite[tile];
			Graphics::blit_sprite(*sprite, drawX, drawY, tileScale, 0);
		}
	}
	render_beach(camera, tileScale);
}

// Call every frame to update the shore tiles
void beach_update(F32 dt) {
	for (U32 i = 0; i < num_beach_tiles; i++) {
		if (beach_tiles[i].delay > 0.0) {
			beach_tiles[i].delay -= dt;
		} else {
			beach_tiles[i].add_junk();
		}
	}
}

I32 find_beach_tile_index(V2U tile) {
	for (U32 i = 0; i < num_beach_tiles; i++) {
		if (beach_tiles[i].coord.x == tile.x && beach_tiles[i].coord.y == tile.y) {
			return I32(i);
		}
	}
	return -1;
}

void push_beach_tile(V2U pos) {
	if (find_beach_tile_index(pos) >= 0) {
		return;
	}
	beach_tiles[num_beach_tiles] = BeachTileInfo{ pos };
	beach_tiles[num_beach_tiles].empty();
	num_beach_tiles++;
}

void remove_beach_tile(V2U pos) {
	I32 beachIndex = find_beach_tile_index(pos);
	if (beachIndex < 0) {
		return;
	}
	beach_tiles[U32(beachIndex)] = beach_tiles[num_beach_tiles - 1];
	num_beach_tiles--;
}

void sync_beach_tile_with_world(V2U pos) {
	if (pos.x >= size.x || pos.y >= size.y) {
		remove_beach_tile(pos);
		return;
	}
	if (tiles[pos.y * size.x + pos.x] == TILE_BEACH) {
		push_beach_tile(pos);
	}
	else {
		remove_beach_tile(pos);
	}
}

BeachTileInfo* find_beach_tile(V2U tile) {
	I32 beachIndex = find_beach_tile_index(tile);
	return beachIndex >= 0 ? &beach_tiles[U32(beachIndex)] : nullptr;
}

B32 beach_has_junk(V2U tile) {
	BeachTileInfo* beach = find_beach_tile(tile);
	return (beach && beach->junkNum > 0) ? B32_TRUE : B32_FALSE;
}

B32 pop_beach_junk(V2U tile, Inventory::ItemType* outItem) {
	BeachTileInfo* beach = find_beach_tile(tile);
	if (!beach || beach->junkNum == 0) {
		return B32_FALSE;
	}
	*outItem = beach->pop_junk();
	return B32_TRUE;
}


}