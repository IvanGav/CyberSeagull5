#pragma once

#include "drillengine/DrillLib.h"
#include "drillengine/PNG.h"

namespace Resources {

struct Texture {
	RGBA8* pixels;
	U32 width;
	U32 height;
};

Texture load_texture(StrA path) {
	Texture result{};
	PNG::read_image(globalArena, &result.pixels, &result.width, &result.height, stracat(globalArena, "resources/textures/"a, path));
	if (!result.pixels) {
		abort("Failed to read image\n"a);
	}
	for (I32 i = 0; i < result.width * result.height; i++) {
		U8 r = result.pixels[i].r;
		result.pixels[i].r = result.pixels[i].b;
		result.pixels[i].b = r;
	}
	return result;
}

struct Sprite {
	Texture* tex;
	U32 x;
	U32 y;
	U32 width;
	U32 height;
	U32 animFrames;
};

Sprite rotate90(Sprite& s) {
	RGBA8* newData = globalArena.alloc_aligned<RGBA8>(s.width * s.height * s.animFrames, 4);
	for (U32 y = 0; y < s.height; y++) {
		for (U32 x = 0; x < s.width; x++) {
			newData[x * (s.height * s.animFrames) + s.height - y - 1] = s.tex->pixels[y * s.tex->width + x];
		}
	}
	Texture* tex = globalArena.alloc<Texture>(1);
	tex->pixels = newData;
	tex->width = s.height * 3;
	tex->height = s.width;
	return Sprite{ tex, 0, 0, s.height, s.width, s.animFrames };
}

Texture tileset;
struct {
	Sprite undef;
	Sprite grass;
	Sprite grass_blades;
	Sprite grassIron;
	Sprite grassCopper;
	Sprite grassFlowers;
	Sprite grassMisc;
	Sprite sand;
	Sprite beach;
	Sprite sandxgrass;
	Sprite water;
	Sprite mountain;
	Sprite oil;
	Sprite assemblerSmall;
	Sprite assemblerLarge;
	Sprite hive;
	Sprite hiveLarge;
	Sprite splitter;
	Sprite junction;
	Sprite merger;
	Sprite beeFly;
	Sprite beeMine;
	Sprite beeCarry;
	struct {
		Sprite downToUp;
		Sprite downToRight;
		Sprite downToLeft;
		Sprite leftToRight;
		Sprite leftToUp;
		Sprite leftToDown;
		Sprite rightToLeft;
		Sprite rightToUp;
		Sprite rightToDown;
		Sprite upToDown;
		Sprite upToLeft;
		Sprite upToRight;
	} belt;
	struct {
		Sprite inElevator;
		Sprite inChute;
		Sprite outLeft;
		Sprite outDown;
		Sprite outRight;
		Sprite outUp;
	} via;
	Sprite num[10];
	Sprite letters[26];
	struct {
		Sprite ironOre;
		Sprite copperOre;
		Sprite gull;
		Sprite copperCable;
		Sprite ironPlate;
		Sprite greenCircuit;
		Sprite camera;
		Sprite feather;
		Sprite gear;
		Sprite nuclearHeart;
		Sprite uranium;
		Sprite kittyCat;
		Sprite pollen;
		Sprite honey;
		Sprite lemonJuice;
		Sprite cyberGull;
	} item;
	struct {
		Sprite downOff;
		Sprite downOn;
		Sprite upOff;
		Sprite upOn;
		Sprite leftOff;
		Sprite leftOn;
		Sprite rightOff;
		Sprite rightOn;
	} assembler;
	struct {
		Sprite belt;
		Sprite assembler;
		Sprite hive;
		Sprite furnace;
		Sprite bigAssembler;
		Sprite bee;
		Sprite bigHive;
		Sprite splitter;
		Sprite junction;
		Sprite viaUp;
		Sprite viaDown;
		Sprite viaOut;
		Sprite camera;
	} icon;
	struct {
		Sprite full;
		Sprite top;
		Sprite right;
		Sprite topRightBottem;
		Sprite topRight;
	} rock;
	struct {
		Sprite downOff;
		Sprite downOn;
		Sprite upOff;
		Sprite upOn;
		Sprite leftOff;
		Sprite leftOn;
		Sprite rightOff;
		Sprite rightOn;
	} bigAssembler;
	Sprite furnace;
	Sprite furnaceOn;
	Sprite camera;
	Sprite dockSegment;
	Sprite ship;
} tile;
struct {
	Texture assembler;
	Texture assemblerBig;
	Texture bee;
	Texture camLens;
	Texture camera;
	Texture chute;
	Texture circuit;
	Texture conveyor;
	Texture copperOre;
	Texture copperWire;
	Texture cyberSeagull;
	Texture elevator;
	Texture feather;
	Texture furnace;
	Texture gear;
	Texture hive;
	Texture hiveBig;
	Texture honey;
	Texture ironOre;
	Texture ironPlate;
	Texture landing;
	Texture pollen;
	Texture powerCore;
	Texture seagull;
	Texture splitter;
	Texture uranium;
} tooltip;
Texture tutorial[9];
Texture controlsOverlay;
Texture winMessage;



void load() {
	tileset = load_texture("tileset.png"a);
	tile.undef = Sprite{ &tileset, 0, 0, 16, 16, 1 };
	tile.grass = Sprite{ &tileset, 15 * 16, 16 * 3, 16, 16, 6 };
	tile.grassIron = Sprite{ &tileset, 15 * 16, 32, 16, 16, 4 };	// i marked them as animation frames, but they're resource richness, really
	tile.grassCopper = Sprite{ &tileset, 15 * 16, 16, 16, 16, 4 };
	tile.grassFlowers = Sprite{ &tileset, 15 * 16, 0, 16, 16, 4 };
	tile.grassMisc = Sprite{ &tileset, 15 * 16, 16 * 6, 16, 16, 4 };
	tile.sand = Sprite{ &tileset, 15 * 16, 16 * 5, 16, 16, 4 };
	tile.beach = Sprite{ &tileset, 15 * 16, 16 * 4, 16, 16, 2 };
	tile.water = Sprite{ &tileset, 0, 2 * 16, 16, 16, 1 };
	tile.mountain = Sprite{ &tileset, 11 * 16, 13 * 16, 16, 16, 1 };
	tile.oil = Sprite{ &tileset,3 * 16, 5 * 16, 16, 16, 2 };
	tile.assemblerSmall = Sprite{ &tileset, 16, 48, 16, 16, 1 };
	tile.assemblerLarge = Sprite{ &tileset, 0, 96, 32, 32, 1 };
	tile.hive = Sprite{ &tileset, 16, 32, 16, 16, 1 };
	tile.hiveLarge = Sprite{ &tileset, 0, 64, 32, 32, 1 };
	tile.splitter = Sprite{ &tileset, 3 * 16, 6 * 16, 16, 16, 1 };
	tile.junction = Sprite{ &tileset, 3 * 16, 7 * 16, 16, 16, 1 };
	tile.merger = Sprite{ &tileset, 32, 64, 16, 16, 1 };
	tile.beeFly = Sprite{ &tileset, 0, 160, 16, 16, 4 };
	tile.beeMine = Sprite{ &tileset, 0, 144, 16, 16, 4 };
	tile.beeCarry = Sprite{ &tileset, 0, 176, 16, 16, 4 };
	tile.belt.leftToRight = Sprite{ &tileset, 64, 0, 16, 16, 3 };
	tile.belt.rightToLeft = Sprite{ &tileset, 64, 160, 16, 16, 3 };
	tile.belt.downToUp = Sprite{ &tileset, 64, 16, 16, 16, 3 };
	tile.belt.upToDown = Sprite{ &tileset, 64, 176, 16, 16, 3 };
	tile.belt.downToRight = Sprite{ &tileset, 64, 32, 16, 16, 3 };
	tile.belt.downToLeft = Sprite{ &tileset, 64, 64, 16, 16, 3 };
	tile.belt.leftToUp = Sprite{ &tileset, 64, 80, 16, 16, 3 };
	tile.belt.leftToDown = Sprite{ &tileset, 64, 96, 16, 16, 3 };
	tile.belt.rightToUp = Sprite{ &tileset, 64, 48, 16, 16, 3 };
	tile.belt.rightToDown = Sprite{ &tileset, 64, 128, 16, 16, 3 };
	tile.belt.upToLeft = Sprite{ &tileset, 64, 112, 16, 16, 3 };
	tile.belt.upToRight = Sprite{ &tileset, 64, 144, 16, 16, 3 };
	tile.via.inElevator = Sprite{ &tileset, 16 * 16, 17 * 16, 16, 16, 1 };
	tile.via.inChute = Sprite{ &tileset, 15 * 16, 17 * 16, 16, 16, 1 };
	tile.via.outLeft = Sprite{ &tileset, 256, 112, 16, 16, 3 };
	tile.via.outDown = Sprite{ &tileset, 256, 128, 16, 16, 3 };
	tile.via.outRight = Sprite{ &tileset, 256, 144, 16, 16, 3 };
	tile.via.outUp = Sprite{ &tileset, 256, 160, 16, 16, 3 };
	for (U32 i = 0; i < 10; i++) {
		tile.num[i] = Sprite{&tileset, 16*i, 12*16, 16, 16, 1};
	}

	//Starts at 23 tiles down, and 9 tiles across, having to mod by 9 to not go to far right
	for (U32 i = 0; i < 26; i++) {
		tile.letters[i] = Sprite{ &tileset, 16 * (i % 9), 23 * 16 + (i / 9) * 16, 16, 16, 1 };
	}

	tile.item.ironOre = Sprite{ &tileset, 0, 13 * 16, 16, 16, 1 };
	tile.item.copperOre = Sprite{ &tileset, 16, 13 * 16, 16, 16, 1 };
	tile.item.gull = Sprite{ &tileset, 2*16, 13 * 16, 16, 16, 1 };
	tile.item.copperCable = Sprite{ &tileset, 3 * 16, 13 * 16, 16, 16, 1 };
	tile.item.ironPlate = Sprite{ &tileset, 4 * 16, 13 * 16, 16, 16, 1 };
	tile.item.greenCircuit = Sprite{ &tileset, 5 * 16, 13 * 16, 16, 16, 1 };
	tile.item.camera = Sprite{ &tileset, 6 * 16, 13 * 16, 16, 16, 1 };
	tile.item.feather = Sprite{ &tileset, 7 * 16, 13 * 16, 16, 16, 1 };
	tile.item.gear = Sprite{ &tileset, 8 * 16, 13 * 16, 16, 16, 1 };
	tile.item.nuclearHeart = Sprite{ &tileset, 9 * 16, 13 * 16, 16, 16, 1 };
	tile.item.uranium = Sprite{ &tileset, 10 * 16, 13 * 16, 16, 16, 1 };
	tile.item.kittyCat = Sprite{ &tileset, 11 * 16, 13 * 16, 16, 16, 1 };
	tile.item.pollen = Sprite{ &tileset, 12 * 16, 13 * 16, 16, 16, 1 };
	tile.item.honey = Sprite{ &tileset, 13 * 16, 13 * 16, 16, 16, 1 };
	tile.item.lemonJuice = Sprite{ &tileset, 14 * 16, 13 * 16, 16, 16, 1 };
	tile.item.cyberGull = Sprite{ &tileset, 15 * 16, 13 * 16, 16, 16, 1 };

	tile.icon.belt = Sprite{ &tileset, 0 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.assembler = Sprite{ &tileset, 1 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.hive = Sprite{ &tileset, 2 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.furnace = Sprite{ &tileset, 3 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.bigAssembler = Sprite{ &tileset, 4 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.bee = Sprite{ &tileset, 5 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.bigHive = Sprite{ &tileset, 6 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.splitter = Sprite{ &tileset, 7 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.junction = Sprite{ &tileset, 8 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.viaDown = Sprite{ &tileset, 9 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.viaUp = Sprite{ &tileset, 10 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.viaOut = Sprite{ &tileset, 11 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.camera = Sprite{ &tileset, 12 * 16, 14 * 16, 16, 16, 1 };

	tile.rock.topRight = Sprite{ &tileset, 16 * 10, 11 * 16, 16, 16, 1 };
	tile.rock.top = Sprite{ &tileset, 16 * 11, 11 * 16, 16, 16, 1 };
	tile.rock.right = Sprite{ &tileset, 16 * 10, 12 * 16, 16, 16, 1 };
	tile.rock.full = Sprite{ &tileset, 16 * 11, 12 * 16, 16, 16, 1 };
	tile.rock.topRightBottem = Sprite{ &tileset, 16 * 12, 12 * 16, 16, 16, 1 };

	tile.assembler.downOff = Sprite{ &tileset, 128, 112, 32, 32, 1 };
	tile.assembler.downOn = Sprite{ &tileset, 160, 112, 32, 32, 1 };
	tile.assembler.upOff = Sprite{ &tileset, 192, 112, 32, 32, 1 };
	tile.assembler.upOn = Sprite{ &tileset, 224, 112, 32, 32, 1 };
	tile.assembler.leftOff = Sprite{ &tileset, 128, 144, 32, 32, 1 };
	tile.assembler.leftOn = Sprite{ &tileset, 160, 144, 32, 32, 1 };
	tile.assembler.rightOff = Sprite{ &tileset, 192, 144, 32, 32, 1 };
	tile.assembler.rightOn = Sprite{ &tileset, 224, 144, 32, 32, 1 };

	tile.bigAssembler.downOff = Sprite{ &tileset, 0, 240, 48, 32, 3 };
	tile.bigAssembler.downOn = Sprite{ &tileset, 0, 272, 48, 32, 3 };
	tile.bigAssembler.upOff = Sprite{ &tileset, 0, 304, 48, 32, 3 };
	tile.bigAssembler.upOn = Sprite{ &tileset, 0, 336, 48, 32, 3 };
	tile.bigAssembler.leftOff = Sprite{ &tileset, 144, 240, 32, 48, 3 };
	tile.bigAssembler.leftOn = Sprite{ &tileset, 144, 288, 32, 48, 3 };
	tile.bigAssembler.rightOff = Sprite{ &tileset, 144, 336, 32, 48, 3 };
	tile.bigAssembler.rightOn = Sprite{ &tileset, 144, 384, 32, 48, 3 };
	tile.furnace = Sprite{ &tileset, 17 * 16, 11 * 16, 16, 32, 1 };
	tile.furnaceOn = Sprite{ &tileset, 18 * 16, 11 * 16, 16, 32, 1 };

	tile.camera = Sprite{ &tileset, 17 * 16, 17 * 16, 16, 16, 1 };

	tile.dockSegment = Sprite{ &tileset, 240, 224, 16, 48, 1 };
	tile.ship = Sprite{ &tileset, 256, 208, 48, 64, 1 };

	tooltip.assembler = load_texture("tooltip_assembler.png"a);
	tooltip.assemblerBig = load_texture("tooltip_assembler_big.png"a);
	tooltip.bee = load_texture("tooltip_bee.png"a);
	tooltip.camLens = load_texture("tooltip_cam_lens.png"a);
	tooltip.camera = load_texture("tooltip_camera.png"a);
	tooltip.chute = load_texture("tooltip_chute.png"a);
	tooltip.circuit = load_texture("tooltip_circuit.png"a);
	tooltip.conveyor = load_texture("tooltip_conveyor.png"a);
	tooltip.copperOre = load_texture("tooltip_copper_ore.png"a);
	tooltip.copperWire = load_texture("tooltip_copper_wire.png"a);
	tooltip.cyberSeagull = load_texture("tooltip_cyber_seagull.png"a);
	tooltip.elevator = load_texture("tooltip_elevator.png"a);
	tooltip.feather = load_texture("tooltip_feather.png"a);
	tooltip.furnace = load_texture("tooltip_furnace.png"a);
	tooltip.gear = load_texture("tooltip_gear.png"a);
	tooltip.hive = load_texture("tooltip_hive.png"a);
	tooltip.hiveBig = load_texture("tooltip_hive_big.png"a);
	tooltip.honey = load_texture("tooltip_honey.png"a);
	tooltip.ironOre = load_texture("tooltip_iron_ore.png"a);
	tooltip.ironPlate = load_texture("tooltip_iron_plate.png"a);
	tooltip.landing = load_texture("tooltip_landing.png"a);
	tooltip.pollen = load_texture("tooltip_pollen.png"a);
	tooltip.powerCore = load_texture("tooltip_power_core.png"a);
	tooltip.seagull = load_texture("tooltip_seagull.png"a);
	tooltip.splitter = load_texture("tooltip_splitter.png"a);
	tooltip.uranium = load_texture("tooltip_uranium.png"a);

	for (U32 i = 0; i < ARRAY_COUNT(tutorial); i++) {
		tutorial[i] = load_texture(strafmt(globalArena, "tutorial_%.png"a, i));
	}
	controlsOverlay = load_texture("controls.png"a);
	winMessage = load_texture("win_message.png"a);
}

}