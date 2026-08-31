#pragma once

#include "Ant.h"
#include "Sounds.h"
#include "Graphics.h"
#include "Win32.h"
#include "Resources.h"
#include "World.h"
#include "TileSpace.h"
#include "TerrainGen.h"
#include "Factory.h"

namespace AntSystem {

	static constexpr U32 MAX_ANT_COLONIES = 3;
	static constexpr U32 ANTS_PER_COLONY_MIN = 1;
	static constexpr U32 ANTS_PER_COLONY_MAX = 4;
	static constexpr U32 MIN_COLONY_DISTANCE_FROM_MAIN_HIVE = 16;
	static constexpr U32 MIN_COLONY_DISTANCE_FROM_EACHOTHER = 8;


	struct AntColonies {

		void add_ant(const Ant::Ant& ant) {
			ants.push_back(ant);
		}

		void init(U32 antCount, V2U32 colonyTile, F32 antMoveSpeed = Ant::DEFAULT_SPEED, MemoryArena* arena = &globalArena) {
			allocator = arena ? arena : &globalArena;
			ants.allocator = allocator;
			ants.clear();
			home.tile = colonyTile;
			antSpeed = antMoveSpeed;
		}

		struct HomeAnchor {
			V2U32 tile{};
			V2F32 offsetWorld{ 0.5F, 0.5F };
		};

		MemoryArena* allocator = &globalArena;
		ArenaArrayList <Ant::Ant> ants{};
		HomeAnchor home{};
		F32 antSpeed = Ant::DEFAULT_SPEED;
		I32 maxStartingAnts = 6;
		I32 minStartingAnts = 1;

	};

	U32 random_seed = 0xA17C0DEu;
	V2U32 mainHiveTile{};

	ArenaArrayList<AntColonies> colonies{};
	MemoryArena* colonyArena = &globalArena;
	I32 maxStartingColonies = 3;
	I32 minStartingColonies = 1;
	U32 colonyCount = 0;


	FINLINE U32 next_random() {
		random_seed ^= random_seed << 13;
		random_seed ^= random_seed >> 17;
		random_seed ^= random_seed << 5;
		return random_seed;
	}

	FINLINE U32 random_range(U32 min, U32 max) {
		if (max <= min) {
			return min;
		}

		U32 range = max - min + 1;
		U32 randVal = next_random();
		return min + (randVal % range);
	}

	B32 valid_colony_tile(V2U32 tile) {
		if (tile.x < 0 || tile.y < 0 || tile.x >= World::size.x || tile.y >= World::size.y) {
			return B32_FALSE;
		}
		if (World::get_tile(nullptr, tile.x, tile.y, 0) != World::TILE_GRASS) {
			return B32_FALSE;
		}
		F32 distanceToMainHive = distance(TileSpace::tile_to_world_center(tile), TileSpace::tile_to_world_center(mainHiveTile));
		if (distanceToMainHive < F32(MIN_COLONY_DISTANCE_FROM_MAIN_HIVE)) {
			return B32_FALSE;
		}
		return B32_TRUE;
	}

	// check to see if colony is far enough from a different colony
	B32 colony_dist_others_good(V2U32 tile) {
		for (U32 i = 0; i < colonies.size; i++) {
			V2U32 otherTile = colonies[i].home.tile;

			I32 dx = I32(tile.x) - I32(otherTile.x);
			I32 dy = I32(tile.y) - I32(otherTile.y);

			if (dx * dx + dy * dy < MIN_COLONY_DISTANCE_FROM_EACHOTHER) {
				return B32_FALSE;
			}
		}

		return B32_TRUE;
	}



