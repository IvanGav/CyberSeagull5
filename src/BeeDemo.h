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
#include "Factory.h"

namespace Cyber5eagull::BeeDemo {

static constexpr F32 SPEED = 6.0F;
static constexpr U32 BEE_COUNT = 5;
static constexpr F32 ORE_WORK_SECONDS = 10.0F;
static constexpr F32 FLOWER_WORK_SECONDS = 10.0F;
static constexpr F32 SHORE_WORK_SECONDS = 1.5F;
static constexpr U32 BIG_HIVE_TASK_RADIUS = 15u;
static constexpr U32 SMALL_HIVE_TASK_RADIUS = 9u;
static constexpr U32 POLLEN_PER_HONEY = 3u;
static constexpr F32 HONEY_CONVERSION_SECONDS = 10.0F;

static constexpr U8 STARTING_IRON_PER_TILE = 8;
static constexpr U8 STARTING_COPPER_PER_TILE = 8;
static constexpr U8 STARTING_FLOWER_PER_TILE = 12;

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
    MOUNTAIN,
    CONVEYOR,
    ASSEMBLER_SMALL,
    ASSEMBLER_LARGE,
    SPLITTER,
    MERGER,
    HIVE_SMALL,
    HIVE_BIG,
    COUNT
};

BeeSystem::Colony colony{};
ArenaArrayList<HiveDesc> hives{};
WorldGenerationState worldGeneration{};
F32 mainHiveHoneyProgress = 0.0F;

// Runtime resource counts per tile.
U8 ironRemaining[TerrainGen::MAX_WORLD_MAP_TILES]{};
U8 copperRemaining[TerrainGen::MAX_WORLD_MAP_TILES]{};
U8 flowerRemaining[TerrainGen::MAX_WORLD_MAP_TILES]{};

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

FINLINE U32 tile_resource_index(V2U32 tile) {
    return TerrainGen::world_tile_index(tile);
}

U32 hive_task_radius(const HiveDesc& hive) {
    return hive.large ? BIG_HIVE_TASK_RADIUS : SMALL_HIVE_TASK_RADIUS;
}

const HiveDesc* main_hive() {
    return hives.size != 0 ? &hives[0] : nullptr;
}

B32 same_hive_tile(V2U32 a, V2U32 b) {
    return a.x == b.x && a.y == b.y ? B32_TRUE : B32_FALSE;
}

U32 bees_inside_hive(const HiveDesc& hive) {
    return colony.count_bees_inside_home_tile(hive.tile);
}

U32 bees_inside_main_hive() {
    const HiveDesc* hive = main_hive();
    return hive ? bees_inside_hive(*hive) : 0u;
}

U32 bees_inside_any_hive() {
    U32 result = 0;
    for (U32 i = 0; i < colony.bees.size; i++) {
        if (colony.bees[i].inside_hive()) {
            result++;
        }
    }
    return result;
}

BeeSystem::HomeAnchor home_anchor_for_hive(const HiveDesc& hive) {
    BeeSystem::HomeAnchor anchor{};
    anchor.tile = hive.tile;
    V2U32 footprint = TerrainGen::hive_footprint_size_tiles(hive);
    F32 offsetX = F32(footprint.x) * 0.5F;
    F32 offsetY = F32(footprint.y) * 0.5F;

    // Do not place bees exactly on integer world coordinates for multi-tile hives.
    // That puts them on tile boundaries and makes launch/collision checks act stupid.
    if ((footprint.x & 1u) == 0u) {
        offsetX -= 0.05F;
    }
    if ((footprint.y & 1u) == 0u) {
        offsetY -= 0.05F;
    }

    anchor.offsetWorld = V2F32{ offsetX, offsetY };
    return anchor;
}

BeeSystem::HomeAnchor nearest_hive_anchor_for_tile(V2U32 tile) {
    if (hives.size == 0) {
        return colony.defaultHome;
    }

    V2F32 targetCenter = TileSpace::tile_to_world_center(tile);
    U32 bestHiveIndex = 0;
    F32 bestDistSq = distance_sq(TerrainGen::hive_center_world_position(hives[0]), targetCenter);

    for (U32 i = 1; i < hives.size; i++) {
        F32 distSq = distance_sq(TerrainGen::hive_center_world_position(hives[i]), targetCenter);
        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestHiveIndex = i;
        }
    }

    return home_anchor_for_hive(hives[bestHiveIndex]);
}

BeeSystem::HomeAnchor select_nearest_hive_anchor(V2U32 targetTile, void*) {
    return nearest_hive_anchor_for_tile(targetTile);
}

B32 tile_in_any_hive_radius(V2U32 tile) {
    for (U32 i = 0; i < hives.size; i++) {
        if (TerrainGen::tile_in_hive_radius(tile, hives[i], hive_task_radius(hives[i]))) {
            return B32_TRUE;
        }
    }
    return B32_FALSE;
}

