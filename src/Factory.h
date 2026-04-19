#pragma once

#include "drillengine/DrillLib.h"
#include "Resources.h"
#include "Graphics.h"
#include "World.h"

namespace Cyber5eagull {
V2I tile_to_screen_px(V2U tile);
}

namespace Factory {

enum Item : U32 {
	ITEM_NONE,
	ITEM_IRON,
	ITEM_Count
};

struct ItemStack {
	Item item = ITEM_NONE;
	U32 count = 0;
};

enum MachineType : U32 {
	MACHINE_NONE,
	MACHINE_BELT,
	MACHINE_Count
};

struct Machine {
	MachineType type = MACHINE_NONE;
	U64 generation = 0;
	Resources::Sprite* sprite = nullptr;
	V2U pos{};
	V2U size{ 1, 1 };
	U32 id = 0;
	U32 animFrame = 0;
	ItemStack inventory{};
	U32 inventoryStackSizeLimit = 1;
	Machine* output = nullptr;
	Direction2 inputSide = DIRECTION2_LEFT;
	Direction2 outputSide = DIRECTION2_RIGHT;
};

struct MachineHandle {
	Machine* machine = nullptr;
	U64 generation = 0;

	FINLINE Machine* get() const {
		return (!machine || machine->generation == 0 || machine->generation != generation) ? nullptr : machine;
	}
};

PoolAllocator<Machine> machineAllocator{};
ArenaArrayList<U32> freeMachineIds{};
ArenaArrayList<Machine*> machineIdToMachine{};
ArenaArrayList<Machine*> machineTiles{};
U64 nextGeneration = 1;
U32 tickCount = 0;

struct MachineDef {
	MachineType type = MACHINE_NONE;
	V2U size{ 1, 1 };
	Resources::Sprite* sprite = nullptr;
	V2I relativeInputs[4]{};
	V2I relativeOutputs[4]{};
	U32 inputCount = 0;
	U32 outputCount = 0;
	Direction2 inputSide = DIRECTION2_LEFT;
	Direction2 outputSide = DIRECTION2_RIGHT;
};

FINLINE V2I direction_offset(Direction2 direction) {
	switch (direction) {
	case DIRECTION2_LEFT: return V2I{ -1, 0 };
	case DIRECTION2_RIGHT: return V2I{ 1, 0 };
	case DIRECTION2_FRONT: return V2I{ 0, -1 };
	case DIRECTION2_BACK: return V2I{ 0, 1 };
	default: return V2I{ 0, 0 };
	}
}

FINLINE Direction2 opposite_direction(Direction2 direction) {
	switch (direction) {
	case DIRECTION2_LEFT: return DIRECTION2_RIGHT;
	case DIRECTION2_RIGHT: return DIRECTION2_LEFT;
	case DIRECTION2_FRONT: return DIRECTION2_BACK;
	case DIRECTION2_BACK: return DIRECTION2_FRONT;
	default: return DIRECTION2_INVALID;
	}
}

FINLINE B32 tile_in_bounds(V2U pos) {
	return pos.x < World::size.x && pos.y < World::size.y ? B32_TRUE : B32_FALSE;
}

FINLINE B32 machine_is_belt(const Machine* machine) {
	return machine && machine->generation != 0 && machine->type == MACHINE_BELT ? B32_TRUE : B32_FALSE;
}

MachineDef get_belt(Direction2 src, Direction2 dst) {
	DEBUG_ASSERT(src != DIRECTION2_INVALID && dst != DIRECTION2_INVALID, "Belt direction cannot be invalid"a);
	DEBUG_ASSERT(src != dst, "Belt cannot do a U turn"a);
	MachineDef result{};
	result.type = MACHINE_BELT;
	result.size = V2U{ 1, 1 };
	result.inputCount = 1;
	result.outputCount = 1;
	result.relativeInputs[0] = direction_offset(src);
	result.relativeOutputs[0] = direction_offset(dst);
	result.inputSide = src;
	result.outputSide = dst;
	switch (src) {
	case DIRECTION2_LEFT: {
		switch (dst) {
		case DIRECTION2_RIGHT: result.sprite = &Resources::tile.belt.leftToRight; break;
		case DIRECTION2_FRONT: result.sprite = &Resources::tile.belt.leftToUp; break;
		case DIRECTION2_BACK: result.sprite = &Resources::tile.belt.leftToDown; break;
		default: break;
		}
	} break;
	case DIRECTION2_RIGHT: {
		switch (dst) {
		case DIRECTION2_LEFT: result.sprite = &Resources::tile.belt.rightToLeft; break;
		case DIRECTION2_FRONT: result.sprite = &Resources::tile.belt.rightToUp; break;
		case DIRECTION2_BACK: result.sprite = &Resources::tile.belt.rightToDown; break;
		default: break;
		}
	} break;
	case DIRECTION2_FRONT: {
		switch (dst) {
		case DIRECTION2_RIGHT: result.sprite = &Resources::tile.belt.upToRight; break;
		case DIRECTION2_LEFT: result.sprite = &Resources::tile.belt.upToLeft; break;
		case DIRECTION2_BACK: result.sprite = &Resources::tile.belt.upToDown; break;
		default: break;
		}
	} break;
	case DIRECTION2_BACK: {
		switch (dst) {
		case DIRECTION2_RIGHT: result.sprite = &Resources::tile.belt.downToRight; break;
		case DIRECTION2_FRONT: result.sprite = &Resources::tile.belt.downToUp; break;
		case DIRECTION2_LEFT: result.sprite = &Resources::tile.belt.downToLeft; break;
		default: break;
		}
	} break;
	default: break;
	}
	return result;
}

void apply_machine_def(Machine* machine, const MachineDef& def) {
	machine->type = def.type;
	machine->size = def.size;
	machine->sprite = def.sprite;
	machine->inputSide = def.inputCount != 0 ? def.inputSide : DIRECTION2_INVALID;
	machine->outputSide = def.outputCount != 0 ? def.outputSide : DIRECTION2_INVALID;
	machine->animFrame = 0;
}

MachineHandle alloc_machine() {
	Machine* machine = machineAllocator.alloc();
	machine->generation = nextGeneration++;
	if (freeMachineIds.empty()) {
		machine->id = machineIdToMachine.size;
		machineIdToMachine.push_back(machine);
	}
	else {
		machine->id = freeMachineIds.pop_back();
		machineIdToMachine[machine->id] = machine;
	}
	return MachineHandle{ machine, machine->generation };
}

Machine* get_machine_from_tile(V2U pos) {
	if (!tile_in_bounds(pos)) {
		return nullptr;
	}
	U32 machineId = World::MACHINE_NULL_ID;
	World::get_tile(&machineId, I32(pos.x), I32(pos.y));
	if (machineId == World::MACHINE_NULL_ID || machineId >= machineIdToMachine.size) {
		return nullptr;
	}
	return machineIdToMachine[machineId];
}

B32 has_machine(V2U pos) {
	return get_machine_from_tile(pos) != nullptr ? B32_TRUE : B32_FALSE;
}

B32 has_belt(V2U pos) {
	return machine_is_belt(get_machine_from_tile(pos));
}

Direction2 belt_input_side(V2U pos) {
	Machine* belt = get_machine_from_tile(pos);
	return machine_is_belt(belt) ? belt->inputSide : DIRECTION2_INVALID;
}

Direction2 belt_output_side(V2U pos) {
	Machine* belt = get_machine_from_tile(pos);
	return machine_is_belt(belt) ? belt->outputSide : DIRECTION2_INVALID;
}

B32 tile_can_host_machine(V2U pos) {
	if (!tile_in_bounds(pos)) {
		return B32_FALSE;
	}
	U32 existingId = World::MACHINE_NULL_ID;
	World::TileType tile = World::get_tile(&existingId, I32(pos.x), I32(pos.y));
	if (existingId != World::MACHINE_NULL_ID) {
		return B32_FALSE;
	}
	switch (tile) {
	case World::TILE_GRASS:
	case World::TILE_SAND:
	case World::TILE_BEACH:
		return B32_TRUE;
	default:
		return B32_FALSE;
	}
}

MachineHandle try_place_machine(V2U pos, const MachineDef& def) {
	for (I32 y = I32(pos.y); y < I32(pos.y + def.size.y); y++) {
		for (I32 x = I32(pos.x); x < I32(pos.x + def.size.x); x++) {
			if (!tile_can_host_machine(V2U{ U32(x), U32(y) })) {
				return MachineHandle{};
			}
		}
	}

	MachineHandle handle = alloc_machine();
	Machine* machine = handle.machine;
	machine->pos = pos;
	machine->inventoryStackSizeLimit = 1;
	apply_machine_def(machine, def);
	World::set_machine(Rng2I32{ I32(pos.x), I32(pos.y), I32(pos.x + def.size.x - 1), I32(pos.y + def.size.y - 1) }, machine->id);
	machineTiles.push_back(machine);
	return handle;
}

B32 set_belt_shape(V2U pos, Direction2 src, Direction2 dst) {
	if (src == DIRECTION2_INVALID || dst == DIRECTION2_INVALID || src == dst) {
		return B32_FALSE;
	}
	MachineDef def = get_belt(src, dst);
	if (!def.sprite) {
		return B32_FALSE;
	}

	Machine* existing = get_machine_from_tile(pos);
	if (existing) {
		if (!machine_is_belt(existing)) {
			return B32_FALSE;
		}
		apply_machine_def(existing, def);
		return B32_TRUE;
	}

	return try_place_machine(pos, def).get() ? B32_TRUE : B32_FALSE;
}

B32 place_belt(V2U pos) {
	Machine* existing = get_machine_from_tile(pos);
	if (existing) {
		return machine_is_belt(existing);
	}
	return set_belt_shape(pos, DIRECTION2_LEFT, DIRECTION2_RIGHT);
}

void remove_machine(Machine* machine) {
	if (!machine || machine->generation == 0) {
		return;
	}
	machineIdToMachine[machine->id] = nullptr;
	freeMachineIds.push_back(machine->id);
	World::set_machine(Rng2I32{ I32(machine->pos.x), I32(machine->pos.y), I32(machine->pos.x + machine->size.x - 1), I32(machine->pos.y + machine->size.y - 1) }, World::MACHINE_NULL_ID);
	machineTiles.remove_obj_unordered(machine);
	machine->generation = 0;
	machineAllocator.free(machine);
}

void remove_machine(V2U pos) {
	if (Machine* machine = get_machine_from_tile(pos)) {
		remove_machine(machine);
	}
}

void reset() {
	for (U32 i = 0; i < machineTiles.size; i++) {
		Machine* machine = machineTiles[i];
		if (!machine || machine->generation == 0) {
			continue;
		}
		machineIdToMachine[machine->id] = nullptr;
		machine->generation = 0;
		machineAllocator.free(machine);
	}
	freeMachineIds.clear();
	machineTiles.clear();
	for (U32 i = 1; i < machineIdToMachine.size; i++) {
		machineIdToMachine[i] = nullptr;
	}
	for (U32 y = 0; y < World::size.y; y++) {
		for (U32 x = 0; x < World::size.x; x++) {
			World::tileMachineIds[y * World::size.x + x] = World::MACHINE_NULL_ID;
		}
	}
}

void update() {
	F64 time = current_time_seconds();
	for (Machine* machine : machineTiles) {
		if (!machine || !machine->sprite || machine->sprite->animFrames == 0) {
			continue;
		}
		machine->animFrame = U32(fractf64(time * 2.5) * F64(machine->sprite->animFrames)) % machine->sprite->animFrames;
	}
	tickCount++;
}

void render(I32 tileScale) {
	for (Machine* machine : machineTiles) {
		if (!machine || !machine->sprite) {
			continue;
		}
		V2I screenPos = Cyber5eagull::tile_to_screen_px(machine->pos);
		Graphics::blit_sprite_cutout(*machine->sprite, screenPos.x, screenPos.y, tileScale, machine->animFrame);
	}
}

void init() {
	machineAllocator.arena = &globalArena;
	freeMachineIds.allocator = &globalArena;
	machineIdToMachine.allocator = &globalArena;
	machineTiles.allocator = &globalArena;
	freeMachineIds.clear();
	machineIdToMachine.clear();
	machineTiles.clear();
	machineIdToMachine.push_back(nullptr);
	nextGeneration = 1;
	tickCount = 0;
}

}
