#pragma once

#include "drillengine/DrillLib.h"
#include "Resources.h"
#include "Graphics.h"
#include "World.h"
#include "Inventory.h"

namespace Cyber5eagull {
V2I tile_to_screen_px(V2U tile);
}

namespace Factory {

struct ItemStack {
	Inventory::ItemType item;
	U32 count;
};

enum MachineType : U32 {
	MACHINE_NONE,
	MACHINE_BELT,
	MACHINE_ASSEMBLER,
	MACHINE_SPLITTER,
	MACHINE_MERGER,
	MACHINE_Count
};

const U32 MAX_IO_DEFS = 8;

struct IODef {
	V2I pos;
	Flags8 ioDirections;
};

struct Machine;

struct MachineHandle {
	Machine* machine = nullptr;
	U64 generation = 0;

	FINLINE Machine* get() const;
};
struct Machine {
	MachineType type = MACHINE_NONE;
	U64 generation = 0;
	Resources::Sprite* sprite = nullptr;
	Resources::Sprite* spriteProcessingAlt = nullptr;
	V2U pos{};
	V2U size{ 1, 1 };
	U32 id = 0;
	U32 animFrame = 0;
	ItemStack inventory{};
	U32 inventoryStackSizeLimit = 1;
	F32 processTime;
	F32 maxProcessTime;
	U32 amountToProcess;
	MachineHandle output;
	IODef ioDefs[MAX_IO_DEFS];
};

FINLINE Machine* MachineHandle::get() const {
	return (!machine || machine->generation == 0 || machine->generation != generation) ? nullptr : machine;
}

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
	Resources::Sprite* spriteProcessingAlt = nullptr;
	U32 inventoryStackSize = 1;
	U32 processAtOnce = 1;
	F32 maxProcessTime = 0.5F;
	IODef ioDefs[MAX_IO_DEFS];
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

void update_machine_connections(Machine* machine) {
	if (!machine) {
		return;
	}
	machine->output = MachineHandle{};
	for (U32 i = 0; i < MAX_IO_DEFS; i++) {
		IODef io = machine->ioDefs[i];
		if (io.ioDirections == 0) {
			continue;
		}
		if (io.ioDirections & World::MACHINE_OUTPUT_DOWN && World::can_connect_input(V2U{ machine->pos.x + io.pos.x, machine->pos.y + io.pos.y + 1 }, DIRECTION2_FRONT)) {
			Machine* m = get_machine_from_tile(V2U{ machine->pos.x + io.pos.x, machine->pos.y + io.pos.y + 1 });;
			machine->output = MachineHandle{ m, m->generation };
			break;
		}
		if (io.ioDirections & World::MACHINE_OUTPUT_UP && World::can_connect_input(V2U{ machine->pos.x + io.pos.x, machine->pos.y + io.pos.y - 1 }, DIRECTION2_BACK)) {
			Machine* m = get_machine_from_tile(V2U{ machine->pos.x + io.pos.x, machine->pos.y + io.pos.y - 1 });;
			machine->output = MachineHandle{ m, m->generation };
			break;
		}
		if (io.ioDirections & World::MACHINE_OUTPUT_LEFT && World::can_connect_input(V2U{ machine->pos.x + io.pos.x - 1, machine->pos.y + io.pos.y }, DIRECTION2_RIGHT)) {
			Machine* m = get_machine_from_tile(V2U{ machine->pos.x + io.pos.x - 1, machine->pos.y + io.pos.y });;
			machine->output = MachineHandle{ m, m->generation };
			break;
		}
		if (io.ioDirections & World::MACHINE_OUTPUT_RIGHT && World::can_connect_input(V2U{ machine->pos.x + io.pos.x + 1, machine->pos.y + io.pos.y }, DIRECTION2_LEFT)) {
			Machine* m = get_machine_from_tile(V2U{ machine->pos.x + io.pos.x + 1, machine->pos.y + io.pos.y });;
			machine->output = MachineHandle{ m, m->generation };
			break;
		}
	}
}

IODef rotate_iodef(IODef io, V2U bounds, Rotation2 orientation) {
	V2I boundsi{ I32(bounds.x), I32(bounds.y) };
	Flags8 newDirections{};
	switch (orientation) {
	case ROTATION2_0: newDirections = io.ioDirections; break;
	case ROTATION2_90: {
		if (io.ioDirections & World::MACHINE_INPUT_UP) newDirections |= World::MACHINE_INPUT_RIGHT;
		if (io.ioDirections & World::MACHINE_INPUT_DOWN) newDirections |= World::MACHINE_INPUT_LEFT;
		if (io.ioDirections & World::MACHINE_INPUT_LEFT) newDirections |= World::MACHINE_INPUT_UP;
		if (io.ioDirections & World::MACHINE_INPUT_RIGHT) newDirections |= World::MACHINE_INPUT_DOWN;
		if (io.ioDirections & World::MACHINE_OUTPUT_UP) newDirections |= World::MACHINE_OUTPUT_RIGHT;
		if (io.ioDirections & World::MACHINE_OUTPUT_DOWN) newDirections |= World::MACHINE_OUTPUT_LEFT;
		if (io.ioDirections & World::MACHINE_OUTPUT_LEFT) newDirections |= World::MACHINE_OUTPUT_UP;
		if (io.ioDirections & World::MACHINE_OUTPUT_RIGHT) newDirections |= World::MACHINE_OUTPUT_DOWN;
	} break;
	case ROTATION2_180: {
		if (io.ioDirections & World::MACHINE_INPUT_UP) newDirections |= World::MACHINE_INPUT_DOWN;
		if (io.ioDirections & World::MACHINE_INPUT_DOWN) newDirections |= World::MACHINE_INPUT_UP;
		if (io.ioDirections & World::MACHINE_INPUT_LEFT) newDirections |= World::MACHINE_INPUT_RIGHT;
		if (io.ioDirections & World::MACHINE_INPUT_RIGHT) newDirections |= World::MACHINE_INPUT_LEFT;
		if (io.ioDirections & World::MACHINE_OUTPUT_UP) newDirections |= World::MACHINE_OUTPUT_DOWN;
		if (io.ioDirections & World::MACHINE_OUTPUT_DOWN) newDirections |= World::MACHINE_OUTPUT_UP;
		if (io.ioDirections & World::MACHINE_OUTPUT_LEFT) newDirections |= World::MACHINE_OUTPUT_RIGHT;
		if (io.ioDirections & World::MACHINE_OUTPUT_RIGHT) newDirections |= World::MACHINE_OUTPUT_LEFT;
	} break;
	case ROTATION2_270: {
		if (io.ioDirections & World::MACHINE_INPUT_UP) newDirections |= World::MACHINE_INPUT_LEFT;
		if (io.ioDirections & World::MACHINE_INPUT_DOWN) newDirections |= World::MACHINE_INPUT_RIGHT;
		if (io.ioDirections & World::MACHINE_INPUT_LEFT) newDirections |= World::MACHINE_INPUT_DOWN;
		if (io.ioDirections & World::MACHINE_INPUT_RIGHT) newDirections |= World::MACHINE_INPUT_UP;
		if (io.ioDirections & World::MACHINE_OUTPUT_UP) newDirections |= World::MACHINE_OUTPUT_LEFT;
		if (io.ioDirections & World::MACHINE_OUTPUT_DOWN) newDirections |= World::MACHINE_OUTPUT_RIGHT;
		if (io.ioDirections & World::MACHINE_OUTPUT_LEFT) newDirections |= World::MACHINE_OUTPUT_DOWN;
		if (io.ioDirections & World::MACHINE_OUTPUT_RIGHT) newDirections |= World::MACHINE_OUTPUT_UP;
	} break;
	}
	
	return IODef{ apply_rotation(io.pos, orientation, boundsi), newDirections };
}

FINLINE B32 machine_is_belt(const Machine* machine) {
	return machine && machine->generation != 0 && machine->type == MACHINE_BELT ? B32_TRUE : B32_FALSE;
}

MachineDef get_assembler(Rotation2 orientation) {
	MachineDef result{};
	result.type = MACHINE_ASSEMBLER;
	result.size = V2U{ 2, 2 };
	switch (orientation) {
	case ROTATION2_0: result.sprite = &Resources::tile.assembler.downOff; result.spriteProcessingAlt = &Resources::tile.assembler.downOn; break;
	case ROTATION2_90: result.sprite = &Resources::tile.assembler.leftOff; result.spriteProcessingAlt = &Resources::tile.assembler.leftOn; break;
	case ROTATION2_180: result.sprite = &Resources::tile.assembler.upOff; result.spriteProcessingAlt = &Resources::tile.assembler.upOn; break;
	case ROTATION2_270: result.sprite = &Resources::tile.assembler.rightOff; result.spriteProcessingAlt = &Resources::tile.assembler.rightOn; break;
	}
	result.inventoryStackSize = 2;
	result.maxProcessTime = 2.0F;
	result.ioDefs[0] = rotate_iodef(IODef{ V2I{ 0, 1 }, World::MACHINE_INPUT_DOWN }, result.size, orientation);
	result.ioDefs[1] = rotate_iodef(IODef{ V2I{ 1, 1 }, World::MACHINE_OUTPUT_DOWN }, result.size, orientation);
	return result;
}

Flags8 direction_to_input_flag(Direction2 dir) {
	switch (dir) {
	case DIRECTION2_LEFT: return World::MACHINE_INPUT_LEFT;
	case DIRECTION2_RIGHT: return World::MACHINE_INPUT_RIGHT;
	case DIRECTION2_FRONT: return World::MACHINE_INPUT_UP;
	case DIRECTION2_BACK: return World::MACHINE_INPUT_DOWN;
	}
	return 0;
}
Flags8 direction_to_output_flag(Direction2 dir) {
	switch (dir) {
	case DIRECTION2_LEFT: return World::MACHINE_OUTPUT_LEFT;
	case DIRECTION2_RIGHT: return World::MACHINE_OUTPUT_RIGHT;
	case DIRECTION2_FRONT: return World::MACHINE_OUTPUT_UP;
	case DIRECTION2_BACK: return World::MACHINE_OUTPUT_DOWN;
	}
	return 0;
}

Direction2 input_flag_to_direction(Flags8 flag) {
	if (flag & World::MACHINE_INPUT_DOWN) return DIRECTION2_BACK;
	if (flag & World::MACHINE_INPUT_UP) return DIRECTION2_FRONT;
	if (flag & World::MACHINE_INPUT_RIGHT) return DIRECTION2_RIGHT;
	if (flag & World::MACHINE_INPUT_LEFT) return DIRECTION2_LEFT;
	return DIRECTION2_INVALID;
}
Direction2 output_flag_to_direction(Flags8 flag) {
	if (flag & World::MACHINE_OUTPUT_DOWN) return DIRECTION2_BACK;
	if (flag & World::MACHINE_OUTPUT_UP) return DIRECTION2_FRONT;
	if (flag & World::MACHINE_OUTPUT_RIGHT) return DIRECTION2_RIGHT;
	if (flag & World::MACHINE_OUTPUT_LEFT) return DIRECTION2_LEFT;
	return DIRECTION2_INVALID;
}

FINLINE B32 machine_is_static_structure(const Machine* machine) {
	return machine && machine->generation != 0 && machine->type != MACHINE_NONE && machine->type != MACHINE_BELT ? B32_TRUE : B32_FALSE;
}

FINLINE V2U machine_footprint(MachineType type, Rotation2 orientation) {
	switch (type) {
	case MACHINE_ASSEMBLER: return V2U{ 2, 2 };
	case MACHINE_SPLITTER:
	case MACHINE_MERGER:
	case MACHINE_BELT:
	default: return V2U{ 1, 1 };
	}
}

MachineDef get_belt(Direction2 src, Direction2 dst) {
	DEBUG_ASSERT(src != DIRECTION2_INVALID && dst != DIRECTION2_INVALID, "Belt direction cannot be invalid"a);
	DEBUG_ASSERT(src != dst, "Belt cannot do a U turn"a);
	MachineDef result{};
	result.type = MACHINE_BELT;
	result.size = V2U{ 1, 1 };
	result.ioDefs[0] = IODef{ V2I{ 0, 0 }, Flags8(direction_to_input_flag(src) | direction_to_output_flag(dst)) };
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

MachineDef get_static_machine(MachineType type, Rotation2 orientation) {
	MachineDef result{};
	result.type = type;
	switch (type) {
	case MACHINE_ASSEMBLER:
		result = get_assembler(orientation);
		break;
	case MACHINE_SPLITTER:
		result.size = V2U{ 1, 1 };
		result.sprite = &Resources::tile.splitter;
		break;
	case MACHINE_MERGER:
		result.size = V2U{ 1, 1 };
		result.sprite = &Resources::tile.merger;
		break;
	default:
		break;
	}
	return result;
}

void apply_machine_def(Machine* machine, const MachineDef& def) {
	machine->type = def.type;
	machine->size = def.size;
	machine->sprite = def.sprite;
	machine->spriteProcessingAlt = def.spriteProcessingAlt;
	machine->animFrame = 0;
	machine->maxProcessTime = def.maxProcessTime;
	machine->amountToProcess = def.processAtOnce;
	machine->inventoryStackSizeLimit = def.inventoryStackSize;
	memcpy(machine->ioDefs, def.ioDefs, sizeof(machine->ioDefs));
	for (U32 i = 0; i < MAX_IO_DEFS; i++) {
		if (machine->ioDefs[i].ioDirections != 0) {
			World::set_connectivity(V2U{ machine->pos.x + machine->ioDefs[i].pos.x, machine->pos.y + machine->ioDefs[i].pos.y }, machine->ioDefs[i].ioDirections);
		}
	}
	for (U32 i = 0; i < MAX_IO_DEFS; i++) {
		if (machine->ioDefs[i].ioDirections == 0) {
			continue;
		}
		update_machine_connections(machine);
		update_machine_connections(get_machine_from_tile(V2U{ machine->pos.x + machine->ioDefs[i].pos.x + 1, machine->pos.y + machine->ioDefs[i].pos.y }));
		update_machine_connections(get_machine_from_tile(V2U{ machine->pos.x + machine->ioDefs[i].pos.x - 1, machine->pos.y + machine->ioDefs[i].pos.y }));
		update_machine_connections(get_machine_from_tile(V2U{ machine->pos.x + machine->ioDefs[i].pos.x, machine->pos.y + machine->ioDefs[i].pos.y + 1 }));
		update_machine_connections(get_machine_from_tile(V2U{ machine->pos.x + machine->ioDefs[i].pos.x, machine->pos.y + machine->ioDefs[i].pos.y - 1 }));
	}
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

B32 has_machine(V2U pos) {
	return get_machine_from_tile(pos) != nullptr ? B32_TRUE : B32_FALSE;
}

B32 has_belt(V2U pos) {
	return machine_is_belt(get_machine_from_tile(pos));
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
	World::set_machine(Rng2I32{ I32(pos.x), I32(pos.y), I32(pos.x + def.size.x - 1), I32(pos.y + def.size.y - 1) }, machine->id);
	machineTiles.push_back(machine);
	apply_machine_def(machine, def);
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

B32 place_machine_type(V2U pos, MachineType type, Rotation2 orientation) {
	if (type == MACHINE_BELT) {
		return place_belt(pos);
	}
	MachineDef def = get_static_machine(type, orientation);
	if (!def.sprite) {
		return B32_FALSE;
	}
	Machine* existing = get_machine_from_tile(pos);
	if (existing) {
		if (existing->type == type) {
			return B32_TRUE;
		}
		return B32_FALSE;
	}
	return try_place_machine(pos, def).get() ? B32_TRUE : B32_FALSE;
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
	for (U32 i = 0; i < MAX_IO_DEFS; i++) {
		if (machine->ioDefs[i].ioDirections == 0) {
			continue;
		}
		update_machine_connections(get_machine_from_tile(V2U{ machine->pos.x + machine->ioDefs[i].pos.x + 1, machine->pos.y + machine->ioDefs[i].pos.y }));
		update_machine_connections(get_machine_from_tile(V2U{ machine->pos.x + machine->ioDefs[i].pos.x - 1, machine->pos.y + machine->ioDefs[i].pos.y }));
		update_machine_connections(get_machine_from_tile(V2U{ machine->pos.x + machine->ioDefs[i].pos.x, machine->pos.y + machine->ioDefs[i].pos.y + 1 }));
		update_machine_connections(get_machine_from_tile(V2U{ machine->pos.x + machine->ioDefs[i].pos.x, machine->pos.y + machine->ioDefs[i].pos.y - 1 }));
	}
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

U32 animRawTime;

void update(F32 dt) {
	F64 time = current_time_seconds();
	animRawTime = U32(fractf64(time) * 100.0);
	for (Machine* machine : machineTiles) {
		machine->animFrame = U32(time * 10.0F) % machine->sprite->animFrames;
		machine->processTime -= dt;
		if (machine->inventory.count != 0 && machine->processTime <= 0.0F) {
			if (Machine* output = machine->output.get()) {
				if (output->inventory.count == 0) {
					output->processTime = output->maxProcessTime;
				}
				if (output->inventory.count == 0 || output->inventory.item == machine->inventory.item) {
					I32 amountToTransfer = max(I32(min(output->inventoryStackSizeLimit, output->inventory.count + machine->inventory.count, machine->amountToProcess) - output->inventory.count), 0);
					output->inventory.item = machine->inventory.item;
					output->inventory.count += amountToTransfer;
					machine->inventory.count -= amountToTransfer;
					if (amountToTransfer > 0) {
						machine->processTime = machine->maxProcessTime;
					}
				}
			}
		}
		machine->animFrame = animRawTime / 8 % machine->sprite->animFrames;
	}
	tickCount++;
}

void render(I32 tileScale) {
	for (Machine* machine : machineTiles) {
		if (!machine || !machine->sprite) {
			continue;
		}
		V2I screenPos = Cyber5eagull::tile_to_screen_px(machine->pos);
		if (machine->type == MACHINE_ASSEMBLER) {
			U32 beltAnimTime = animRawTime / 8 % Resources::tile.belt.downToUp.animFrames;
			if (machine->sprite == &Resources::tile.assembler.downOff) {
				Graphics::blit_sprite_cutout(Resources::tile.belt.downToUp, screenPos.x, screenPos.y + 16 * tileScale, tileScale, beltAnimTime);
				Graphics::blit_sprite_cutout(Resources::tile.belt.upToDown, screenPos.x + 16 * tileScale, screenPos.y + 16 * tileScale, tileScale, beltAnimTime);
			}
		}
		Graphics::blit_sprite_cutout(machine->spriteProcessingAlt && machine->inventory.count ? *machine->spriteProcessingAlt : *machine->sprite, screenPos.x, screenPos.y, tileScale, machine->animFrame);
	}
	for (Machine* machine : machineTiles) {
		if (machine && machine->type == MACHINE_BELT && machine->inventory.count > 0) {
			F32 t = clamp01(machine->processTime / machine->maxProcessTime);
			V2F renderStartPos = DIRECTION2_V2F[input_flag_to_direction(machine->ioDefs[0].ioDirections)] * 0.5F + 0.5F;
			V2F renderEndPos = DIRECTION2_V2F[output_flag_to_direction(machine->ioDefs[0].ioDirections)] * 0.5F + 0.5F;
			V2F renderOffset = (renderEndPos + (renderStartPos - renderEndPos) * t) * 16 * tileScale - 8 * tileScale;
			V2I screenPos = Cyber5eagull::tile_to_screen_px(machine->pos);
			Graphics::blit_sprite_cutout(*Inventory::itemSprite[machine->inventory.item], screenPos.x + I32(renderOffset.x), screenPos.y + I32(renderOffset.y), tileScale, 0);
		}
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