B32 has_conveyor(V2U32 tile) {
    return Factory::has_belt(V2U{ tile.x, tile.y });
}

void remove_conveyor_tile(V2U32 tile) {
	if (Factory::has_belt(tile)) {
		Factory::remove_machine(tile);
	}
}

void add_conveyor_tile(V2U32 tile) {
    Factory::place_belt(V2U{ tile.x, tile.y });
}
void remove_machine_tile(V2U32 tile) {
    if (Factory::has_machine(V2U{ tile.x, tile.y })) {
        Factory::remove_machine(V2U{ tile.x, tile.y });
    }
}

void sync_beach_runtime_tile(V2U32 tile) {
    World::sync_beach_tile_with_world(V2U{ tile.x, tile.y });
}

static constexpr F32 BEE_PATH_CLEARANCE_RADIUS = 0.18F;
static constexpr F32 BEE_SEGMENT_SAMPLE_STEP = 0.18F;

B32 tile_blocks_bee_path(V2U32 tile, V2U32 startTile, V2U32 goalTile);

FINLINE B32 bee_position_hits_blocker(V2F32 position, V2U32 startTile, V2U32 goalTile, void*) {
    if (position.x < 0.0F || position.y < 0.0F || position.x >= F32(World::size.x) || position.y >= F32(World::size.y)) {
        return B32_TRUE;
    }

    // Give bees clean launch/landing clearance inside their source/target hive footprint.
    for (U32 i = 0; i < hives.size; i++) {
        const HiveDesc& hive = hives[i];
        if (!TerrainGen::tile_in_hive_footprint(startTile, hive) && !TerrainGen::tile_in_hive_footprint(goalTile, hive)) {
            continue;
        }

        V2U32 footprint = TerrainGen::hive_footprint_size_tiles(hive);
        F32 minX = F32(hive.tile.x) - BEE_PATH_CLEARANCE_RADIUS;
        F32 minY = F32(hive.tile.y) - BEE_PATH_CLEARANCE_RADIUS;
        F32 maxX = F32(hive.tile.x + footprint.x) + BEE_PATH_CLEARANCE_RADIUS;
        F32 maxY = F32(hive.tile.y + footprint.y) + BEE_PATH_CLEARANCE_RADIUS;
        if (position.x >= minX && position.x <= maxX && position.y >= minY && position.y <= maxY) {
            return B32_FALSE;
        }
    }

    V2U32 centerTile = TileSpace::world_to_tile(position);
    for (I32 dy = -1; dy <= 1; dy++) {
        for (I32 dx = -1; dx <= 1; dx++) {
            I32 tx = I32(centerTile.x) + dx;
            I32 ty = I32(centerTile.y) + dy;
            if (tx < 0 || ty < 0 || tx >= I32(World::size.x) || ty >= I32(World::size.y)) {
                continue;
            }

            V2U32 tile{ U32(tx), U32(ty) };
            if (!tile_blocks_bee_path(tile, startTile, goalTile)) {
                continue;
            }

            F32 minX = F32(tx);
            F32 minY = F32(ty);
            F32 maxX = minX + 1.0F;
            F32 maxY = minY + 1.0F;
            F32 closestX = clamp(position.x, minX, maxX);
            F32 closestY = clamp(position.y, minY, maxY);
            V2F32 delta = position - V2F32{ closestX, closestY };
            if (length_sq(delta) < BEE_PATH_CLEARANCE_RADIUS * BEE_PATH_CLEARANCE_RADIUS) {
                return B32_TRUE;
            }
        }
    }

    return B32_FALSE;
}

B32 tile_blocks_bee_path(V2U32 tile, V2U32 startTile, V2U32 goalTile) {
    if (!tile_in_bounds(tile)) {
        return B32_TRUE;
    }
    if (TileSpace::same_tile(tile, startTile) || TileSpace::same_tile(tile, goalTile)) {
        return B32_FALSE;
    }

    World::TileType tileType = TerrainGen::get_world_tile(tile);
    if (tileType == World::TILE_WATER || tileType == World::TILE_MOUNTAIN) {
        return B32_TRUE;
    }

    Factory::Machine* machine = Factory::get_machine_from_tile(V2U{ tile.x, tile.y });
    if (machine && !Factory::machine_is_belt(machine)) {
        return B32_TRUE;
    }

    for (U32 i = 0; i < hives.size; i++) {
        if (TerrainGen::tile_in_hive_footprint(tile, hives[i])) {
            if (TerrainGen::tile_in_hive_footprint(startTile, hives[i]) || TerrainGen::tile_in_hive_footprint(goalTile, hives[i])) {
                return B32_FALSE;
            }
            return B32_TRUE;
        }
    }

    return B32_FALSE;
}

