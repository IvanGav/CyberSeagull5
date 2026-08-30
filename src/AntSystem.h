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

	struct AntColonies {
		
		void init(U32 antCount, V2U32 colonyTile, F32 antMoveSpeed = Ant::DEFAULT_SPEED, MemoryArena* arena = &globalArena) {
			allocator = arena ? arena : &globalArena;
			ants.allocator = allocator;
			ants.clear();
			home.tile = colonyTile;
			antSpeed = antMoveSpeed;
		}

		void add_ant(const Ant::Ant& ant) {
			ants.push_back(ant);
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

	ArenaArrayList<AntColonies> colonies{};
	MemoryArena* colonyArena = &globalArena;
	I32 maxStartingColonies = 3;
	I32 minStartingColonies = 1;


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
		for (U32 i = 0; i < colonies.size; i++) {
			for (U32 i = 0; i < colonies[i].ants.size; i++) {
				render_ant(colonies[i].ants[i], camera, worldTileScale, frameTimeSeconds);
			}
		}
	}

}
