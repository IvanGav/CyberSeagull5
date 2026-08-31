#pragma once

#include "drillengine/DrillLib.h"
#include "TileSpace.h"
#include "Sounds.h"

namespace Ant {

	using namespace TileSpace;

	static constexpr F32 DEFAULT_SPEED = 2.0F;
	static constexpr F32 WANDER_RADIUS = 4.0F;
	static constexpr F32 TARGET_EPSILON = 0.15F;

	FINLINE F32 random01(U32 value) {
		U32 h = hash32(value);
		return F32(h & 0xFFFFu) * (1.0F / 65535.0F);
	}

	enum AntType : U8 {
		ANT_WORKER = 0,
		ANT_SOLDIER,
		ANT_QUEEN,
		ANT_COUNT
	};

	// Ant Data Structure
	struct Ant {	
		static constexpr F32 walkWobbleFrequency = 0.24F;
		static constexpr F32 walkWobbleAmplitude = 0.10F;

		V2F32 position{};
		V2F32 velocity{};
		V2F32 targetPosition{};


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
			moveSpeed = speed;
			walkPhaseTurns = 0.0F;

			position = TileSpace::tile_to_world_center(spawnTile);
			targetPosition = position;
			walkPhaseTurns = random01(seed + 1u);
			targetTimer = 0.0F;
			health = type == ANT_SOLDIER ? 20.0F : 10.0F;
			attackCooldown = 0.0F;
		}

		void choose_random_target() {
			F32 angle = random01(randomSeed + 2u) * 2.0f * MATH_PI;
			F32 distance = random01(randomSeed++) * WANDER_RADIUS;

			targetPosition =
				TileSpace::tile_to_world_center(homeTile) + 
				V2F32{ cosf32(angle), sinf32(angle) } * distance;
		}

		void update(F32 dt, V2U32 worldSize) {
			dt = max(dt, 0.0F);
			targetTimer -= dt;
			attackCooldown = max(attackCooldown - dt, 0.0F);

			if (targetTimer <= 0.0F || distance_sq(position, targetPosition) <= TARGET_EPSILON * TARGET_EPSILON) {
				choose_random_target();
				targetTimer = 2.0F + random01(randomSeed++) * 3.0F; // choose a new target every 2-5 seconds
			}
			else {
				velocity = V2F32{};
			}

			// Keep ants inside map, 0.1 clamp
			position.x = clamp(position.x, 0.1F, F32(worldSize.x) - 0.1F);
			position.y = clamp(position.y, 0.1F, F32(worldSize.y) - 0.1F);
		}

		B32 is_alive() const {
			return health > 0.0F ? B32_TRUE : B32_FALSE;
		}

	};
}
