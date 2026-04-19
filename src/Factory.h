#pragma once

#include "drillengine/DrillLib.h"

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

struct Machine {
	MachineType type;
	ItemStack inventory;
	U32 inventoryStackSizeLimit;
	Machine* output;
};

struct MachineDef {
	V2U size;
};

MachineDef rotate(MachineDef& src, Direction2 direction) {
	MachineDef result = src;
}

MachineDef get_belt(Direction2 src, Direction2 dst) {
	DEBUG_ASSERT(src != dst, "Belt cannot do a U turn"a);
	MachineDef result{};
	switch (src) {
	case DIRECTION2_LEFT: {
	} break;
	case DIRECTION2_RIGHT: {
	} break;
	case DIRECTION2_FRONT: {
	} break;
	case DIRECTION2_BACK: {
	} break;
	return result;
}

ArenaArrayList<Machine*> machineTiles;
U32 tickCount;

void update() {
	//TODO reverse post order
	for (Machine* machine : machineTiles) {
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

}