void append_preferred_directions(V2U32 current, V2U32 goalTile, TileSpace::NeighborDirection* directionsOut) {
    I32 dx = I32(goalTile.x) - I32(current.x);
    I32 dy = I32(goalTile.y) - I32(current.y);
    TileSpace::NeighborDirection xToward = dx >= 0 ? TileSpace::NeighborDirection::EAST : TileSpace::NeighborDirection::WEST;
    TileSpace::NeighborDirection xAway = dx >= 0 ? TileSpace::NeighborDirection::WEST : TileSpace::NeighborDirection::EAST;
    TileSpace::NeighborDirection yToward = dy >= 0 ? TileSpace::NeighborDirection::SOUTH : TileSpace::NeighborDirection::NORTH;
    TileSpace::NeighborDirection yAway = dy >= 0 ? TileSpace::NeighborDirection::NORTH : TileSpace::NeighborDirection::SOUTH;

    if (abs(dx) >= abs(dy)) {
        directionsOut[0] = xToward;
        directionsOut[1] = yToward;
        directionsOut[2] = yAway;
        directionsOut[3] = xAway;
    }
    else {
        directionsOut[0] = yToward;
        directionsOut[1] = xToward;
        directionsOut[2] = xAway;
        directionsOut[3] = yAway;
    }
}

B32 find_bee_path(V2U32 startTile, V2U32 goalTile, V2U32* pathTilesOut, U32* pathCountOut, U32 maxPathTiles, void*) {
    *pathCountOut = 0;
    if (!tile_in_bounds(startTile) || !tile_in_bounds(goalTile) || maxPathTiles == 0) {
        return B32_FALSE;
    }
    if (TileSpace::same_tile(startTile, goalTile)) {
        pathTilesOut[0] = startTile;
        *pathCountOut = 1;
        return B32_TRUE;
    }

    U32 tileCount = World::size.x * World::size.y;
    static I32 parents[TerrainGen::MAX_WORLD_MAP_TILES];
    static U8 visited[TerrainGen::MAX_WORLD_MAP_TILES];
    static V2U32 queue[TerrainGen::MAX_WORLD_MAP_TILES];
    memset(visited, 0, tileCount * sizeof(visited[0]));
    for (U32 i = 0; i < tileCount; i++) {
        parents[i] = -1;
    }

    U32 queueRead = 0;
    U32 queueWrite = 0;
    U32 startIndex = tile_resource_index(startTile);
    U32 goalIndex = tile_resource_index(goalTile);
    visited[startIndex] = 1u;
    queue[queueWrite++] = startTile;

    while (queueRead < queueWrite) {
        V2U32 current = queue[queueRead++];
        if (TileSpace::same_tile(current, goalTile)) {
            break;
        }

        TileSpace::NeighborDirection directions[4]{};
        append_preferred_directions(current, goalTile, directions);
        for (U32 i = 0; i < 4; i++) {
            V2U32 next = TileSpace::neighbor_tile(current, directions[i]);
            if (!tile_in_bounds(next)) {
                continue;
            }
            U32 nextIndex = tile_resource_index(next);
            if (visited[nextIndex] || tile_blocks_bee_path(next, startTile, goalTile)) {
                continue;
            }
            visited[nextIndex] = 1u;
            parents[nextIndex] = I32(tile_resource_index(current));
            queue[queueWrite++] = next;
        }
    }

    if (!visited[goalIndex]) {
        return B32_FALSE;
    }

    V2U32 fullPath[Bee::Bee::MAX_PATH_TILES]{};
    U32 fullCount = 0;
    for (I32 cursor = I32(goalIndex); cursor >= 0; cursor = parents[cursor]) {
        if (fullCount >= Bee::Bee::MAX_PATH_TILES) {
            return B32_FALSE;
        }
        fullPath[fullCount++] = V2U32{ U32(cursor % World::size.x), U32(cursor / World::size.x) };
    }
    if (fullCount == 0) {
        return B32_FALSE;
    }

    static V2U32 orderedPath[Bee::Bee::MAX_PATH_TILES];
    for (U32 i = 0; i < fullCount; i++) {
        orderedPath[i] = fullPath[fullCount - 1u - i];
    }

    U32 outCount = 0;
    pathTilesOut[outCount++] = orderedPath[0];
    if (fullCount > 1) {
        I32 prevDx = I32(orderedPath[1].x) - I32(orderedPath[0].x);
        I32 prevDy = I32(orderedPath[1].y) - I32(orderedPath[0].y);
        for (U32 i = 1; i + 1 < fullCount; i++) {
            I32 dx = I32(orderedPath[i + 1].x) - I32(orderedPath[i].x);
            I32 dy = I32(orderedPath[i + 1].y) - I32(orderedPath[i].y);
            if (dx != prevDx || dy != prevDy) {
                if (outCount >= maxPathTiles) {
                    return B32_FALSE;
                }
                pathTilesOut[outCount++] = orderedPath[i];
                prevDx = dx;
                prevDy = dy;
            }
        }
        if (outCount >= maxPathTiles) {
            return B32_FALSE;
        }
        pathTilesOut[outCount++] = orderedPath[fullCount - 1];
    }

    *pathCountOut = outCount;
    return B32_TRUE;
}


