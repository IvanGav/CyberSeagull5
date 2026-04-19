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
	} recipeList;

	struct {
		RecipeGroup belt;
		RecipeGroup smelter;
		RecipeGroup assembler;
	} recipeGroups;


	void init() {
		recipeList.unit = RecipeDef{
			0, {}, {}, 1.0, nullptr // defines a 1 second crafting time with 0 inputs/outputs; special case for belts and such
		};
		recipeList.ironSmelt = RecipeDef{
			1, {{ITEM_IRON_ORE, 2}},
			{ITEM_IRON_PLATE, 1 },
			5.0, &Resources::tile.item.ironPlate
		};
		recipeList.ironGear = RecipeDef{
			1, {{ITEM_IRON_PLATE, 4}},
			{ITEM_GEAR, 3 },
			2.0,& Resources::tile.item.gear
		};

		recipeGroups.belt = RecipeGroup::make(make_arena_array_list(globalArena, &recipeList.unit));
		recipeGroups.smelter = RecipeGroup::make(make_arena_array_list(globalArena, &recipeList.ironSmelt));
		recipeGroups.assembler = RecipeGroup::make(make_arena_array_list(globalArena, &recipeList.ironGear));
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