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

Xoshiro256 rng;
constexpr U32 MAX_JUNK_PER_BEACH_TILE = 5;
constexpr U32 MAX_JUNK_DELAY = 10;
constexpr U32 MIN_JUNK_DELAY = 1;

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
	void add_junk() {
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

void init(V2U extent) {
	size = extent;
	tiles = globalArena.alloc<TileType>(extent.x * extent.y);
	num_beach_tiles = extent.y;
	beach_tiles = globalArena.alloc<BeachTileInfo>(extent.y); // the entire left side of the map is a beach
	rng.seed_rand();
	for (U32 i = 0; i < extent.x * extent.y; i++) {
		tiles[i] = TILE_GRASS;
	}
	// overwrite all left tiles to be beach tiles; just temporary
	for (U32 i = 0; i < extent.y; i++) {
		tiles[i * extent.x] = TILE_BEACH;
		beach_tiles[i] = BeachTileInfo{ V2U {0,i}, 1.0, 0, {0} };
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
	I32 tileStartX = camStartX / (tileScale * 16);
	I32 tileStartY = camStartY / (tileScale * 16);
	I32 tileEndX = (camEndX + tileSize - 1) / tileSize;
	I32 tileEndY = (camEndY + tileSize - 1) / tileSize;
	for (I32 y = max(tileStartY, 0); y < min(tileEndY, I32(size.y)); y++) {
		for (I32 x = max(tileStartX, 0); x < min(tileEndX, I32(size.x)); x++) {
			Graphics::blit_sprite16x4(*tileSprite[tiles[y * size.x + x]], x * tileSize - camStartX, y * tileSize - camStartY, 0);
		}
	}
	render_beach(camera, tileScale);
}

// Call every frame to update the shore tiles
void beach_update(F32 dt) {
	for (U32 i = 0; i < num_beach_tiles; i++) {
		beach_tiles[i].delay -= dt;
		if (beach_tiles[i].delay <= 0.0) {
			beach_tiles[i].delay = F32((rng.next() % (MAX_JUNK_DELAY+MIN_JUNK_DELAY))-MIN_JUNK_DELAY);
			beach_tiles[i].add_junk();
		}
	}
}


}