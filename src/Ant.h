#pragma once

#include "drillengine/DrillLib.h"
#include "TileSpace.h"
#include "Sounds.h"

namespace Ant {

	using namespace TileSpace;

	// userData is a pointer to any data that the function may need to access; just gives it flexibility
	using PathFindFn = B32(*)(V2U32 startTile, V2U32 goalTile, V2U32* pathTilesOut, U32* pathCountOut, U32 maxPathTiles, void* userData);
	using PositionBlockedFn = B32(*)(V2F32 position, V2U32 startTile, V2U32 goalTile, void* userData);	

	static constexpr F32 DEFAULT_SPEED = 0.5F;
	static constexpr F32 WANDER_RADIUS = 4.0F;
	static constexpr F32 TARGET_EPSILON = 0.15F;
	static constexpr U32 MAX_PATH_TILES = 1024u;

	
	// Allows for customization of pathfinding and position blocking logic
	// Also so I dont have to include the entire pathfinding system in this header
	PathFindFn gPathFindFn = nullptr;
	void* gPathFindUserData = nullptr;

	PositionBlockedFn gPositionBlockedFn = nullptr;
	void* gPositionBlockedUserData = nullptr;
	


	enum AntType : U8 {
		ANT_WORKER = 0,
		ANT_SOLDIER,
		ANT_QUEEN,
		ANT_COUNT
	};

	FINLINE F32 random01(U32 value) {
		U32 h = hash32(value);
		return F32(h & 0xFFFFu) * (1.0F / 65535.0F);
	}

	void set_path_finder(PathFindFn fn, void* userData = nullptr) {
		gPathFindFn = fn;
		gPathFindUserData = userData;
	}

	void set_position_collider(PositionBlockedFn fn, void* userData = nullptr) {
		gPositionBlockedFn = fn;
		gPositionBlockedUserData = userData;
	}

	// Ant Data Structure
	struct Ant {	
		static constexpr F32 walkWobbleFrequency = 0.24F;
		static constexpr F32 walkWobbleAmplitude = 0.10F;

		V2F32 position{};
		V2F32 velocity{};
		V2F32 targetPosition{ 0,0 };

		V2U32 pathTiles[MAX_PATH_TILES]{};
		U32 pathTileCount = 0;
		U32 pathTileIndex = 0;
		V2U32 pathGoalTile{};


		V2U32 homeTile{};
		AntType type = ANT_WORKER;

		F32 moveSpeed = DEFAULT_SPEED;
		F32 walkPhaseTurns = 0.0F;
		F32 targetTimer = 0.0F;
		F32 health = 10.0F;
		F32 attackCooldown = 0.0F;

		U32 randomSeed = 0;

		//spawns the ant at a given tile position
		void spawn(V2U32 spawnTile, F32 speed = DEFAULT_SPEED, AntType antType = ANT_WORKER, U32 seed = 0) {
			homeTile = spawnTile;
			type = antType;
			targetPosition = TileSpace::tile_to_world_center(spawnTile);
			moveSpeed = speed;
			walkPhaseTurns = 0.0F;

			position = TileSpace::tile_to_world_center(spawnTile);
			walkPhaseTurns = random01(seed + 1u);
			targetTimer = 0.0F;
			health = type == ANT_SOLDIER ? 20.0F : 10.0F;
			attackCooldown = 0.0F;
		}


		void move_towards_target(V2F32 targetTile, F32 dt) {
			if (distance_sq(position, targetTile) <= TARGET_EPSILON * TARGET_EPSILON) {
				velocity = V2F32{};
				return;
			}
			velocity = moveSpeed * Bee::normalize_v2_safe(targetTile - position);
			
			V2F32 step = velocity * dt;

			position = clamp(position + step, V2F32{ 0.1F, 0.1F }, V2F32{ F32(World::size.x) - 0.1F, F32(World::size.y) - 0.1F });
		}

		void move_towards_target(F32 dt) {
			move_towards_target(targetPosition, dt);
		}


		B32 has_valid_path(V2F32 target) {
			if (target.x && target.y) {
				return B32_FALSE;
			}

			if (!gPathFindFn) {
				return B32_FALSE;
			}

			V2U32 startTile = TileSpace::world_to_tile(position);
			V2U32 goalTile = TileSpace::world_to_tile(target);
			B32 pathFound = gPathFindFn(startTile, goalTile, pathTiles, &pathTileCount, MAX_PATH_TILES, gPathFindUserData);

			return pathFound;
		}

		V2F32 get_random_idle_target() {
			V2F32 idle_target{};


			while (has_valid_path(idle_target)) {
				F32 angle = random01(randomSeed + 2u) * 2.0f * MATH_PI;
				F32 distance = random01(randomSeed++) * WANDER_RADIUS;
				idle_target = TileSpace::tile_to_world_center(homeTile) +
					V2F32{ cosf32(angle), sinf32(angle) } * distance;
			}
	
			return idle_target;
		}

		void idle_movement(F32 dt) {

			if (distance_sq(position, targetPosition) <= TARGET_EPSILON * TARGET_EPSILON) {
				targetTimer -= dt;
			}

			if (targetTimer <= 0.0F) {
				targetPosition = get_random_idle_target();
				targetTimer = 2.0F + random01(randomSeed++) * 3.0F; // choose a new target every 2-5 seconds

				return;
			}


		    move_towards_target(dt);

		}

		void update(F32 dt, V2U32 worldSize) {
			dt = max(dt, 0.0F);	

			attackCooldown = max(attackCooldown - dt, 0.0F);

			idle_movement(dt);
		
		}

		B32 is_alive() const {
			return health > 0.0F ? B32_TRUE : B32_FALSE;
		}

	};
}
