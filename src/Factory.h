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

struct Machine {
	ItemStack inventory;
	U32 inventoryStackSizeLimit;
	Machine* output;
};


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