B32 has_adjacent_conveyor(V2U32 tile) {
    using namespace TileSpace;
    V2U32 north = neighbor_tile(tile, NeighborDirection::NORTH);
    V2U32 east = neighbor_tile(tile, NeighborDirection::EAST);
    V2U32 south = neighbor_tile(tile, NeighborDirection::SOUTH);
    V2U32 west = neighbor_tile(tile, NeighborDirection::WEST);
    return (has_conveyor(north) || has_conveyor(east) || has_conveyor(south) || has_conveyor(west)) ? B32_TRUE : B32_FALSE;
}

B32 try_insert_adjacent_belt_item(V2U32 tile, Inventory::ItemType item, U32 count = 1) {
    using namespace TileSpace;
    V2U32 neighbors[4]{
        neighbor_tile(tile, NeighborDirection::NORTH),
        neighbor_tile(tile, NeighborDirection::EAST),
        neighbor_tile(tile, NeighborDirection::SOUTH),
        neighbor_tile(tile, NeighborDirection::WEST),
    };

    for (U32 i = 0; i < 4; i++) {
        V2U32 beltTile = neighbors[i];
        Factory::Machine* belt = Factory::get_machine_from_tile(V2U{ beltTile.x, beltTile.y });
        if (!Factory::machine_is_belt(belt) || count == 0) {
            continue;
        }
        Inventory::ItemStack& stack = belt->get_item_stack();
        if (stack.count != 0 && U32(stack.item) != item) {
            continue;
        }
        U32 freeSpace = belt->inventoryStackSizeLimit > stack.count ? (belt->inventoryStackSizeLimit - stack.count) : 0;
        if (freeSpace < count) {
            continue;
        }
        if (stack.count == 0) {
            stack.item = Inventory::ItemType(item);
        }
        stack.count += count;
        return B32_TRUE;
    }
    return B32_FALSE;
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

void clear_machines_in_footprint(V2U32 topLeft, V2U32 footprint) {
    for (U32 y = 0; y < footprint.y; y++) {
        for (U32 x = 0; x < footprint.x; x++) {
            V2U32 tile{ topLeft.x + x, topLeft.y + y };
            remove_machine_tile(tile);
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
            World::TileType tileType = TerrainGen::get_world_tile(tile);
            if (tileType == World::TILE_WATER || tileType == World::TILE_MOUNTAIN) {
                return B32_FALSE;
            }
            if (Factory::has_machine(V2U{ tile.x, tile.y })) {
                return B32_FALSE;
            }
        }
    }
    return B32_TRUE;
}

void clear_tile_resource_runtime(V2U32 tile) {
    U32 index = tile_resource_index(tile);
    ironRemaining[index] = 0;
    copperRemaining[index] = 0;
    flowerRemaining[index] = 0;
}

void reset_tile_resource_runtime(V2U32 tile) {
    clear_tile_resource_runtime(tile);
    switch (TerrainGen::get_world_tile(tile)) {
    case World::TILE_GRASS_IRON:   ironRemaining[tile_resource_index(tile)] = STARTING_IRON_PER_TILE; break;
    case World::TILE_GRASS_COPPER: copperRemaining[tile_resource_index(tile)] = STARTING_COPPER_PER_TILE; break;
    case World::TILE_GRASS_FLOWERS: flowerRemaining[tile_resource_index(tile)] = STARTING_FLOWER_PER_TILE; break;
    default: break;
    }
}

void rebuild_resource_runtime() {
    memset(ironRemaining, 0, sizeof(ironRemaining));
    memset(copperRemaining, 0, sizeof(copperRemaining));
    memset(flowerRemaining, 0, sizeof(flowerRemaining));
    for (U32 y = 0; y < World::size.y; y++) {
        for (U32 x = 0; x < World::size.x; x++) {
            reset_tile_resource_runtime(V2U32{ x, y });
        }
    }
}

B32 tile_has_harvestable_resource(V2U32 tile) {
    if (!tile_in_bounds(tile)) {
        return B32_FALSE;
    }

    U32 index = tile_resource_index(tile);
    switch (TerrainGen::get_world_tile(tile)) {
    case World::TILE_BEACH:
        return World::beach_has_junk(V2U{ tile.x, tile.y });
    case World::TILE_GRASS_IRON:
        return ironRemaining[index] > 0 ? B32_TRUE : B32_FALSE;
    case World::TILE_GRASS_COPPER:
        return copperRemaining[index] > 0 ? B32_TRUE : B32_FALSE;
    case World::TILE_GRASS_FLOWERS:
        return flowerRemaining[index] > 0 ? B32_TRUE : B32_FALSE;
    default:
        return B32_FALSE;
    }
}

Inventory::ItemType item_for_resource_tile(V2U32 tile) {
    switch (TerrainGen::get_world_tile(tile)) {
    case World::TILE_GRASS_IRON:    return Inventory::ITEM_IRON_ORE;
    case World::TILE_GRASS_COPPER:  return Inventory::ITEM_COPPER_ORE;
    case World::TILE_GRASS_FLOWERS: return Inventory::ITEM_POLLEN;
    default:                        return Inventory::ITEM_IRON_ORE;
    }
}

B32 consume_resource_from_tile(V2U32 tile, Inventory::ItemType* itemOut) {
    if (!tile_in_bounds(tile)) {
        return B32_FALSE;
    }

    U32 index = tile_resource_index(tile);
    switch (TerrainGen::get_world_tile(tile)) {
    case World::TILE_BEACH: {
        return World::pop_beach_junk(V2U{ tile.x, tile.y }, itemOut);
    } break;

    case World::TILE_GRASS_IRON: {
        if (ironRemaining[index] == 0) {
            return B32_FALSE;
        }
        ironRemaining[index]--;
        if (itemOut) {
            *itemOut = Inventory::ITEM_IRON_ORE;
        }
        if (ironRemaining[index] == 0) {
            TerrainGen::set_world_tile(tile, World::TILE_GRASS);
        }
        return B32_TRUE;
    } break;

    case World::TILE_GRASS_COPPER: {
        if (copperRemaining[index] == 0) {
            return B32_FALSE;
        }
        copperRemaining[index]--;
        if (itemOut) {
            *itemOut = Inventory::ITEM_COPPER_ORE;
        }
        if (copperRemaining[index] == 0) {
            TerrainGen::set_world_tile(tile, World::TILE_GRASS);
        }
        return B32_TRUE;
    } break;

    case World::TILE_GRASS_FLOWERS: {
        if (flowerRemaining[index] == 0) {
            return B32_FALSE;
        }
        flowerRemaining[index]--;
        if (itemOut) {
            *itemOut = Inventory::ITEM_POLLEN;
        }
        if (flowerRemaining[index] == 0) {
            TerrainGen::set_world_tile(tile, World::TILE_GRASS);
        }
        return B32_TRUE;
    } break;

    default:
        return B32_FALSE;
    }
}

B32 tile_has_accessible_hive_path(V2U32 tile) {
    if (hives.size == 0) {
        return B32_FALSE;
    }

    V2U32 scratchPath[Bee::Bee::MAX_PATH_TILES]{};
    U32 scratchCount = 0;
    for (U32 i = 0; i < hives.size; i++) {
        if (!TerrainGen::tile_in_hive_radius(tile, hives[i], hive_task_radius(hives[i]))) {
            continue;
        }
        BeeSystem::HomeAnchor anchor = home_anchor_for_hive(hives[i]);
        if (find_bee_path(anchor.tile, tile, scratchPath, &scratchCount, Bee::Bee::MAX_PATH_TILES, nullptr)) {
            return B32_TRUE;
        }
    }
    return B32_FALSE;
}

B32 tile_is_selectable_task(V2U32 tile) {
    if (!tile_in_bounds(tile) || Factory::has_machine(V2U{ tile.x, tile.y }) || !tile_in_any_hive_radius(tile) || find_hive_covering_tile(tile) >= 0) {
        return B32_FALSE;
    }
    switch (TerrainGen::get_world_tile(tile)) {
    case World::TILE_BEACH:
    case World::TILE_GRASS_IRON:
    case World::TILE_GRASS_COPPER:
    case World::TILE_GRASS_FLOWERS:
        return tile_has_harvestable_resource(tile) && tile_has_accessible_hive_path(tile);
    default:
        return B32_FALSE;
    }
}

void seed_conveyors(V2U32) {
    Factory::reset();
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
    World::reset_runtime_state();
    seed_conveyors(hiveTile);
    TerrainGen::generate(&worldGeneration, &hives, hiveTile);
    Bee::set_path_finder(find_bee_path);
    Bee::set_position_collider(bee_position_hits_blocker);
    rebuild_resource_runtime();
    mainHiveHoneyProgress = 0.0F;
    colony.init(BEE_COUNT, hiveTile, SPEED);
    colony.set_home_selector(select_nearest_hive_anchor);
    if (hives.size != 0) {
        BeeSystem::HomeAnchor mainAnchor = home_anchor_for_hive(hives[0]);
        colony.set_default_home(mainAnchor.tile, mainAnchor.offsetWorld);
        for (U32 i = 0; i < colony.bees.size; i++) {
            colony.bees[i].set_home_anchor(mainAnchor.tile, mainAnchor.offsetWorld);
            colony.bees[i].snap_to_home();
        }
    }
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

void deposit_bee_cargo_to_inventory(Bee::Bee& bee) {
    if (!bee.carrying()) {
        return;
    }
    Inventory::add_item(Inventory::ItemType(bee.carriedItem), bee.carriedCount);
    bee.clear_cargo();
}

void update_main_hive_honey_conversion(F32 dt) {
    if (dt <= 0.0F) {
        return;
    }

    U32 beesInHives = bees_inside_any_hive();
    if (beesInHives == 0 || Inventory::count(Inventory::ITEM_POLLEN) < POLLEN_PER_HONEY) {
        return;
    }

    // Inventory is currently global, so conversion has to be global too.
    // Use all bees that are actually inside a hive so satellite hives contribute instead of stalling conversion.
    mainHiveHoneyProgress += dt * F32(beesInHives);
    while (mainHiveHoneyProgress >= HONEY_CONVERSION_SECONDS && Inventory::count(Inventory::ITEM_POLLEN) >= POLLEN_PER_HONEY) {
        mainHiveHoneyProgress -= HONEY_CONVERSION_SECONDS;
        Inventory::try_take_item(Inventory::ITEM_POLLEN, POLLEN_PER_HONEY);
        Inventory::add_item(Inventory::ITEM_HONEY, 1);
    }
}

void handle_work_cycle_finished(const BeeSystem::Event& event) {
    if (event.beeIndex >= colony.bees.size) {
        return;
    }

    Bee::Bee& bee = colony.bees[event.beeIndex];
    Inventory::ItemType item{};
    if (!consume_resource_from_tile(event.task.targetTile, &item)) {
        colony.unqueue_task_for_tile(event.task.targetTile);
        bee.clear_cargo();
        return;
    }

    B32 resourceRemaining = tile_has_harvestable_resource(event.task.targetTile);
    BeeSystem::HomeAnchor depositHome = nearest_hive_anchor_for_tile(event.task.targetTile);

    if ((event.task.type == BeeTasks::TaskType::TASK_ORE || event.task.type == BeeTasks::TaskType::TASK_FLOWER) && bee.activeTask.returnHomeAfterWork == B32_FALSE) {
        if (try_insert_adjacent_belt_item(event.task.targetTile, item, 1)) {
            bee.clear_cargo();
            if (!resourceRemaining) {
                colony.unqueue_task_for_tile(event.task.targetTile);
            }
            return;
        }

        bee.set_home_anchor(depositHome.tile, depositHome.offsetWorld);
        bee.set_cargo(item, 1);
        bee.state = BeeTasks::State::STATE_TRAVEL_HOME;
        bee.velocity = V2F32{};
    }
    else {
        bee.set_home_anchor(depositHome.tile, depositHome.offsetWorld);
        bee.set_cargo(item, 1);
    }

    if (!resourceRemaining) {
        colony.unqueue_task_for_tile(event.task.targetTile);
    }
}

void handle_bee_reached_home(const BeeSystem::Event& event) {
    if (event.beeIndex >= colony.bees.size) {
        return;
    }
    deposit_bee_cargo_to_inventory(colony.bees[event.beeIndex]);
}

void update(F32 dt) {
    colony.update(dt);

    for (U32 i = 0; i < colony.events.size; i++) {
        const BeeSystem::Event& event = colony.events[i];
        switch (event.type) {
        case BeeSystem::EventType::EVENT_WORK_CYCLE_FINISHED:
            handle_work_cycle_finished(event);
            break;
        case BeeSystem::EventType::EVENT_BEE_REACHED_HOME:
            handle_bee_reached_home(event);
            break;
        default:
            break;
        }
    }

    update_main_hive_honey_conversion(dt);
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
    case CreativeBrush::MOUNTAIN: return &Resources::tile.mountain;
    case CreativeBrush::CONVEYOR: return &Resources::tile.belt.leftToRight;
    case CreativeBrush::ASSEMBLER_SMALL: return &Resources::tile.assemblerSmall;
    case CreativeBrush::ASSEMBLER_LARGE: return &Resources::tile.assemblerLarge;
    case CreativeBrush::SPLITTER: return &Resources::tile.splitter;
    case CreativeBrush::MERGER: return &Resources::tile.merger;
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
    if (cellX == 11 && cellY == 13) return CreativeBrush::MOUNTAIN;
    if (cellX == 4 && cellY == 0) return CreativeBrush::CONVEYOR;
    if (cellX == 1 && cellY == 3) return CreativeBrush::ASSEMBLER_SMALL;
    if ((cellX == 0 || cellX == 1) && (cellY == 6 || cellY == 7)) return CreativeBrush::ASSEMBLER_LARGE;
    if (cellX == 2 && cellY == 3) return CreativeBrush::SPLITTER;
    if (cellX == 2 && cellY == 4) return CreativeBrush::MERGER;
    if (cellX == 1 && cellY == 2) return CreativeBrush::HIVE_SMALL;
    if ((cellX == 0 || cellX == 1) && (cellY == 4 || cellY == 5)) return CreativeBrush::HIVE_BIG;
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
    clear_machines_in_footprint(topLeft, footprint);
    clear_hives_in_footprint(topLeft, footprint);
    hives.push_back(newHive);
}

void place_structure(V2U32 topLeft, Factory::MachineType type, Rotation2 orientation) {
    V2U32 footprint = Factory::machine_footprint(type, orientation);
    if (topLeft.x + footprint.x > World::size.x || topLeft.y + footprint.y > World::size.y) {
        return;
    }
    for (U32 y = 0; y < footprint.y; y++) {
        for (U32 x = 0; x < footprint.x; x++) {
            V2U32 tile{ topLeft.x + x, topLeft.y + y };
            World::TileType tileType = TerrainGen::get_world_tile(tile);
            if (tileType == World::TILE_WATER || tileType == World::TILE_MOUNTAIN) {
                return;
            }
        }
    }
    clear_tasks_in_footprint(topLeft, footprint);
    clear_hives_in_footprint(topLeft, footprint);
    clear_machines_in_footprint(topLeft, footprint);
    Factory::place_machine_type(V2U{ topLeft.x, topLeft.y }, type, orientation);
}

void apply_creative_brush(CreativeBrush brush, V2U32 tile, Rotation2 orientation) {
    if (!tile_in_bounds(tile)) {
        return;
    }

    switch (brush) {
    case CreativeBrush::TASK_SELECT: {
        queue_tile_task(tile);
    } break;

    case CreativeBrush::ERASE: {
        unqueue_tile_task(tile);
        remove_machine_tile(tile);
        remove_hive_covering_tile(tile);
        TerrainGen::set_world_tile(tile, World::TILE_GRASS);
        sync_beach_runtime_tile(tile);
        clear_tile_resource_runtime(tile);
    } break;

    case CreativeBrush::GRASS:
    case CreativeBrush::IRON:
    case CreativeBrush::COPPER:
    case CreativeBrush::FLOWERS:
    case CreativeBrush::SAND:
    case CreativeBrush::BEACH:
    case CreativeBrush::WATER:
    case CreativeBrush::MOUNTAIN: {
        unqueue_tile_task(tile);
        remove_machine_tile(tile);
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
        case CreativeBrush::MOUNTAIN: worldType = World::TILE_MOUNTAIN; break;
        default: break;
        }
        TerrainGen::set_world_tile(tile, worldType);
        sync_beach_runtime_tile(tile);
        reset_tile_resource_runtime(tile);
    } break;

    case CreativeBrush::CONVEYOR: {
        unqueue_tile_task(tile);
        remove_hive_covering_tile(tile);
        if (TerrainGen::get_world_tile(tile) != World::TILE_WATER && TerrainGen::get_world_tile(tile) != World::TILE_MOUNTAIN) {
            add_conveyor_tile(tile);
        }
    } break;

    case CreativeBrush::ASSEMBLER_SMALL: {
        place_structure(tile, Factory::MACHINE_ASSEMBLER, orientation);
    } break;

    case CreativeBrush::ASSEMBLER_LARGE: {
        place_structure(tile, Factory::MACHINE_ASSEMBLER, orientation);
    } break;

    case CreativeBrush::SPLITTER: {
        place_structure(tile, Factory::MACHINE_SPLITTER, orientation);
    } break;

    case CreativeBrush::MERGER: {
        place_structure(tile, Factory::MACHINE_MERGER, orientation);
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

void render_main_hive_status(V2F32 camera, I32 worldTileScale) {
    const HiveDesc* hive = main_hive();
    if (!hive) {
        return;
    }

    V2U32 footprint = TerrainGen::hive_footprint_size_tiles(*hive);
    I32 panelWidth = max(32 * worldTileScale, 120);
    I32 rowHeight = max(8 * worldTileScale, 20);
    I32 panelHeight = rowHeight * 3 + 38;
    V2F32 screenTopLeft = world_to_screen(TileSpace::tile_to_world(hive->tile), camera, worldTileScale);
    I32 hivePixelWidth = I32(footprint.x) * world_tile_pixels(worldTileScale);
    I32 panelX = I32(roundf32(screenTopLeft.x)) + hivePixelWidth / 2 - panelWidth / 2;
    I32 panelY = I32(roundf32(screenTopLeft.y)) - panelHeight - 8;

    fill_rect_blended(panelX, panelY, panelWidth, panelHeight, RGBA8{ 18, 18, 18, 210 });
    fill_rect(panelX, panelY, panelWidth, 1, RGBA8{ 210, 180, 70, 255 });
    fill_rect(panelX, panelY + panelHeight - 1, panelWidth, 1, RGBA8{ 210, 180, 70, 255 });
    fill_rect(panelX, panelY, 1, panelHeight, RGBA8{ 210, 180, 70, 255 });
    fill_rect(panelX + panelWidth - 1, panelY, 1, panelHeight, RGBA8{ 210, 180, 70, 255 });

    I32 iconScale = max((worldTileScale * 3) / 4, 1);
    I32 iconSize = 16 * iconScale;
    I32 iconX = panelX + 4;
    I32 valueX = iconX + iconSize + 4;

    I32 pollenY = panelY + 6;
    I32 honeyY = pollenY + rowHeight;
    I32 beeY = honeyY + rowHeight;

    fill_rect_blended(panelX + 2, pollenY - 1, panelWidth - 4, rowHeight, RGBA8{ 90, 160, 90, 70 });
    fill_rect_blended(panelX + 2, honeyY - 1, panelWidth - 4, rowHeight, RGBA8{ 190, 150, 60, 70 });
    fill_rect_blended(panelX + 2, beeY - 1, panelWidth - 4, rowHeight, RGBA8{ 90, 120, 190, 70 });

    Graphics::blit_sprite_cutout(*Inventory::itemSprite[Inventory::ITEM_POLLEN], iconX, pollenY, iconScale, 0);
    Graphics::display_num(Inventory::count(Inventory::ITEM_POLLEN), valueX, pollenY, 16);

    Graphics::blit_sprite_cutout(*Inventory::itemSprite[Inventory::ITEM_HONEY], iconX, honeyY, iconScale, 0);
    Graphics::display_num(Inventory::count(Inventory::ITEM_HONEY), valueX, honeyY, 16);

    Graphics::blit_sprite_cutout(Resources::tile.beeFly, iconX, beeY, iconScale, 0);
    Graphics::display_num(bees_inside_any_hive(), valueX, beeY, 16);

    I32 barOuterX = panelX + 6;
    I32 barOuterY = panelY + panelHeight - 22;
    I32 barOuterW = panelWidth - 12;
    I32 barOuterH = max(16, worldTileScale * 4);
    I32 barX = barOuterX + 1;
    I32 barY = barOuterY + 1;
    I32 barW = barOuterW - 2;
    I32 barH = barOuterH - 2;
    F32 progress = HONEY_CONVERSION_SECONDS > 0.0F ? clamp01(mainHiveHoneyProgress / HONEY_CONVERSION_SECONDS) : 0.0F;
    fill_rect(barOuterX, barOuterY, barOuterW, barOuterH, RGBA8{ 15, 15, 15, 255 });
    fill_rect(barX, barY, barW, barH, RGBA8{ 55, 45, 30, 255 });
    fill_rect(barX, barY, I32(roundf32(F32(barW) * progress)), barH, RGBA8{ 255, 200, 70, 255 });
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
    render_main_hive_status(camera, worldTileScale);
}

void render_conveyors(V2F32, I32, F64) {
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

    Resources::Sprite* beeSprite = &Resources::tile.beeFly;
    if (bee.carrying() && bee.state == BeeTasks::State::STATE_TRAVEL_HOME) {
        beeSprite = &Resources::tile.beeCarry;
    }
    else if (bee.state == BeeTasks::State::STATE_WORKING) {
        beeSprite = &Resources::tile.beeMine;
    }

    F32 animTurns = fractf64(frameTimeSeconds * 6.0 + bee.flightPhaseTurns);
    U32 animFrame = U32(animTurns * F32(beeSprite->animFrames)) % beeSprite->animFrames;
    V2F32 beeScreenCenter = world_to_screen(bee.position, camera, worldTileScale);
    V2F32 beeScreenTopLeft = beeScreenCenter - V2F32{ F32(beeSprite->width * worldTileScale) * 0.5F, F32(beeSprite->height * worldTileScale) * 0.5F };
    Graphics::blit_sprite_cutout(*beeSprite, I32(roundf32(beeScreenTopLeft.x)), I32(roundf32(beeScreenTopLeft.y)), worldTileScale, animFrame);


    render_bee_progress_bar(bee, camera, worldTileScale);
}

void render_bees(V2F32 camera, I32 worldTileScale, F64 frameTimeSeconds) {
    for (U32 beeIndex = 0; beeIndex < colony.bees.size; beeIndex++) {
        render_bee(colony.bees[beeIndex], camera, worldTileScale, frameTimeSeconds);
    }
}

}
