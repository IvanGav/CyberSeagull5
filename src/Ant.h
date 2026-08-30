#pragma once

#include "drillengine/DrillLib.h"
#include "TileSpace.h"
#include "Sounds.h"

namespace Ant {

	using namespace TileSpace;

	enum AntType : U8 {
		ANT_WORKER = 0,
		ANT_SOLDIER,
		ANT_QUEEN,
		ANT_COUNT
	};


	static constexpr F32 DEFAULT_SPEED = 6.0F;
	

	struct Ant {	
		static constexpr F32 walkWobbleFrequency = 0.24F;
		static constexpr F32 walkWobbleAmplitude = 0.10F;

		V2F32 position{};
		V2F32 velocity{};
		V2U32 homeTile{};

		F32 moveSpeed = DEFAULT_SPEED;
		F32 walkPhaseTurns = 0.0F;

		//spawns the ant at a given tile position
		void spawn(V2U32 spawnTile, F32 speed = DEFAULT_SPEED) {
			homeTile = spawnTile;
			position = TileSpace::tile_to_world_center(spawnTile);
			moveSpeed = speed;
			walkPhaseTurns = 0.0F;
		}

		void update(F32 dt) {
			// Simple random walk for demonstration purposes
			F32 angle = Bee::hash01(U32(position.x * 1000.0f + position.y * 1000.0f)) * 2.0f * MATH_PI;
			velocity = V2F32{ cosf32(angle), sinf32(angle) } * moveSpeed;
			position += velocity * dt;
			// Keep the ant within the world bounds
			position.x = clamp(position.x, 0.0f, F32(World::size.x));
			position.y = clamp(position.y, 0.0f, F32(World::size.y));
		}

	};
}
