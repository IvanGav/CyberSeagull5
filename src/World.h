#pragma once

#include "drillengine/DrillLib.h"
#include "Resources.h"
#include "Graphics.h"
#include "Win32.h"

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

void init(V2U extent) {
	size = extent;
	tiles = globalArena.alloc<TileType>(extent.x * extent.y);
	for (U32 i = 0; i < extent.x * extent.y; i++) {
		tiles[i] = TILE_GRASS;
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
}


}