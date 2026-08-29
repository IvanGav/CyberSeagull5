#pragma once

#include "drillengine/DrillLib.h"
#include "Resources.h"
#include "Graphics.h"
#include "Win32.h"
#include "Inventory.h"
#include "TileSpace.h"

namespace Cyber5eagull::BeeDemo { U32 get_richness_animation_frame(U32 x, U32 y); };

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
U32 maxDepth;
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

TileType get_tile(U32* machineId, I32 x, I32 y, I32 depth) {
	if (x < 0 || y < 0 || depth < 0 || x >= size.x || y >= size.y || depth >= maxDepth) {
		return TILE_UNDEF;
	}
	if (machineId) {
		*machineId = tileMachineIds[(depth * size.y + y) * size.x + x];
	}
	return tiles[y * size.x + x];
}

Xoshiro256 rng;
constexpr U32 MAX_JUNK_PER_BEACH_TILE = 3;
constexpr F32 MAX_JUNK_DELAY = 250.0f;
constexpr F32 MIN_JUNK_DELAY = 30.0f;
constexpr F32 JUNK_SHORE_LIFETIME = 100.0f;

struct JunkInfo {
	V2F coord;
	Inventory::ItemType item;
	F32 timeOnShore;
};

struct BeachTileInfo {
	V2U coord;
	F32 delay;
	U32 junkNum;
	JunkInfo junkList[MAX_JUNK_PER_BEACH_TILE];

	void empty() {
		junkNum = 0;
		U32 minDelay = U32(MIN_JUNK_DELAY);
		U32 maxDelay = U32(MAX_JUNK_DELAY);
		U64 delayRange = U64(maxDelay - minDelay + 1);
		delay = F32((rng.next() % delayRange) + minDelay);
	}

