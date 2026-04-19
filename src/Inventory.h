#pragma once

#include "drillengine/DrillLib.h"
#include "Graphics.h"
#include "Win32.h"
#include "Resources.h"

namespace Inventory {

enum ItemType : U8 {
    ITEM_IRON_ORE,
    ITEM_COPPER_ORE,
    ITEM_GULL,
    ITEM_COPPER_CABLE,
    ITEM_IRON_PLATE,
    ITEM_GREEN_CIRCUIT,
    ITEM_CAMERA,
    ITEM_FEATHER,
    ITEM_GEAR,
    ITEM_NUCLEAR_HEART,
    ITEM_URANIUM,
   	ITEM_POLLEN,
	ITEM_HONEY,
	ITEM_LEMON_JUICE,
	ITEM_Count
};

Resources::Sprite* itemSprite[ITEM_Count];

U32 item_font_size = 32;
U32 x_off = 10;
U32 y_off = 20;

ArenaArrayList<U32> inv; // inv[item] = total amount of that item

FINLINE B32 item_valid(ItemType item) {
    return item < inv.size ? B32_TRUE : B32_FALSE;
}

FINLINE void clear_counts() {
    for (U32 i = 0; i < inv.size; i++) {
        inv[i] = 0;
    }
}

FINLINE void add_item(ItemType item, U32 amount = 1) {
    if (!item_valid(item) || amount == 0) {
        return;
    }
    inv[item] += amount;
}

FINLINE B32 try_take_item(ItemType item, U32 amount = 1) {
    if (!item_valid(item) || amount == 0 || inv[item] < amount) {
        return B32_FALSE;
    }
    inv[item] -= amount;
    return B32_TRUE;
}

FINLINE U32 count(ItemType item) {
    return item_valid(item) ? inv[item] : 0u;
}

void init() {
    inv.clear();
    inv.resize(ITEM_Count);
    clear_counts();

    itemSprite[ITEM_IRON_ORE] = &Resources::tile.item.ironOre;
    itemSprite[ITEM_COPPER_ORE] = &Resources::tile.item.copperOre;
    itemSprite[ITEM_GULL] = &Resources::tile.item.gull;
    itemSprite[ITEM_COPPER_CABLE] = &Resources::tile.item.copperCable;
    itemSprite[ITEM_IRON_PLATE] = &Resources::tile.item.ironPlate;
    itemSprite[ITEM_GREEN_CIRCUIT] = &Resources::tile.item.greenCircuit;
    itemSprite[ITEM_CAMERA] = &Resources::tile.item.camera;
    itemSprite[ITEM_FEATHER] = &Resources::tile.item.feather;
    itemSprite[ITEM_GEAR] = &Resources::tile.item.gear;
    itemSprite[ITEM_NUCLEAR_HEART] = &Resources::tile.item.nuclearHeart;
    itemSprite[ITEM_URANIUM] = &Resources::tile.item.uranium;
  	itemSprite[ITEM_POLLEN] = &Resources::tile.item.pollen;
	itemSprite[ITEM_HONEY] = &Resources::tile.item.honey;
  	itemSprite[ITEM_LEMON_JUICE] = &Resources::tile.item.lemonJuice;

}

void draw_inv() {
    for (U32 i = 0; i < inv.size; i++) {
        Graphics::blit_sprite_cutout(*itemSprite[i], x_off, y_off + (item_font_size + 2) * i, item_font_size / 16, 0);
        Graphics::display_num(inv[i], x_off + item_font_size, y_off + (item_font_size + 2) * i, item_font_size);
    }
}

}