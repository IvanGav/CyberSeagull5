#pragma once

#include "Resources.h"
#include "Inventory.h"
#include "SelectUI.h"

namespace Recipe {
	using namespace Inventory;

	static constexpr U32 MAX_UNIQUE_INPUTS = 4;

	// singular recipe
	struct RecipeDef {
		U32 numInputs;
		ItemStack inputs[MAX_UNIQUE_INPUTS];

		ItemStack output;

		F32 time;
		Resources::Sprite* recipeSprite;
	};

	ArenaArrayList<Resources::Sprite*> sprites_from(ArenaArrayList<RecipeDef*> list) {
		ArenaArrayList<Resources::Sprite*> out{}; for (U32 i = 0; i < list.size; i++) { out.push_back(list[i]->recipeSprite); } return out;
	}

	// a group of recipes; specified possible selectable recipes for some machine
	struct RecipeGroup {
		ArenaArrayList<RecipeDef*> options;
		ArenaArrayList<Resources::Sprite*> sprites; // for SelectorUI
		static RecipeGroup make(ArenaArrayList<RecipeDef*> list_options) {
			return RecipeGroup{ list_options, sprites_from(list_options) };
		}
	};

	struct {
		RecipeDef unit;
		RecipeDef ironSmelt;
		RecipeDef ironGear;
		RecipeDef copperCable;
		RecipeDef greenCircuit;
		RecipeDef nuclearHeart;
		RecipeDef camera;
		RecipeDef cyberSeagull;
	} recipeList;

	struct {
		RecipeGroup belt;
		RecipeGroup smelter;
		RecipeGroup assembler;
		RecipeGroup furnace;
		RecipeGroup bigAssembler;
	} recipeGroups;


	void init() {
		recipeList.unit = RecipeDef{
			0, {}, {}, 1.0, nullptr // defines a 1 second crafting time with 0 inputs/outputs; special case for belts and such
		};
		recipeList.ironSmelt = RecipeDef{
			1, {{ITEM_IRON_ORE, 3}},
			{ITEM_IRON_PLATE, 1 },
			7.0, &Resources::tile.item.ironPlate
		};
		recipeList.copperCable = RecipeDef{
			1, {{ITEM_COPPER_ORE, 4}},
			{ITEM_COPPER_CABLE, 1 },
			4.0, &Resources::tile.item.copperCable
		};
		recipeList.ironGear = RecipeDef{
			1, {{ITEM_IRON_PLATE, 5}},
			{ITEM_GEAR, 3 },
			2.0,& Resources::tile.item.gear
		};
		recipeList.greenCircuit = RecipeDef{
			//3, {{ITEM_IRON_PLATE, 2}, {ITEM_COPPER_CABLE, 4}, {ITEM_FEATHER, 1}},
			1, {{ITEM_COPPER_CABLE, 8}},
			{ITEM_GREEN_CIRCUIT, 2 },
			8.0,&Resources::tile.item.greenCircuit
		};
		recipeList.nuclearHeart = RecipeDef{
			3, {{ITEM_IRON_PLATE, 5}, {ITEM_URANIUM, 3}, {ITEM_GREEN_CIRCUIT, 1}},
			{ITEM_NUCLEAR_HEART, 1 },
			20.0,&Resources::tile.item.greenCircuit
		};
		recipeList.camera = RecipeDef{
			3, {{ITEM_IRON_PLATE, 2}, {ITEM_GEAR, 8}, {ITEM_GREEN_CIRCUIT, 2}},
			{ITEM_CAMERA, 1 },
			15.0,&Resources::tile.item.greenCircuit
		};
		recipeList.cyberSeagull = RecipeDef{
			3, {{ITEM_NUCLEAR_HEART, 1}, {ITEM_CAMERA, 2}, {ITEM_GULL, 1}},
			{ITEM_LEMON_JUICE, 1 },
			15.0,&Resources::tile.item.greenCircuit
		};

		recipeGroups.belt = RecipeGroup::make(make_arena_array_list(globalArena, &recipeList.unit));
		recipeGroups.furnace = RecipeGroup::make(make_arena_array_list(globalArena, &recipeList.ironSmelt,&recipeList.copperCable));
		recipeGroups.assembler = RecipeGroup::make(make_arena_array_list(globalArena, &recipeList.greenCircuit, &recipeList.ironGear));
		recipeGroups.bigAssembler = RecipeGroup::make(make_arena_array_list(globalArena, &recipeList.nuclearHeart, &recipeList.camera, &recipeList.cyberSeagull));
	}

	// This is the struct you want to put in the tiles with recipes
	struct RecipeRef {
		RecipeDef* def;
		F32 progress;

		static RecipeRef from(RecipeDef* def) {
			return def ? RecipeRef{ def, def->time } : RecipeRef{};
		}
		void reset() {
			if (def != nullptr) progress = def->time;
		}
		// call every frame; return true if recipe has finished
		bool tick(F32 dt) {
			if (def == nullptr) {
				return false;
			}
			progress -= dt;
			if (progress <= 0.0F) {
				progress = 0.0F;
			}
			return progress <= 0.0F;
		}
	};

	// given a recipe group and a callback, call when a recipe is chosen
	void set_select_ui_recipes(RecipeGroup& group, void(*chosen_callback)(U32)) {
		SelectUI::change_select_options(group.sprites, chosen_callback);
		SelectUI::open = true;
	}
};