	void add_junk() {
		DEBUG_ASSERT(Inventory::inv.size != 0, "Inventory must be initialized before the world lol");

		U32 minDelay = U32(MIN_JUNK_DELAY);
		U32 maxDelay = U32(MAX_JUNK_DELAY);
		U64 delayRange = U64(maxDelay - minDelay + 1);

		delay = F32((rng.next() % delayRange) + minDelay);
		if (junkNum == MAX_JUNK_PER_BEACH_TILE) {
			return;
		}

		static const Inventory::ItemType allowedShoreItems[] = {
			Inventory::ITEM_FEATHER,
			Inventory::ITEM_FEATHER,
			Inventory::ITEM_FEATHER,
			Inventory::ITEM_IRON_ORE,
			Inventory::ITEM_IRON_ORE,
			Inventory::ITEM_COPPER_ORE,
			Inventory::ITEM_COPPER_ORE,
			Inventory::ITEM_GULL,
			Inventory::ITEM_GULL,
			Inventory::ITEM_GEAR,
			Inventory::ITEM_URANIUM,
		};

		Inventory::ItemType shoreItem =
			allowedShoreItems[rng.next() % ARRAY_COUNT(allowedShoreItems)];

		junkList[junkNum] = JunkInfo{
			TileSpace::tile_to_world(coord) + rand01v2f(rng) - V2F{0.5, 0.5},
			shoreItem,
			JUNK_SHORE_LIFETIME
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

void set_machine(Rng2I32 range, I32 depth, U32 machineId) {
	if (depth < 0 || depth >= maxDepth) {
		return;
	}
	U32* machineIdLayer = tileMachineIds + depth * size.x * size.y;
	Flags8* connectLayer = canMachineConnect + depth * size.x * size.y;
	range = range.intersected(Rng2I32{ 0, 0, I32(size.x - 1), I32(size.y - 1) });
	for (I32 y = range.minY; y <= range.maxY; y++) {
		for (I32 x = range.minX; x <= range.maxX; x++) {
			machineIdLayer[y * size.x + x] = machineId;
			connectLayer[y * size.x + x] = 0;
		}
	}
}

void set_connectivity(V2U pos, I32 depth, Flags8 machineConnectFlags) {
	Flags8* connectLayer = canMachineConnect + depth * size.x * size.y;
	if (pos.x < size.x && pos.y < size.y && tileMachineIds[(depth * size.y + pos.y) * size.x + pos.x] != MACHINE_NULL_ID) {
		connectLayer[pos.y * size.x + pos.x] = machineConnectFlags;
	}
}

Flags8 get_connectivity_flags(V2U pos, U32 depth) {
	if (pos.x >= size.x || pos.y >= size.y || depth >= maxDepth) {
		return 0;
	}
	Flags8* connectLayer = canMachineConnect + depth * size.x * size.y;
	Flags8 connectFlags = connectLayer[pos.y * size.x + pos.x];
	return connectFlags;
}

B32 can_connect_input(V2U pos, U32 depth, Direction2 fromDir) {
	if (pos.x >= size.x || pos.y >= size.y || depth >= maxDepth) {
		return B32_FALSE;
	}
	Flags8* connectLayer = canMachineConnect + depth * size.x * size.y;
	Flags8 connectFlags = connectLayer[pos.y * size.x + pos.x];
	switch (fromDir) {
	case DIRECTION2_LEFT: return connectFlags & MACHINE_INPUT_LEFT;
	case DIRECTION2_RIGHT: return connectFlags & MACHINE_INPUT_RIGHT;
	case DIRECTION2_FRONT: return connectFlags & MACHINE_INPUT_UP;
	case DIRECTION2_BACK: return connectFlags & MACHINE_INPUT_DOWN;
	}
	return B32_FALSE;
}

void reset_runtime_state() {
	num_beach_tiles = 0;
	for (U32 i = 0; i < size.x * size.y * maxDepth; i++) {
		tileMachineIds[i] = MACHINE_NULL_ID;
		canMachineConnect[i] = 0;
	}
}

void init(V2U extent) {
	size = extent;
	maxDepth = 2;
	tiles = globalArena.alloc<TileType>(extent.x * extent.y);
	num_beach_tiles = 0;
	beach_tiles = globalArena.alloc<BeachTileInfo>(extent.x * extent.y);
	rng.seed_rand();
	tileMachineIds = globalArena.alloc<U32>(extent.x * extent.y * maxDepth);
	canMachineConnect = globalArena.alloc<Flags8>(extent.x * extent.y * maxDepth);
	for (U32 i = 0; i < extent.x * extent.y; i++) {
		tiles[i] = TILE_GRASS;
	}
	reset_runtime_state();

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

FINLINE B32 tile_is_beach(I32 x, I32 y) {
	if (x < 0 || y < 0 || x >= I32(size.x) || y >= I32(size.y)) {
		return B32_FALSE;
	}
	return (tiles[y * size.x + x] == TILE_BEACH) || (tiles[y * size.x + x] == TILE_SAND) ? B32_TRUE : B32_FALSE;
}

FINLINE B32 tile_is_water(I32 x, I32 y) {
	if (x < 0 || y < 0 || x >= I32(size.x) || y >= I32(size.y)) {
		return B32_TRUE;
	}
	// check if tile is water or out of bounds
	return (tiles[y * size.x + x] == TILE_WATER) ? B32_TRUE : B32_FALSE;
}
FINLINE B32 tile_is_grass(I32 x, I32 y) {
	if (x < 0 || y < 0 || x >= I32(size.x) || y >= I32(size.y)) {
		return B32_FALSE;
	}

	TileType tile = tiles[y * size.x + x];
	return tile == TILE_GRASS ||
		tile == TILE_GRASS_IRON ||
		tile == TILE_GRASS_COPPER ||
		tile == TILE_GRASS_FLOWERS
		? B32_TRUE
		: B32_FALSE;
}

U32 sand_render_frame(I32 x, I32 y) {
	if (!tile_is_grass(x + 1, y)) {
		return 0u;
	}

	B32 grassAbove = tile_is_grass(x, y - 1);
	B32 grassBelow = tile_is_grass(x, y + 1);

	if (grassAbove && grassBelow) {
		return 3u;
	}

	if (grassAbove) {
		return 2u;
	}

	return 1u;
}
struct BeachRenderInfo {
	U32 frame;
	U32 rotation;
	B32 flipX;
};

BeachRenderInfo beach_render_info_for_tile(I32 x, I32 y) {
	if (!tile_is_beach(x, y)) {
		return BeachRenderInfo{ 0u, 0u, 0u };
	}

	B32 beachAbove = tile_is_beach(x, y - 1);
	B32 beachBelow = tile_is_beach(x, y + 1);

	if (beachAbove == beachBelow) {
		return BeachRenderInfo{ 0u, 0u, 0u };
	}

	// The water side determines which handed L-corner is needed.
	B32 waterOnMissingSide = beachAbove
		? tile_is_water(x, y + 1)
		: tile_is_water(x, y - 1);

	if (beachAbove) {
		// Shore continues above and steps below.
		return BeachRenderInfo{
			1u,
			waterOnMissingSide ? 0u : 3u,
			0u
		};
	}

	// Shore continues below and steps above.
	return BeachRenderInfo{
		1u,
		waterOnMissingSide ? 2u : 0u,
	    waterOnMissingSide ? 1u : 0u
	};
}

struct MountainRenderInfo {
	Resources::Sprite* sprite;
	U32 rotation;
};

MountainRenderInfo mountain_sprite_for_tile(I32 x, I32 y) {
	B32 above = tile_is_mountain(x, y - 1);
	B32 right = tile_is_mountain(x + 1, y);
	B32 below = tile_is_mountain(x, y + 1);
	B32 left = tile_is_mountain(x - 1, y);

	U32 mountainMask =
		(above ? 1u : 0u) |
		(right ? 2u : 0u) |
		(below ? 4u : 0u) |
		(left ? 8u : 0u);

	switch (mountainMask) {
	case 0u:
		return { &Resources::tile.rock.full, 0u };

		// Lone neighboring mountain pieces use the top sprite.
	case 1u: // Top
		return { &Resources::tile.rock.top, 0u };

	case 2u: // Right
		return { &Resources::tile.rock.top, 1u };

	case 4u: // Bottom
		return { &Resources::tile.rock.top, 2u };

	case 8u: // Left
		return { &Resources::tile.rock.top, 3u };

		// Straight horizontal connection
	case 10u: // Left + right
		return { &Resources::tile.rock.top, 0u };

		// Straight vertical connection
	case 5u: // Top + bottom
		return { &Resources::tile.rock.right, 0u };

		// Corners.
	case 3u: // Top + right
		return { &Resources::tile.rock.topRight, 0u };

	case 6u: // Right + bottom
		return { &Resources::tile.rock.topRight, 1u };

	case 12u: // Bottom + left
		return { &Resources::tile.rock.topRight, 2u };

	case 9u: // Left + top
		return { &Resources::tile.rock.topRight, 3u };

		// Three neighbor side use the right sprite : )
	case 7u: // Top + right + bottom
		return { &Resources::tile.rock.right, 0u };

	case 14u: // Right + bottom + left
		return { &Resources::tile.rock.right, 1u };

	case 13u: // Bottom + left + top
		return { &Resources::tile.rock.right, 2u };

	case 11u: // Left + top + right
		return { &Resources::tile.rock.right, 3u };

	case 15u:
		return { &Resources::tile.rock.full, 0u };
	}

	return { &Resources::tile.rock.full, 0u };
}

U32 grass_decoration_frame(I32 x, I32 y) {
	if (tiles[y * size.x + x] != TILE_GRASS) {
		return 0;
	}

	// Spread the three base grass textures evenly across the world
	U32 selectedFrame = hash32(
		0x6A09E667u ^
		(U32(x) * 0x9E3779B9u) ^
		(U32(y) * 0x85EBCA6Bu)
	) % 3u;

	// Check nearby tiles for deterministic clump anchors
	for (I32 anchorY = max(y - 2, 0); anchorY <= y; anchorY++) {
		for (I32 anchorX = max(x - 2, 0); anchorX <= x; anchorX++) {
			if (tiles[anchorY * size.x + anchorX] != TILE_GRASS) {
				continue;
			}

			U32 hash = hash32(
				0x6A09E667u ^
				(U32(anchorX) * 0x9E3779B9u) ^
				(U32(anchorY) * 0x85EBCA6Bu)
			);

			U32 rarity = hash & 255u;
			U32 frame = 0;

			if (rarity < 10u) {
				frame = 3;
			}
			else if (rarity < 25u) {
				frame = 4;
			}
			else if (rarity < 40u) {
				frame = 5;
			}
			else {
				continue;
			}

			U32 clumpSize = 1u + ((hash >> 8) % 2u); // 1 or 2
			B32 horizontal = ((hash >> 10) & 1u) ? B32_TRUE : B32_FALSE;

			for (U32 clumpIndex = 0; clumpIndex < clumpSize; clumpIndex++) {
				I32 clumpX = anchorX + (horizontal ? I32(clumpIndex) : 0);
				I32 clumpY = anchorY + (horizontal ? 0 : I32(clumpIndex));

				if (clumpX == x && clumpY == y) {
					selectedFrame = max(selectedFrame, frame);
				}
			}
		}
	}

	return selectedFrame;
}

U32 grass_misc_decoration_frame(I32 x, I32 y) {
	if (tiles[y * size.x + x] != TILE_GRASS) {
		return 0u;
	}

	U32 hash = hash32(
		0xC0FFEE12u ^
		(U32(x) * 0x9E3779B9u) ^
		(U32(y) * 0x85EBCA6Bu)
	);

	if ((hash & 255u) >= 16u) {  
		return 0u;
	}

	// Select one of the Grassmisc frames.
	return 1u + ((hash >> 8) % Resources::tile.grassMisc.animFrames);
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

	for (I32 y = max(tileStartY, 0); y < min(tileEndY, I32(size.y)); y++) {
		for (I32 x = max(tileStartX, 0); x < min(tileEndX, I32(size.x)); x++) {
			I32 drawX = x * tileSize - camStartX;
			I32 drawY = y * tileSize - camStartY;
			TileType tile = tiles[y * size.x + x];

			MountainRenderInfo mountainInfo{
				&Resources::tile.rock.full,
				0u
			};

			Resources::Sprite* sprite =
				tile == TILE_MOUNTAIN
				? (mountainInfo = mountain_sprite_for_tile(x, y)).sprite
				: tileSprite[tile];

			U32 richness =
				Cyber5eagull::BeeDemo::get_richness_animation_frame(x, y);

			U32 grassFrame =
				tile == TILE_GRASS
				? grass_decoration_frame(x, y)
				: 0u;

			U32 sandFrame = 0u;
			U32 sandRotation = 0u;

			if (tile == TILE_SAND) {
				B32 grassAbove = tile_is_grass(x, y - 1);
				B32 grassRight = tile_is_grass(x + 1, y);
				B32 grassBelow = tile_is_grass(x, y + 1);
				B32 grassLeft = tile_is_grass(x - 1, y);

				U32 grassMask =
					(grassAbove ? 1u : 0u) |
					(grassRight ? 2u : 0u) |
					(grassBelow ? 4u : 0u) |
					(grassLeft ? 8u : 0u);

				switch (grassMask) {
				case 0u:
					// Plain sand.
					sandFrame = 0u;
					break;

				// One grass edge: frame 1 rotated into position.
				case 1u: // Top
					sandFrame = 1u;
					sandRotation = 3u;
					break;

				case 2u: // Right
					sandFrame = 1u;
					sandRotation = 0u;
					break;

				case 4u: // Bottom
					sandFrame = 1u;
					sandRotation = 1u;
					break;

				case 8u: // Left
					sandFrame = 1u;
					sandRotation = 2u;
					break;

				// Two adjacent grass edges: frame 2 rotated into position.
				case 3u: // Top + right
					sandFrame = 2u;
					sandRotation = 0u;
					break;

				case 6u: // Right + bottom
					sandFrame = 2u;
					sandRotation = 1u;
					break;

				case 12u: // Bottom + left
					sandFrame = 2u;
					sandRotation = 2u;
					break;

				case 9u: // Left + top
					sandFrame = 2u;
					sandRotation = 3u;
					break;

				// Three grass edges: frame 3 rotated into position.
				case 7u: // Top + right + bottom
					sandFrame = 3u;
					sandRotation = 0u;
					break;

				case 14u: // Right + bottom + left
					sandFrame = 3u;
					sandRotation = 1u;
					break;

				case 13u: // Bottom + left + top
					sandFrame = 3u;
					sandRotation = 2u;
					break;

				case 11u: // Left + top + right
					sandFrame = 3u;
					sandRotation = 3u;
					break;

				// No exact textures exist for opposite/all four sides.
				case 5u: // Top + bottom
					sandFrame = 1u;
					sandRotation = 3u;
					break;

				case 10u: // Left + right
					sandFrame = 1u;
					sandRotation = 0u;
					break;

				case 15u: // All four sides
					sandFrame = 3u;
					sandRotation = 0u;
					break;
				}
			}

			BeachRenderInfo beachInfo =
				tile == TILE_BEACH
				? beach_render_info_for_tile(x, y)
				: BeachRenderInfo{ 0u, 0u, B32_FALSE };

			if (tile == TILE_MOUNTAIN) {
				Graphics::blit_sprite_rotated_cutout(
					*mountainInfo.sprite,
					drawX,
					drawY,
					tileScale,
					0u,
					mountainInfo.rotation
				);
			}
			else if (tile == TILE_BEACH && beachInfo.frame != 0u) {
				Graphics::blit_sprite_rotated_cutout(
					*sprite,
					drawX,
					drawY,
					tileScale,
					beachInfo.frame,
					beachInfo.rotation,
					beachInfo.flipX
				);
			}else {
				U32 animationFrame =
					tile == TILE_SAND
					? sandFrame
					: tile == TILE_GRASS
					? grassFrame
					: richness;

				if (tile == TILE_SAND && sandFrame != 0u) {
					Graphics::blit_sprite_rotated_cutout(
						*sprite,
						drawX,
						drawY,
						tileScale,
						animationFrame,
						sandRotation
					);
				}
				else if (tile == TILE_SAND) {
					U32 rotationHash = hash32(
						0xBADC0DEu ^
						(U32(x) * 0x9E3779B9u) ^
						(U32(y) * 0x85EBCA6Bu)
					);

					Graphics::blit_sprite_rotated_cutout(
						*sprite,
						drawX,
						drawY,
						tileScale,
						animationFrame,
						rotationHash & 3u
					);
				}
				else {
					Graphics::blit_sprite(
						*sprite,
						drawX,
						drawY,
						tileScale,
						animationFrame
					);
				}
			}
			U32 grassMiscFrame =
				tile == TILE_GRASS
				? grass_misc_decoration_frame(x, y)
				: 0u;

			if (grassMiscFrame != 0u) {
				Graphics::blit_sprite_cutout(
					Resources::tile.grassMisc,
					drawX,
					drawY,
					tileScale,
					grassMiscFrame - 1u
				);
			}
		}
	}

	render_beach(camera, tileScale);

	for (U32 x = 1; x < 7; x++) {
		I32 drawX = x * tileSize - camStartX;
		I32 drawY = World::size.y / 2 * tileSize - camStartY;

		Graphics::blit_sprite_cutout(
			Resources::tile.dockSegment,
			drawX,
			drawY,
			tileScale,
			0
		);
	}
}

// Call every frame to update the shore tiles
void beach_update(F32 dt) {
	for (U32 i = 0; i < num_beach_tiles; i++) {
		BeachTileInfo& beach = beach_tiles[i];

		beach.delay -= dt;
		if (beach.delay <= 0.0f) {
			beach.add_junk();
		}

		for (U32 junkIndex = 0; junkIndex < beach.junkNum;) {
			JunkInfo& junk = beach.junkList[junkIndex];
			junk.timeOnShore -= dt;

			if (junk.timeOnShore <= 0.0f) {
				beach.junkList[junkIndex] = beach.junkList[beach.junkNum - 1];
				beach.junkNum--;
			}
			else {
				junkIndex++;
			}
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