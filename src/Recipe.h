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
		ItemType inputs[MAX_UNIQUE_INPUTS];
		U32 inputCounts[MAX_UNIQUE_INPUTS];

		ItemType output;
		U32 outputCount;

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
		RecipeDef ironSmelt;
		RecipeDef ironGear;
	} recipeList;

	struct {
		RecipeGroup assembler;
	} recipeGroups;


	void init() {
		recipeList.ironSmelt = RecipeDef{
			1, {ITEM_IRON_ORE}, {2},
			ITEM_IRON_PLATE, 1,
			5.0, &Resources::tile.item.ironPlate
		};
		recipeList.ironGear = RecipeDef{
			1, {ITEM_IRON_PLATE}, {4},
			ITEM_GEAR, 3,
			2.0,& Resources::tile.item.gear
		};

		recipeGroups.assembler = RecipeGroup::make(make_arena_array_list(globalArena, &recipeList.ironSmelt,&recipeList.ironGear));
	}

	// This is the struct you want to put in the tiles with recipes
	struct RecipeRef {
		RecipeDef* def;
		F32 progress;

		static RecipeRef from(RecipeDef* def) {
			return RecipeRef{ def, def->time };
		}
		void reset() {
			if(def != nullptr) progress = def->time;
		}
		// call every frame; return true if recipe has just finished
		bool tick(F32 dt) {
			progress -= dt;
			return progress <= 0.0;
		}
	};

	// given a recipe group and a callback, call when a recipe is chosen
	void set_select_ui_recipes(RecipeGroup& group, void(*chosen_callback)(U32)) {
		SelectUI::change_select_options(group.sprites, chosen_callback);
		SelectUI::open = true;
	}
};