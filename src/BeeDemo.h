#pragma once

#include "Win32.h"
#include "Resources.h"
#include "Graphics.h"
#include "World.h"
#include "Bee.h"
#include "BeeSystem.h"
#include "BeeTasks.h"
#include "TileSpace.h"
#include "TerrainGen.h"

namespace Cyber5eagull::BeeDemo {

static constexpr F32 SPEED = 6.0F;
static constexpr U32 BEE_COUNT = 5;
static constexpr F32 ORE_WORK_SECONDS = 10.0F;
static constexpr F32 FLOWER_WORK_SECONDS = 10.0F;
static constexpr F32 SHORE_WORK_SECONDS = 1.5F;
static constexpr U32 BIG_HIVE_TASK_RADIUS = 15u;
static constexpr U32 SMALL_HIVE_TASK_RADIUS = 9u;

using TerrainGen::HiveDesc;
using TerrainGen::WorldGenerationState;

enum class CreativeBrush : U8 {
	TASK_SELECT = 0,
	ERASE,
	GRASS,
	IRON,
	COPPER,
	FLOWERS,
	SAND,
	BEACH,
	WATER,
	CONVEYOR,
	HIVE_SMALL,
	HIVE_BIG,
	COUNT
};

BeeSystem::Colony colony{};
ArenaArrayList<V2U32> conveyorTiles{};
ArenaArrayList<HiveDesc> hives{};
WorldGenerationState worldGeneration{};

FINLINE I32 world_tile_pixels(I32 worldTileScale) {
	return 16 * worldTileScale;
}

FINLINE F32 world_tile_pixels_f32(I32 worldTileScale) {
	return F32(world_tile_pixels(worldTileScale));
}

FINLINE V2F32 world_to_screen(V2F32 worldPosition, V2F32 camera, I32 worldTileScale) {
	return worldPosition * world_tile_pixels_f32(worldTileScale) - camera;
}

FINLINE B32 tile_in_bounds(V2U32 tile) {
	return TerrainGen::tile_in_bounds(tile);
}

U32 hive_task_radius(const HiveDesc& hive) {
	return hive.large ? BIG_HIVE_TASK_RADIUS : SMALL_HIVE_TASK_RADIUS;
}

B32 tile_in_any_hive_radius(V2U32 tile) {
	for (U32 i = 0; i < hives.size; i++) {
		if (TerrainGen::tile_in_hive_radius(tile, hives[i], hive_task_radius(hives[i]))) {
			return B32_TRUE;
		}
	}
	return B32_FALSE;
}

void add_conveyor_tile(V2U32 tile) {
	if (!tile_in_bounds(tile)) {
		return;
	}
	for (U32 i = 0; i < conveyorTiles.size; i++) {
		if (TileSpace::same_tile(conveyorTiles[i], tile)) {
			return;
		}
	}
	conveyorTiles.push_back(tile);
}

void remove_conveyor_tile(V2U32 tile) {
	for (U32 i = 0; i < conveyorTiles.size; i++) {
		if (TileSpace::same_tile(conveyorTiles[i], tile)) {
			conveyorTiles.remove_ordered(i);
			return;
		}
	}
}

B32 has_conveyor(V2U32 tile) {
	for (U32 i = 0; i < conveyorTiles.size; i++) {
		if (TileSpace::same_tile(conveyorTiles[i], tile)) {
			return B32_TRUE;
		}
	}
	return B32_FALSE;
}

B32 has_adjacent_conveyor(V2U32 tile) {
	using namespace TileSpace;
	V2U32 north = neighbor_tile(tile, NeighborDirection::NORTH);
	V2U32 east = neighbor_tile(tile, NeighborDirection::EAST);
	V2U32 south = neighbor_tile(tile, NeighborDirection::SOUTH);
	V2U32 west = neighbor_tile(tile, NeighborDirection::WEST);
	return (has_conveyor(north) || has_conveyor(east) || has_conveyor(south) || has_conveyor(west)) ? B32_TRUE : B32_FALSE;
}

I32 find_hive_covering_tile(V2U32 tile) {
	for (U32 i = 0; i < hives.size; i++) {
		if (TerrainGen::tile_in_hive_footprint(tile, hives[i])) {
			return I32(i);
		}
	}
	return -1;
}

void remove_hive_covering_tile(V2U32 tile) {
	I32 hiveIndex = find_hive_covering_tile(tile);
	if (hiveIndex >= 0) {
		hives.remove_ordered(U32(hiveIndex));
	}
}

void clear_tasks_in_footprint(V2U32 topLeft, V2U32 footprint) {
	for (U32 y = 0; y < footprint.y; y++) {
		for (U32 x = 0; x < footprint.x; x++) {
			V2U32 tile{ topLeft.x + x, topLeft.y + y };
			colony.unqueue_task_for_tile(tile);
		}
	}
}

void clear_conveyors_in_footprint(V2U32 topLeft, V2U32 footprint) {
	for (U32 y = 0; y < footprint.y; y++) {
		for (U32 x = 0; x < footprint.x; x++) {
			remove_conveyor_tile(V2U32{ topLeft.x + x, topLeft.y + y });
		}
	}
}

void clear_hives_in_footprint(V2U32 topLeft, V2U32 footprint) {
	for (U32 y = 0; y < footprint.y; y++) {
		for (U32 x = 0; x < footprint.x; x++) {
			remove_hive_covering_tile(V2U32{ topLeft.x + x, topLeft.y + y });
		}
	}
}

B32 can_place_hive_footprint(V2U32 topLeft, V2U32 footprint) {
	if (topLeft.x + footprint.x > World::size.x || topLeft.y + footprint.y > World::size.y) {
		return B32_FALSE;
	}
	for (U32 y = 0; y < footprint.y; y++) {
		for (U32 x = 0; x < footprint.x; x++) {
			V2U32 tile{ topLeft.x + x, topLeft.y + y };
			if (!tile_in_bounds(tile)) {
				return B32_FALSE;
			}
			if (TerrainGen::get_world_tile(tile) == World::TILE_WATER) {
				return B32_FALSE;
			}
		}
	}
	return B32_TRUE;
}

B32 tile_is_selectable_task(V2U32 tile) {
	if (!tile_in_bounds(tile) || has_conveyor(tile) || !tile_in_any_hive_radius(tile) || find_hive_covering_tile(tile) >= 0) {
		return B32_FALSE;
	}
	switch (TerrainGen::get_world_tile(tile)) {
	case World::TILE_BEACH:
	case World::TILE_GRASS_IRON:
	case World::TILE_GRASS_COPPER:
	case World::TILE_GRASS_FLOWERS:
		return B32_TRUE;
	default:
		return B32_FALSE;
	}
}

void seed_conveyors(V2U32) {
	conveyorTiles.allocator = &globalArena;
	conveyorTiles.clear();
}

BeeTasks::Task make_task_for_tile(V2U32 tile) {
	if (!tile_is_selectable_task(tile)) {
		return BeeTasks::Task{};
	}
	B32 hasNearbyConveyor = has_adjacent_conveyor(tile);
	switch (TerrainGen::get_world_tile(tile)) {
	case World::TILE_BEACH:
		return BeeTasks::make_shore_task(tile, SHORE_WORK_SECONDS);
	case World::TILE_GRASS_IRON:
	case World::TILE_GRASS_COPPER:
		return BeeTasks::make_ore_task(tile, ORE_WORK_SECONDS, hasNearbyConveyor ? B32_FALSE : B32_TRUE);
	case World::TILE_GRASS_FLOWERS:
		return BeeTasks::make_flower_task(tile, FLOWER_WORK_SECONDS, hasNearbyConveyor ? B32_FALSE : B32_TRUE);
	default:
		return BeeTasks::Task{};
	}
}

void init(V2U32 hiveTile) {
	TerrainGen::generate(&worldGeneration, &hives, hiveTile);
	seed_conveyors(hiveTile);
	colony.init(BEE_COUNT, hiveTile, SPEED);
}

void queue_tile_task(V2U32 tile) {
	if (!tile_is_selectable_task(tile) || colony.is_tile_selected(tile)) {
		return;
	}
	BeeTasks::Task task = make_task_for_tile(tile);
	if (!BeeTasks::task_is_valid(task)) {
		return;
	}
	colony.queue_task(task);
}

void unqueue_tile_task(V2U32 tile) {
	colony.unqueue_task_for_tile(tile);
}

void update(F32 dt) {
	colony.update(dt);
}

Resources::Sprite* creative_brush_sprite(CreativeBrush brush) {
	switch (brush) {
	case CreativeBrush::TASK_SELECT: return &Resources::tile.beeFly;
	case CreativeBrush::ERASE: return nullptr;
	case CreativeBrush::GRASS: return &Resources::tile.grass;
	case CreativeBrush::IRON: return &Resources::tile.grassIron;
	case CreativeBrush::COPPER: return &Resources::tile.grassCopper;
	case CreativeBrush::FLOWERS: return &Resources::tile.grassFlowers;
	case CreativeBrush::SAND: return &Resources::tile.sand;
	case CreativeBrush::BEACH: return &Resources::tile.beach;
	case CreativeBrush::WATER: return &Resources::tile.water;
	case CreativeBrush::CONVEYOR: return &Resources::tile.belt.leftToRight;
	case CreativeBrush::HIVE_SMALL: return &Resources::tile.hive;
	case CreativeBrush::HIVE_BIG: return &Resources::tile.hiveLarge;
	default: return nullptr;
	}
}

CreativeBrush creative_brush_from_tileset_cell(I32 cellX, I32 cellY, CreativeBrush fallback = CreativeBrush::TASK_SELECT) {
	if (cellX == 1 && cellY == 0) return CreativeBrush::GRASS;
	if (cellX == 2 && cellY == 0) return CreativeBrush::IRON;
	if (cellX == 2 && cellY == 1) return CreativeBrush::COPPER;
	if (cellX == 2 && cellY == 2) return CreativeBrush::FLOWERS;
	if (cellX == 0 && cellY == 1) return CreativeBrush::SAND;
	if (cellX == 1 && cellY == 1) return CreativeBrush::BEACH;
	if (cellX == 0 && cellY == 2) return CreativeBrush::WATER;
	if (cellX == 4 && cellY == 0) return CreativeBrush::CONVEYOR;
	if (cellX == 1 && cellY == 2) return CreativeBrush::HIVE_SMALL;
	if (cellX == 0 && cellY == 4) return CreativeBrush::HIVE_BIG;
	return fallback;
}

void place_hive(V2U32 topLeft, B32 large) {
	HiveDesc newHive{};
	newHive.tile = topLeft;
	newHive.large = large;
	V2U32 footprint = TerrainGen::hive_footprint_size_tiles(newHive);
	if (!can_place_hive_footprint(topLeft, footprint)) {
		return;
	}
	clear_tasks_in_footprint(topLeft, footprint);
	clear_conveyors_in_footprint(topLeft, footprint);
	clear_hives_in_footprint(topLeft, footprint);
	hives.push_back(newHive);
}

void apply_creative_brush(CreativeBrush brush, V2U32 tile) {
	if (!tile_in_bounds(tile)) {
		return;
	}

	switch (brush) {
	case CreativeBrush::TASK_SELECT: {
		queue_tile_task(tile);
	} break;

	case CreativeBrush::ERASE: {
		unqueue_tile_task(tile);
		remove_conveyor_tile(tile);
		remove_hive_covering_tile(tile);
		TerrainGen::set_world_tile(tile, World::TILE_GRASS);
	} break;

	case CreativeBrush::GRASS:
	case CreativeBrush::IRON:
	case CreativeBrush::COPPER:
	case CreativeBrush::FLOWERS:
	case CreativeBrush::SAND:
	case CreativeBrush::BEACH:
	case CreativeBrush::WATER: {
		unqueue_tile_task(tile);
		remove_conveyor_tile(tile);
		remove_hive_covering_tile(tile);
		World::TileType worldType = World::TILE_GRASS;
		switch (brush) {
		case CreativeBrush::GRASS: worldType = World::TILE_GRASS; break;
		case CreativeBrush::IRON: worldType = World::TILE_GRASS_IRON; break;
		case CreativeBrush::COPPER: worldType = World::TILE_GRASS_COPPER; break;
		case CreativeBrush::FLOWERS: worldType = World::TILE_GRASS_FLOWERS; break;
		case CreativeBrush::SAND: worldType = World::TILE_SAND; break;
		case CreativeBrush::BEACH: worldType = World::TILE_BEACH; break;
		case CreativeBrush::WATER: worldType = World::TILE_WATER; break;
		default: break;
		}
		TerrainGen::set_world_tile(tile, worldType);
	} break;

	case CreativeBrush::CONVEYOR: {
		unqueue_tile_task(tile);
		remove_hive_covering_tile(tile);
		if (TerrainGen::get_world_tile(tile) != World::TILE_WATER) {
			add_conveyor_tile(tile);
		}
	} break;

	case CreativeBrush::HIVE_SMALL: {
		place_hive(tile, B32_FALSE);
	} break;

	case CreativeBrush::HIVE_BIG: {
		place_hive(tile, B32_TRUE);
	} break;

	default: break;
	}
}

void fill_rect(I32 x, I32 y, I32 width, I32 height, RGBA8 color) {
	I32 startX = max(x, 0);
	I32 startY = max(y, 0);
	I32 endX = min(x + width, Win32::framebufferWidth);
	I32 endY = min(y + height, Win32::framebufferHeight);
	if (startX >= endX || startY >= endY) {
		return;
	}
	for (I32 py = startY; py < endY; py++) {
		RGBA8* row = Win32::framebuffer + py * Win32::framebufferWidth;
		for (I32 px = startX; px < endX; px++) {
			row[px] = color;
		}
	}
}

void fill_rect_blended(I32 x, I32 y, I32 width, I32 height, RGBA8 color) {
	I32 startX = max(x, 0);
	I32 startY = max(y, 0);
	I32 endX = min(x + width, Win32::framebufferWidth);
	I32 endY = min(y + height, Win32::framebufferHeight);
	if (startX >= endX || startY >= endY || color.a == 0) {
		return;
	}
	U32 srcA = color.a;
	U32 invA = 255u - srcA;
	for (I32 py = startY; py < endY; py++) {
		RGBA8* row = Win32::framebuffer + py * Win32::framebufferWidth;
		for (I32 px = startX; px < endX; px++) {
			RGBA8 dst = row[px];
			row[px].r = U8((U32(dst.r) * invA + U32(color.r) * srcA) / 255u);
			row[px].g = U8((U32(dst.g) * invA + U32(color.g) * srcA) / 255u);
			row[px].b = U8((U32(dst.b) * invA + U32(color.b) * srcA) / 255u);
			row[px].a = 255;
		}
	}
}

void render_tile_selection(V2U32 tile, V2F32 camera, I32 worldTileScale) {
	V2F32 screenTopLeft = world_to_screen(TileSpace::tile_to_world(tile), camera, worldTileScale);
	I32 x = I32(roundf32(screenTopLeft.x));
	I32 y = I32(roundf32(screenTopLeft.y));
	I32 tilePixels = world_tile_pixels(worldTileScale);
	fill_rect_blended(x, y, tilePixels, tilePixels, RGBA8{ 215, 215, 215, 72 });
}

void render_task_markers(V2F32 camera, I32 worldTileScale) {
	for (U32 i = 0; i < colony.queuedTasks.size; i++) {
		const BeeSystem::QueuedTask& queued = colony.queuedTasks[i];
		if (queued.selected) {
			render_tile_selection(queued.task.targetTile, camera, worldTileScale);
		}
	}
}

struct VisibleTileBounds {
	I32 minX = 0;
	I32 minY = 0;
	I32 maxX = -1;
	I32 maxY = -1;
};

VisibleTileBounds visible_tile_bounds(V2F32 camera, I32 worldTileScale) {
	VisibleTileBounds result{};
	F32 tilePixels = world_tile_pixels_f32(worldTileScale);
	result.minX = max(I32(floorf32(camera.x / tilePixels)) - 1, 0);
	result.minY = max(I32(floorf32(camera.y / tilePixels)) - 1, 0);
	result.maxX = min(I32(floorf32((camera.x + F32(Win32::framebufferWidth) - 1.0F) / tilePixels)) + 1, I32(World::size.x) - 1);
	result.maxY = min(I32(floorf32((camera.y + F32(Win32::framebufferHeight) - 1.0F) / tilePixels)) + 1, I32(World::size.y) - 1);
	return result;
}

void render_tile_overlay(V2U32 tile, V2F32 camera, I32 worldTileScale, RGBA8 color) {
	V2F32 screenTopLeft = world_to_screen(TileSpace::tile_to_world(tile), camera, worldTileScale);
	I32 tilePixels = world_tile_pixels(worldTileScale);
	fill_rect_blended(I32(roundf32(screenTopLeft.x)), I32(roundf32(screenTopLeft.y)), tilePixels, tilePixels, color);
}

void render_hive_range_overlay(const HiveDesc& hive, V2F32 camera, I32 worldTileScale) {
	VisibleTileBounds visible = visible_tile_bounds(camera, worldTileScale);
	V2U32 footprint = TerrainGen::hive_footprint_size_tiles(hive);
	I32 radiusTiles = I32(hive_task_radius(hive));
	I32 minTileX = max(I32(hive.tile.x) - radiusTiles - 1, visible.minX);
	I32 minTileY = max(I32(hive.tile.y) - radiusTiles - 1, visible.minY);
	I32 maxTileX = min(I32(hive.tile.x + footprint.x) + radiusTiles, visible.maxX);
	I32 maxTileY = min(I32(hive.tile.y + footprint.y) + radiusTiles, visible.maxY);
	if (minTileX > maxTileX || minTileY > maxTileY) {
		return;
	}

	V2F32 hiveMin = TileSpace::tile_to_world(hive.tile);
	V2F32 hiveMax = hiveMin + V2F32{ F32(footprint.x), F32(footprint.y) };
	F32 radius = F32(radiusTiles);
	F32 radiusSq = radius * radius;
	F32 borderWidthTiles = hive.large ? 1.1F : 0.9F;
	F32 borderStart = max(radius - borderWidthTiles, 0.0F);
	F32 borderStartSq = borderStart * borderStart;

	for (I32 y = minTileY; y <= maxTileY; y++) {
		for (I32 x = minTileX; x <= maxTileX; x++) {
			V2U32 tile{ U32(x), U32(y) };
			V2F32 tileCenter = TileSpace::tile_to_world_center(tile);
			V2F32 closestPoint = clamp(tileCenter, hiveMin, hiveMax);
			F32 distSq = distance_sq(tileCenter, closestPoint);
			if (distSq > radiusSq) {
				continue;
			}
			if (distSq < borderStartSq) {
				continue;
			}
			render_tile_overlay(tile, camera, worldTileScale, RGBA8{ 40, 255, 90, 104 });
		}
	}
}

void render_hive_ranges(V2F32 camera, I32 worldTileScale) {
	for (U32 i = 0; i < hives.size; i++) {
		render_hive_range_overlay(hives[i], camera, worldTileScale);
	}
}

void render_hives(V2F32 camera, I32 worldTileScale) {
	for (U32 i = 0; i < hives.size; i++) {
		const HiveDesc& hive = hives[i];
		Resources::Sprite& hiveSprite = hive.large ? Resources::tile.hiveLarge : Resources::tile.hive;
		V2U32 footprint = TerrainGen::hive_footprint_size_tiles(hive);
		I32 desiredPixelWidth = I32(footprint.x) * world_tile_pixels(worldTileScale);
		I32 spriteScale = max(desiredPixelWidth / I32(hiveSprite.width), 1);
		V2F32 screenTopLeft = world_to_screen(TileSpace::tile_to_world(hive.tile), camera, worldTileScale);
		Graphics::blit_sprite_cutout(hiveSprite, I32(roundf32(screenTopLeft.x)), I32(roundf32(screenTopLeft.y)), spriteScale, 0);
	}
}

void render_conveyors(V2F32 camera, I32 worldTileScale, F64 frameTimeSeconds) {
	Resources::Sprite& conveyorSprite = Resources::tile.belt.leftToRight;
	U32 animFrame = U32(fractf64(frameTimeSeconds * 2.5) * F32(conveyorSprite.animFrames)) % conveyorSprite.animFrames;
	for (U32 i = 0; i < conveyorTiles.size; i++) {
		V2F32 screenTopLeft = world_to_screen(TileSpace::tile_to_world(conveyorTiles[i]), camera, worldTileScale);
		Graphics::blit_sprite_cutout(conveyorSprite, I32(roundf32(screenTopLeft.x)), I32(roundf32(screenTopLeft.y)), worldTileScale, animFrame);
	}
}

void render_bee_progress_bar(const Bee::Bee& bee, V2F32 camera, I32 worldTileScale) {
	if (bee.state != BeeTasks::State::STATE_WORKING || !bee.hasTask || bee.activeTask.workDurationSeconds <= 0.0F) {
		return;
	}
	F32 progress = bee.work_progress01();
	I32 barWidth = max(18 * worldTileScale / 4, 12);
	I32 barHeight = max(4 * worldTileScale / 4, 3);
	V2F32 beeScreenCenter = world_to_screen(bee.position, camera, worldTileScale);
	I32 x = I32(roundf32(beeScreenCenter.x)) - barWidth / 2;
	I32 y = I32(roundf32(beeScreenCenter.y)) - max(12 * worldTileScale / 4, 10);
	fill_rect(x - 1, y - 1, barWidth + 2, barHeight + 2, RGBA8{ 0, 0, 0, 255 });
	fill_rect(x, y, barWidth, barHeight, RGBA8{ 60, 60, 60, 255 });
	fill_rect(x, y, I32(roundf32(F32(barWidth) * progress)), barHeight, RGBA8{ 40, 220, 80, 255 });
}

void render_bee(const Bee::Bee& bee, V2F32 camera, I32 worldTileScale, F64 frameTimeSeconds) {
	if (bee.inside_hive()) {
		return;
	}
	Resources::Sprite& beeSprite = bee.state == BeeTasks::State::STATE_WORKING ? Resources::tile.beeMine : Resources::tile.beeFly;
	F32 animTurns = fractf64(frameTimeSeconds * 6.0 + bee.flightPhaseTurns);
	U32 animFrame = U32(animTurns * F32(beeSprite.animFrames)) % beeSprite.animFrames;
	V2F32 beeScreenCenter = world_to_screen(bee.position, camera, worldTileScale);
	V2F32 beeScreenTopLeft = beeScreenCenter - V2F32{ F32(beeSprite.width * worldTileScale) * 0.5F, F32(beeSprite.height * worldTileScale) * 0.5F };
	Graphics::blit_sprite_cutout(beeSprite, I32(roundf32(beeScreenTopLeft.x)), I32(roundf32(beeScreenTopLeft.y)), worldTileScale, animFrame);
	render_bee_progress_bar(bee, camera, worldTileScale);
}

void render_bees(V2F32 camera, I32 worldTileScale, F64 frameTimeSeconds) {
	for (U32 beeIndex = 0; beeIndex < colony.bees.size; beeIndex++) {
		render_bee(colony.bees[beeIndex], camera, worldTileScale, frameTimeSeconds);
	}
}

}
