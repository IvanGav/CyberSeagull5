#pragma once

#include "drillengine/DrillLib.h"
#include "Resources.h"
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
	Item item;
	U32 count;
};

enum MachineType {
	MACHINE_NONE,
	MACHINE_BELT,
	MACHINE_Count
};

U64 nextGeneration = 1;
struct Machine {
	MachineType type;
	U64 generation;
	Resources::Sprite* sprite;
	V2U pos;
	V2U size;
	U32 id;
	U32 animFrame;
	ItemStack inventory;
	U32 inventoryStackSizeLimit;
	Machine* output;
};

struct MachineHandle {
	Machine* machine;
	U64 generation;

	FINLINE Machine* get() {
		return !machine || machine->generation == 0 || machine->generation > generation ? nullptr : machine;
	}
};

PoolAllocator<Machine> machineAllocator;
ArenaArrayList<U32> freeMachineIds;
ArenaArrayList<Machine*> machineIdToMachine;

struct MachineDef {
	MachineType type;
	V2U size;
	Resources::Sprite* sprite;
	V2I relativeInputs[4];
	V2I relativeOutputs[4];
	U32 inputCount;
	U32 outputCount;
};

MachineDef rotate(MachineDef& src, Rotation2 rot) {
	MachineDef result = src;
	return result;
}

MachineDef get_belt(Direction2 src, Direction2 dst) {
	DEBUG_ASSERT(src != dst, "Belt cannot do a U turn"a);
	MachineDef result{};
	result.type = MACHINE_BELT;
	result.size = V2U{ 1, 1 };
	result.inputCount = 1;
	result.outputCount = 1;
	result.relativeInputs[0] = DIRECTION2_V2I[src];
	result.relativeOutputs[1] = DIRECTION2_V2I[dst];
	switch (src) {
	case DIRECTION2_LEFT: {
		switch (dst) {
		case DIRECTION2_RIGHT: result.sprite = &Resources::tile.belt.leftToRight;  break;
		case DIRECTION2_FRONT: result.sprite = &Resources::tile.belt.leftToUp; break;
		case DIRECTION2_BACK: result.sprite = &Resources::tile.belt.leftToDown; break;
		}
	} break;
	case DIRECTION2_RIGHT: {
		switch (dst) {
		case DIRECTION2_LEFT: result.sprite = &Resources::tile.belt.rightToLeft; break;
		case DIRECTION2_FRONT: result.sprite = &Resources::tile.belt.rightToUp; break;
		case DIRECTION2_BACK: result.sprite = &Resources::tile.belt.rightToDown; break;
		}
	} break;
	case DIRECTION2_FRONT: {
		switch (dst) {
		case DIRECTION2_RIGHT: result.sprite = &Resources::tile.belt.upToRight;  break;
		case DIRECTION2_LEFT: result.sprite = &Resources::tile.belt.upToLeft; break;
		case DIRECTION2_BACK: result.sprite = &Resources::tile.belt.upToDown; break;
		}
	} break;
	case DIRECTION2_BACK: {
		switch (dst) {
		case DIRECTION2_RIGHT: result.sprite = &Resources::tile.belt.downToRight;  break;
		case DIRECTION2_FRONT: result.sprite = &Resources::tile.belt.downToUp; break;
		case DIRECTION2_LEFT: result.sprite = &Resources::tile.belt.downToLeft; break;
		}
	} break;
	}
	return result;
}

MachineHandle alloc_machine() {
	Machine* machine = machineAllocator.alloc();
	machine->generation = nextGeneration++;
	if (freeMachineIds.empty()) {
		machine->id = machineIdToMachine.size;
		machineIdToMachine.push_back(machine);
	} else {
		machine->id = freeMachineIds.pop_back();
		machineIdToMachine.data[machine->id] = machine;
	}
	return MachineHandle{ machine, machine->generation };
}

ArenaArrayList<Machine*> machineTiles;
U32 tickCount;

void remove_machine(Machine* machine) {
	machineIdToMachine[machine->id] = nullptr;
	freeMachineIds.push_back(machine->id);
	World::set_machine(Rng2I32{ I32(machine->pos.x), I32(machine->pos.y), I32(machine->pos.x + machine->size.x - 1), I32(machine->pos.y + machine->size.y - 1) }, World::MACHINE_NULL_ID);
	machineTiles.remove_obj_unordered(machine);
	machine->generation = 0;
	machineAllocator.free(machine);
}

Machine* get_machine_from_tile(V2U pos) {
	U32 machineId;
	World::get_tile(&machineId, pos.x, pos.y);
	if (machineId != World::MACHINE_NULL_ID) {
		Machine* machine = machineIdToMachine.data[machineId];
		return machine;
	}
	return nullptr;
}

void remove_machine(V2U pos) {
	if (Machine* machine = get_machine_from_tile(pos)) {
		remove_machine(machine);
	}
}

MachineHandle try_place_machine(V2U pos, MachineDef& def) {
	for (I32 y = I32(pos.y); y < I32(pos.y + def.size.y); y++) {
		for (I32 x = I32(pos.x); x < I32(pos.x + def.size.x); x++) {
			U32 existingId;
			if (World::get_tile(&existingId, x, y) != World::TILE_GRASS || existingId != World::MACHINE_NULL_ID) {
				return MachineHandle{};
			}
		}
	}
	MachineHandle machine = alloc_machine();
	machine.machine->type = def.type;
	machine.machine->pos = pos;
	machine.machine->size = def.size;
	machine.machine->inventoryStackSizeLimit = 1;
	machine.machine->sprite = def.sprite;
	World::set_machine(Rng2I32{ I32(pos.x), I32(pos.y), I32(pos.x + def.size.x - 1), I32(pos.y + def.size.y - 1) }, machine.machine->id);
	machineTiles.push_back(machine.machine);
	return machine;
}

void update() {
	//TODO reverse post order
	F64 time = current_time_seconds();
	for (Machine* machine : machineTiles) {
		machine->animFrame = U32(time * 10.0F) % machine->sprite->animFrames;
		if (machine->inventory.count != 0) {
			Machine* output = machine->output;
			if (output->inventory.count == 0 || output->inventory.item == machine->inventory.item) {
				I32 amountToTransfer = max(I32(min(output->inventoryStackSizeLimit, output->inventory.count + machine->inventory.count) - machine->inventory.count), 0);
				output->inventory.item = machine->inventory.item;
				output->inventory.count += amountToTransfer;
				machine->inventory.count -= amountToTransfer;
			}
		}
	}
	tickCount++;
}

void render(I32 tileScale) {
	for (Machine* machine : machineTiles) {
		V2I screenPos = Cyber5eagull::tile_to_screen_px(machine->pos);
		Graphics::blit_sprite_cutout(*machine->sprite, screenPos.x, screenPos.y, tileScale, machine->animFrame);
		if (machine->type == MACHINE_BELT) {
			Graphics::blit_sprite_cutout(Resources::tile.beeFly, screenPos.x, screenPos.y, tileScale, 0);
		}
	}
}

void init() {
	machineIdToMachine.push_back(nullptr);
}

}