	void init(V2U32 playerHiveTile) {
		colonies.allocator = colonyArena;
		colonies.clear();

		mainHiveTile = playerHiveTile;
		random_seed = 0xA17C0DEu;

		colonyCount = random_range(
			U32(minStartingColonies),
			U32(maxStartingColonies)
		);

		for (U32 colonyIndex = 0; colonyIndex < colonyCount; colonyIndex++) {
			V2U32 colonyTile{};
			B32 foundTile = B32_FALSE;

			for (U32 attempt = 0; attempt < 100 && !foundTile; attempt++) {
				colonyTile = V2U32{
					random_range(1, World::size.x - 2),
					random_range(1, World::size.y - 2)
				};
				
				if (valid_colony_tile(colonyTile) && colony_dist_others_good(colonyTile)) {
					foundTile = B32_TRUE;
				}
			}

			if (!foundTile) {
				continue;
			}

			AntColonies& colony = colonies.push_back_zeroed();
			colony.init(0, colonyTile);

			U32 antCount = random_range(
				ANTS_PER_COLONY_MIN,
				ANTS_PER_COLONY_MAX
			);

			for (U32 antIndex = 0; antIndex < antCount; antIndex++) {
				V2U32 spawnTile = colonyTile;

				I32 offsetX = I32(random_range(0, 4)) - 2;
				I32 offsetY = I32(random_range(0, 4)) - 2;

				I32 antX = clamp(
					I32(colonyTile.x + offsetX),
					0,
					I32(World::size.x) - 1
				);

				I32 antY = clamp(
					I32(colonyTile.y + offsetY),
					0,
					I32(World::size.y) - 1
				);

				spawnTile = V2U32{ U32(antX), U32(antY) };

				Ant::Ant ant{};
				ant.spawn(
					spawnTile,
					Ant::DEFAULT_SPEED + F32(random_range(0, 20)) * 0.1F,
					antIndex == 0 ? Ant::ANT_SOLDIER : Ant::ANT_WORKER,
					next_random()
				);

				colony.add_ant(ant);
			}
		}

		colonyCount = colonies.size;
	}

	void update(F32 dt) {
		for (U32 colonyIndex = 0; colonyIndex < colonies.size; colonyIndex++) {
			AntColonies& colony = colonies[colonyIndex];

			for (U32 antIndex = 0; antIndex < colony.ants.size; antIndex++) {
				colony.ants[antIndex].update(dt, World::size);
			}
		}
	}


	void render_hills(V2F camera, I32 worldTileScale) {
		for (U32 colonyIndex = 0; colonyIndex < colonies.size; colonyIndex++) {
			const AntColonies& colony = colonies[colonyIndex];

			V2F32 hillPosition = TileSpace::tile_to_world_center(colony.home.tile);
			V2F32 screenPosition = World::world_to_screen(hillPosition, camera, worldTileScale);

			Resources::Sprite& hillSprite = Resources::tile.hive;

			Graphics::blit_sprite_cutout(
				hillSprite,
				I32(screenPosition.x - F32(hillSprite.width * worldTileScale) * 0.5F),
				I32(screenPosition.y - F32(hillSprite.height * worldTileScale) * 0.5F),
				worldTileScale,
				0
			);
		}
	}

	void render_ant(const Ant::Ant& ant, V2F32 camera, I32 WorldTileScale, F64 frameTimeSeconds) {
		V2F32 antPos = ant.position;
		V2F32 screenPos = (antPos - camera) * WorldTileScale;

		Resources::Sprite* antSprite = &Resources::tile.antWalk;
		F32 animTurns = fractf64(frameTimeSeconds * 6.0 + ant.walkPhaseTurns);
		U32 animFrame = U32(animTurns * F32(antSprite->animFrames)) % antSprite->animFrames;
		V2F32 antScreenCenter = World::world_to_screen(antPos, camera, WorldTileScale);
		V2F32 antScreenTopLeft = antScreenCenter - V2F32{ F32(antSprite->width) * 0.5F, F32(antSprite->height) * 0.5F } * WorldTileScale;
		Graphics::blit_sprite_cutout(*antSprite, I32(roundf32(antScreenTopLeft.x)), I32(roundf32(antScreenTopLeft.y)), WorldTileScale, animFrame);
	}

	void render_ants(V2F32 camera, I32 worldTileScale, F64 frameTimeSeconds) {
		for (U32 colonyIndex = 0; colonyIndex < colonies.size; colonyIndex++) {
			for (U32 antIndex = 0; antIndex < colonies[colonyIndex].ants.size; antIndex++) {
				render_ant(colonies[colonyIndex].ants[antIndex], camera, worldTileScale, frameTimeSeconds);
			}
		}
	}

}
