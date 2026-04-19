#pragma once

#include "World.h"
#include "TileSpace.h"

namespace Cyber5eagull::TerrainGen {

static constexpr U32 MAX_WORLD_MAP_TILES = 256u * 256u;
static constexpr U32 MAX_PATCH_FRONTIER = 128u;
static constexpr U32 MAX_PATCH_TILES = 128u;
static constexpr U32 HIVE_RESOURCE_SAFE_RADIUS = 4u;

struct HiveDesc {
	V2U32 tile{};
	B32 large = B32_FALSE;
};

struct WorldGenerationState {
	U32 seed = 0u;
	U8 resourceClaimMask[MAX_WORLD_MAP_TILES]{};
	U8 resourceBlockMask[MAX_WORLD_MAP_TILES]{};
};

FINLINE B32 tile_in_bounds(V2U32 tile) {
	return tile.x < World::size.x && tile.y < World::size.y ? B32_TRUE : B32_FALSE;
}

FINLINE U32 world_tile_index(V2U32 tile) {
	return U32(tile.y * World::size.x + tile.x);
}

FINLINE U32 world_tile_count() {
	return World::size.x * World::size.y;
}

FINLINE V2U32 offset_tile_signed(V2U32 tile, I32 dx, I32 dy) {
	I32 x = clamp(I32(tile.x) + dx, 0, I32(World::size.x) - 1);
	I32 y = clamp(I32(tile.y) + dy, 0, I32(World::size.y) - 1);
	return V2U32{ U32(x), U32(y) };
}

void set_world_tile(V2U32 tile, World::TileType type) {
	if (!tile_in_bounds(tile)) {
		return;
	}
	World::tiles[world_tile_index(tile)] = type;
}

World::TileType get_world_tile(V2U32 tile) {
	if (!tile_in_bounds(tile)) {
		return World::TILE_UNDEF;
	}
	return World::tiles[world_tile_index(tile)];
}

V2U32 hive_footprint_size_tiles(const HiveDesc& hive) {
	return hive.large ? V2U32{ 2u, 2u } : V2U32{ 1u, 1u };
}

B32 tile_in_hive_footprint(V2U32 tile, const HiveDesc& hive) {
	V2U32 size = hive_footprint_size_tiles(hive);
	return tile.x >= hive.tile.x && tile.y >= hive.tile.y && tile.x < hive.tile.x + size.x && tile.y < hive.tile.y + size.y ? B32_TRUE : B32_FALSE;
}

V2F32 hive_center_world_position(const HiveDesc& hive) {
	V2U32 size = hive_footprint_size_tiles(hive);
	return TileSpace::tile_to_world(hive.tile) + V2F32{ F32(size.x) * 0.5F, F32(size.y) * 0.5F };
}

B32 tile_in_hive_radius(V2U32 tile, const HiveDesc& hive, U32 radiusTiles) {
	V2F32 tileCenter = TileSpace::tile_to_world_center(tile);
	V2U32 size = hive_footprint_size_tiles(hive);
	V2F32 hiveMin = TileSpace::tile_to_world(hive.tile);
	V2F32 hiveMax = hiveMin + V2F32{ F32(size.x), F32(size.y) };
	V2F32 closestPoint = clamp(tileCenter, hiveMin, hiveMax);
	F32 radius = F32(radiusTiles);
	return distance_sq(tileCenter, closestPoint) <= radius * radius ? B32_TRUE : B32_FALSE;
}

void reset_hives(ArenaArrayList<HiveDesc>* hives, V2U32 mainHiveTile) {
	hives->allocator = &globalArena;
	hives->clear();
	HiveDesc& hive = hives->push_back_zeroed();
	hive.tile = mainHiveTile;
	hive.large = B32_TRUE;
}

B32 tile_reserved_for_hive(V2U32 tile, const ArenaArrayList<HiveDesc>& hives) {
	for (U32 i = 0; i < hives.size; i++) {
		if (tile_in_hive_footprint(tile, hives[i])) {
			return B32_TRUE;
		}
	}
	return B32_FALSE;
}

B32 tile_in_any_hive_safe_zone(V2U32 tile, const ArenaArrayList<HiveDesc>& hives) {
	for (U32 i = 0; i < hives.size; i++) {
		if (tile_in_hive_radius(tile, hives[i], HIVE_RESOURCE_SAFE_RADIUS)) {
			return B32_TRUE;
		}
	}
	return B32_FALSE;
}

void clear_resource_masks(WorldGenerationState* state) {
	U32 tileCount = min(world_tile_count(), MAX_WORLD_MAP_TILES);
	memset(state->resourceClaimMask, 0, tileCount * sizeof(state->resourceClaimMask[0]));
	memset(state->resourceBlockMask, 0, tileCount * sizeof(state->resourceBlockMask[0]));
}

void mark_resource_padding(WorldGenerationState* state, V2U32 tile, I32 radius) {
	for (I32 y = max(I32(tile.y) - radius, 0); y <= min(I32(tile.y) + radius, I32(World::size.y) - 1); y++) {
		for (I32 x = max(I32(tile.x) - radius, 0); x <= min(I32(tile.x) + radius, I32(World::size.x) - 1); x++) {
			state->resourceBlockMask[world_tile_index(V2U32{ U32(x), U32(y) })] = 1u;
		}
	}
}

B32 can_claim_resource_tile(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives, V2U32 tile, B32 respectHiveSafeZone) {
	if (!tile_in_bounds(tile)) {
		return B32_FALSE;
	}
	U32 index = world_tile_index(tile);
	if (state->resourceClaimMask[index] || state->resourceBlockMask[index]) {
		return B32_FALSE;
	}
	if (get_world_tile(tile) != World::TILE_GRASS) {
		return B32_FALSE;
	}
	if (tile_reserved_for_hive(tile, hives)) {
		return B32_FALSE;
	}
	if (respectHiveSafeZone && tile_in_any_hive_safe_zone(tile, hives)) {
		return B32_FALSE;
	}
	return B32_TRUE;
}

void claim_resource_tile(WorldGenerationState* state, V2U32 tile, World::TileType type) {
	set_world_tile(tile, type);
	state->resourceClaimMask[world_tile_index(tile)] = 1u;
}

void seed_base_terrain(WorldGenerationState* state) {
	for (U32 y = 0; y < World::size.y; y++) {
		U32 coastHash = hash32(state->seed ^ (y / 2u) ^ 0xA511E9B3u);
		U32 waterCols = 3u + (coastHash & 1u);
		U32 beachCols = 1u;
		U32 sandCols = 1u + ((coastHash >> 5) & 1u);
		if ((coastHash >> 10) & 1u) {
			sandCols++;
		}

		U32 waterEndX = min(waterCols, World::size.x);
		U32 beachEndX = min(waterEndX + beachCols, World::size.x);
		U32 sandEndX = min(beachEndX + sandCols, World::size.x);

		for (U32 x = 0; x < World::size.x; x++) {
			V2U32 tile{ x, y };
			if (x < waterEndX) {
				set_world_tile(tile, World::TILE_WATER);
			}
			else if (x < beachEndX) {
				set_world_tile(tile, World::TILE_BEACH);
				World::push_beach_tile(tile);
			}
			else if (x < sandEndX) {
				set_world_tile(tile, World::TILE_SAND);
			}
			else {
				set_world_tile(tile, World::TILE_GRASS);
			}
		}
	}
}

B32 try_grow_irregular_patch(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives, V2U32 start, World::TileType type, U32 targetSize, U32 seed, B32 respectHiveSafeZone, I32 paddingRadius) {
	if (!can_claim_resource_tile(state, hives, start, respectHiveSafeZone)) {
		return B32_FALSE;
	}

	static const I32 NEIGHBOR_OFFSETS[8][2] = {
		{ 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
		{ 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 }
	};

	V2U32 frontier[MAX_PATCH_FRONTIER]{};
	V2U32 placed[MAX_PATCH_TILES]{};
	U32 frontierCount = 0u;
	U32 placedCount = 0u;

	claim_resource_tile(state, start, type);
	frontier[frontierCount++] = start;
	placed[placedCount++] = start;

	U32 maxIterations = max(targetSize * 10u, 24u);
	for (U32 step = 0; step < maxIterations && frontierCount > 0u && placedCount < targetSize; step++) {
		U32 stepHash = hash32(seed ^ (step * 0x9E3779B9u));
		U32 baseIndex = stepHash % frontierCount;
		V2U32 base = ((stepHash >> 29) & 1u) && placedCount > 0u ? placed[(stepHash >> 16) % placedCount] : frontier[baseIndex];
		B32 expanded = B32_FALSE;

		for (U32 tryIndex = 0; tryIndex < 8u; tryIndex++) {
			U32 tryHash = hash32(stepHash ^ (tryIndex * 0x85EBCA6Bu));
			const I32* offset = NEIGHBOR_OFFSETS[tryHash & 7u];
			I32 nx = I32(base.x) + offset[0];
			I32 ny = I32(base.y) + offset[1];
			if (nx < 0 || ny < 0 || nx >= I32(World::size.x) || ny >= I32(World::size.y)) {
				continue;
			}

			V2U32 nextTile{ U32(nx), U32(ny) };
			if (!can_claim_resource_tile(state, hives, nextTile, respectHiveSafeZone)) {
				continue;
			}
			if (placedCount > 2u && ((tryHash >> 9) & 3u) == 0u) {
				continue;
			}

			claim_resource_tile(state, nextTile, type);
			if (placedCount < MAX_PATCH_TILES) {
				placed[placedCount++] = nextTile;
			}
			if (frontierCount < MAX_PATCH_FRONTIER) {
				frontier[frontierCount++] = nextTile;
			}
			expanded = B32_TRUE;
			break;
		}

		if (!expanded) {
			frontier[baseIndex] = frontier[frontierCount - 1u];
			frontierCount--;
		}
	}

	for (U32 i = 0; i < placedCount; i++) {
		mark_resource_padding(state, placed[i], paddingRadius);
	}
	return placedCount > 0u ? B32_TRUE : B32_FALSE;
}

void generate_random_patches(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives, World::TileType type, U32 patchCount, U32 minPatchSize, U32 maxPatchSize, U32 seedBase) {
	if (World::size.x <= 14u || World::size.y <= 10u) {
		return;
	}

	U32 leftMargin = 10u;
	U32 rightMargin = 3u;
	U32 topMargin = 3u;
	U32 bottomMargin = 3u;
	if (World::size.x <= leftMargin + rightMargin || World::size.y <= topMargin + bottomMargin) {
		return;
	}

	U32 usableWidth = World::size.x - leftMargin - rightMargin;
	U32 usableHeight = World::size.y - topMargin - bottomMargin;
	for (U32 patchIndex = 0; patchIndex < patchCount; patchIndex++) {
		B32 placedPatch = B32_FALSE;
		for (U32 attempt = 0; attempt < 96u; attempt++) {
			U32 hash = hash32(state->seed ^ seedBase ^ (patchIndex * 0x9E3779B9u) ^ (attempt * 0x85EBCA6Bu));
			V2U32 startTile{
				leftMargin + (hash % usableWidth),
				topMargin + ((hash >> 12) % usableHeight)
			};
			U32 patchSize = minPatchSize + ((hash >> 24) % (maxPatchSize - minPatchSize + 1u));
			if (try_grow_irregular_patch(state, hives, startTile, type, patchSize, hash ^ seedBase, B32_TRUE, 1)) {
				placedPatch = B32_TRUE;
				break;
			}
		}
		if (!placedPatch) {
			break;
		}
	}
}

B32 can_place_mountain_tile(const ArenaArrayList<HiveDesc>& hives, V2U32 tile) {
	if (!tile_in_bounds(tile)) {
		return B32_FALSE;
	}
	if (tile_reserved_for_hive(tile, hives) || tile_in_any_hive_safe_zone(tile, hives)) {
		return B32_FALSE;
	}
	return get_world_tile(tile) == World::TILE_GRASS ? B32_TRUE : B32_FALSE;
}

void claim_mountain_tile(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives, V2U32 tile) {
	if (!can_place_mountain_tile(hives, tile)) {
		return;
	}
	U32 index = world_tile_index(tile);
	set_world_tile(tile, World::TILE_MOUNTAIN);
	state->resourceClaimMask[index] = 1u;
	state->resourceBlockMask[index] = 1u;
}

V2F32 normalize_v2_safe(V2F32 v, F32 epsilon) {
	F32 lenSq = length_sq(v);
	if (lenSq <= epsilon * epsilon) {
		return V2F32{};
	}
	F32 invLen = 1.0F / sqrtf32(lenSq);
	return v * invLen;
}

void stamp_mountain_blob(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives, V2F32 centerWorld, F32 radiusX, F32 radiusY, U32 seed) {
	I32 minX = max(I32(floorf32(centerWorld.x - radiusX - 1.0F)), 0);
	I32 maxX = min(I32(ceilf32(centerWorld.x + radiusX + 1.0F)), I32(World::size.x) - 1);
	I32 minY = max(I32(floorf32(centerWorld.y - radiusY - 1.0F)), 0);
	I32 maxY = min(I32(ceilf32(centerWorld.y + radiusY + 1.0F)), I32(World::size.y) - 1);
	for (I32 y = minY; y <= maxY; y++) {
		for (I32 x = minX; x <= maxX; x++) {
			V2U32 tile{ U32(x), U32(y) };
			V2F32 tileCenter = TileSpace::tile_to_world_center(tile);
			F32 dx = (tileCenter.x - centerWorld.x) / max(radiusX, 0.5F);
			F32 dy = (tileCenter.y - centerWorld.y) / max(radiusY, 0.5F);
			F32 dist = dx * dx + dy * dy;
			U32 noiseHash = hash32(seed ^ (x * 0x9E3779B9u) ^ (y * 0x85EBCA6Bu));
			F32 threshold = 1.0F + F32((noiseHash >> 24) & 7u) * 0.04F;
			if (dist <= threshold) {
				claim_mountain_tile(state, hives, tile);
			}
		}
	}
}

void generate_mountain_range(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives, V2U32 startTile, U32 rangeSteps, F32 baseRadius, U32 seed) {
	V2F32 position = TileSpace::tile_to_world_center(startTile);
	F32 angle = 0.14F + F32(seed & 255u) * (1.0F / 2048.0F);
	F32 sinAngle = 0.0F;
	F32 cosAngle = sincosf32(&sinAngle, angle);
	V2F32 direction = normalize_v2_safe(V2F32{ cosAngle * 0.75F, sinAngle }, 0.0001F);
	for (U32 step = 0; step < rangeSteps; step++) {
		F32 t = rangeSteps > 1u ? F32(step) / F32(rangeSteps - 1u) : 0.0F;
		U32 stepHash = hash32(seed ^ (step * 0x9E3779B9u));
		F32 radiusX = baseRadius + 0.7F * sinf32(t * 0.5F) + F32((stepHash >> 24) & 3u) * 0.15F;
		F32 radiusY = baseRadius * 0.85F + F32((stepHash >> 20) & 3u) * 0.12F;
		stamp_mountain_blob(state, hives, position, radiusX, radiusY, stepHash);
		if ((stepHash & 7u) == 0u) {
			V2F32 side = get_orthogonal(direction);
			F32 branchOffset = ((stepHash >> 8) & 1u) ? 1.1F : -1.1F;
			stamp_mountain_blob(state, hives, position + side * branchOffset, radiusX * 0.65F, radiusY * 0.65F, stepHash ^ 0xA57E2D1Bu);
		}
		F32 turnDelta = (F32((stepHash >> 12) & 31u) - 15.0F) * 0.0009F;
		angle += turnDelta;
		cosAngle = sincosf32(&sinAngle, angle);
		direction = normalize_v2_safe(V2F32{ cosAngle * 0.75F, sinAngle }, 0.0001F);
		position += direction * (1.05F + F32((stepHash >> 17) & 3u) * 0.18F);
		if (position.x < 12.0F || position.x > F32(World::size.x) - 4.0F || position.y < 2.0F || position.y > F32(World::size.y) - 3.0F) {
			break;
		}
	}
}

void generate_mountain_ranges(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives) {
	if (World::size.x < 24u || World::size.y < 18u) {
		return;
	}
	U32 rangeCount = 2u + (state->seed & 1u);
	for (U32 rangeIndex = 0; rangeIndex < rangeCount; rangeIndex++) {
		for (U32 attempt = 0; attempt < 24u; attempt++) {
			U32 hash = hash32(state->seed ^ 0x6D2B79F5u ^ (rangeIndex * 0x9E3779B9u) ^ (attempt * 0x85EBCA6Bu));
			V2U32 startTile{
				World::size.x / 3u + (hash % max(World::size.x / 2u, 1u)),
				4u + ((hash >> 11) % max(World::size.y - 8u, 1u))
			};
			if (!can_place_mountain_tile(hives, startTile)) {
				continue;
			}
			U32 steps = 10u + ((hash >> 22) & 15u);
			F32 radius = 1.4F + F32((hash >> 18) & 7u) * 0.18F;
			generate_mountain_range(state, hives, startTile, steps, radius, hash);
			break;
		}
	}
}

void seed_starter_patches(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives, V2U32 hiveTile) {
	try_grow_irregular_patch(state, hives, offset_tile_signed(hiveTile, 5, -2), World::TILE_GRASS_IRON, 7u, state->seed ^ 0x11111111u, B32_FALSE, 1);
	try_grow_irregular_patch(state, hives, offset_tile_signed(hiveTile, 5, 2), World::TILE_GRASS_COPPER, 7u, state->seed ^ 0x22222222u, B32_FALSE, 1);
	for (U32 attempt = 0; attempt < 8u; attempt++) {
		U32 hash = hash32(state->seed ^ 0x33333333u ^ (attempt * 0x9E3779B9u));
		I32 dx = 6 + I32((hash >> 3) & 3u);
		I32 dy = I32(hash & 1u ? (2 + I32((hash >> 7) & 3u)) : -(2 + I32((hash >> 7) & 3u)));
		if (try_grow_irregular_patch(state, hives, offset_tile_signed(hiveTile, dx, dy), World::TILE_GRASS_FLOWERS, 10u + ((hash >> 12) & 3u), hash, B32_FALSE, 1)) {
			break;
		}
	}
}

B32 any_tile_of_type_in_radius(V2U32 center, I32 radius, World::TileType type) {
	for (I32 y = max(I32(center.y) - radius, 0); y <= min(I32(center.y) + radius, I32(World::size.y) - 1); y++) {
		for (I32 x = max(I32(center.x) - radius, 0); x <= min(I32(center.x) + radius, I32(World::size.x) - 1); x++) {
			if (get_world_tile(V2U32{ U32(x), U32(y) }) == type) {
				return B32_TRUE;
			}
		}
	}
	return B32_FALSE;
}

void ensure_nearby_flowers(WorldGenerationState* state, const ArenaArrayList<HiveDesc>& hives, V2U32 hiveTile) {
	if (any_tile_of_type_in_radius(hiveTile, 12, World::TILE_GRASS_FLOWERS)) {
		return;
	}
	for (U32 attempt = 0; attempt < 16u; attempt++) {
		U32 hash = hash32(state->seed ^ 0xF10A0F55u ^ (attempt * 0x85EBCA6Bu));
		I32 dx = 6 + I32((hash >> 4) & 5u);
		I32 dy = I32((hash >> 1) & 1u ? (I32((hash >> 9) & 5u) - 2) : -(I32((hash >> 9) & 5u) - 2));
		if (try_grow_irregular_patch(state, hives, offset_tile_signed(hiveTile, dx, dy), World::TILE_GRASS_FLOWERS, 11u + ((hash >> 13) & 3u), hash, B32_FALSE, 1)) {
			return;
		}
	}
}

void generate(WorldGenerationState* state, ArenaArrayList<HiveDesc>* hives, V2U32 mainHiveTile) {
	U32 timeSeed = U32(current_time_seconds() * 1000000.0);
	state->seed = hash32(timeSeed ^ (World::size.x * 4099u) ^ (World::size.y * 131u) ^ 0xC5E4F123u);

	reset_hives(hives, mainHiveTile);
	clear_resource_masks(state);
	seed_base_terrain(state);
	generate_mountain_ranges(state, *hives);
	seed_starter_patches(state, *hives, mainHiveTile);
	generate_random_patches(state, *hives, World::TILE_GRASS_IRON, 3u, 7u, 12u, 0x51A91D1Du);
	generate_random_patches(state, *hives, World::TILE_GRASS_COPPER, 3u, 7u, 12u, 0xC0FFEE11u);
	generate_random_patches(state, *hives, World::TILE_GRASS_FLOWERS, 4u, 8u, 14u, 0xF10A0F55u);
	ensure_nearby_flowers(state, *hives, mainHiveTile);
}

}
