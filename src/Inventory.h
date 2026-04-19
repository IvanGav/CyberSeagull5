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
	ITEM_CONVEYOR,
	ITEM_Count
};

struct ItemStack {
    Inventory::ItemType item;
    U32 count;
};

Resources::Sprite* itemSprite[ITEM_Count];

U32 item_font_size = 32;
U32 x_off = 10;
U32 y_off = 20;

ArenaArrayList<U32> inv; // inv[item] = total amount of that item
ItemType selectedItem = ITEM_Count;

static constexpr I32 PANEL_BORDER = 3;
static constexpr I32 PANEL_PADDING = 8;
static constexpr I32 ROW_GAP = 4;

FINLINE B32 has_selected_item() {
    return selectedItem != ITEM_Count ? B32_TRUE : B32_FALSE;
}

FINLINE ItemType selected_item() {
    return selectedItem;
}

FINLINE void clear_selected_item() {
    selectedItem = ITEM_Count;
}

FINLINE B32 item_selected(ItemType item) {
    return selectedItem == item ? B32_TRUE : B32_FALSE;
}

FINLINE I32 row_height() {
    return I32(item_font_size) + 8;
}

FINLINE I32 panel_width() {
    return I32(item_font_size) + 52;
}

FINLINE I32 panel_height() {
    return PANEL_PADDING * 2 + I32(inv.size) * row_height() + max(I32(inv.size) - 1, 0) * ROW_GAP + PANEL_BORDER * 2;
}

FINLINE B32 panel_contains(V2F32 mouse) {
    I32 px = I32(mouse.x);
    I32 py = I32(mouse.y);
    I32 w = panel_width();
    I32 h = panel_height();
    return px >= I32(x_off) && px < I32(x_off) + w && py >= I32(y_off) && py < I32(y_off) + h ? B32_TRUE : B32_FALSE;
}

FINLINE I32 row_y(U32 index) {
    return I32(y_off) + PANEL_BORDER + PANEL_PADDING + I32(index) * (row_height() + ROW_GAP);
}

I32 item_index_at_screen(V2F32 mouse) {
    if (!panel_contains(mouse)) {
        return -1;
    }

    I32 localX = I32(mouse.x) - I32(x_off) - PANEL_BORDER;
    I32 localY = I32(mouse.y) - I32(y_off) - PANEL_BORDER - PANEL_PADDING;
    if (localX < 0 || localY < 0) {
        return -1;
    }

    I32 stride = row_height() + ROW_GAP;
    if (stride <= 0) {
        return -1;
    }

    I32 index = localY / stride;
    if (index < 0 || U32(index) >= inv.size) {
        return -1;
    }

    I32 withinRowY = localY - index * stride;
    if (withinRowY >= row_height()) {
        return -1;
    }

    return index;
}
FINLINE B32 item_valid(ItemType item) {
    return item < inv.size ? B32_TRUE : B32_FALSE;
}

FINLINE void clear_counts() {
    for (U32 i = 0; i < inv.size; i++) {
        inv[i] = 0;
    }
}

FINLINE U32 count(ItemType item) {
    return item_valid(item) ? inv[item] : 0u;
}


B32 click_callback(V2F32 mouse) {
    if (!panel_contains(mouse)) {
        return B32_FALSE;
    }

    I32 index = item_index_at_screen(mouse);
    if (index < 0 || U32(index) >= inv.size) {
        return B32_TRUE;
    }

    ItemType item = ItemType(index);
    if (count(item) == 0u) {
        return B32_TRUE;
    }

    if (selectedItem == item) {
        selectedItem = ITEM_Count;
    }
    else {
        selectedItem = item;
    }
    return B32_TRUE;
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

void init() {
    inv.clear();
    inv.resize(ITEM_Count);
    clear_counts();
    selectedItem = ITEM_Count;

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
	itemSprite[ITEM_CONVEYOR] = &Resources::tile.icon.belt;

}

void draw_inv() {
    I32 panelX = I32(x_off);
    I32 panelY = I32(y_off);
    I32 panelW = panel_width();
    I32 panelH = panel_height();
    Graphics::box(panelX, panelY, panelW, panelH, PANEL_BORDER, RGBA8{ 24, 24, 24, 255 }, RGBA8{ 70, 78, 98, 255 });

    for (U32 i = 0; i < inv.size; i++) {
        I32 rowX = panelX + PANEL_BORDER + 4;
        I32 rowY = row_y(i);
        I32 rowW = panelW - PANEL_BORDER * 2 - 8;
        I32 rowH = row_height();
        RGBA8 fill = inv[i] > 0u ? RGBA8{ 96, 114, 146, 255 } : RGBA8{ 66, 70, 78, 255 };
        RGBA8 border = inv[i] > 0u ? RGBA8{ 28, 28, 28, 255 } : RGBA8{ 44, 44, 44, 255 };
        if (selectedItem == ItemType(i)) {
            fill = RGBA8{ 150, 168, 82, 255 };
            border = RGBA8{ 235, 235, 96, 255 };
        }
        Graphics::box(rowX, rowY, rowW, rowH, 2, border, fill);

        I32 iconX = rowX + 4;
        I32 iconY = rowY + 4;
        Graphics::blit_sprite_cutout(*itemSprite[i], iconX, iconY, item_font_size / 16, 0);
        Graphics::display_num(inv[i], iconX + I32(item_font_size) + 6, rowY + 4, item_font_size);
    }
}

}