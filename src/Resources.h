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

Texture scrung;
Sprite scrungPart;

Texture tileset;
struct {
	Sprite undef;
	Sprite grass;
	Sprite grassIron;
	Sprite grassCopper;
	Sprite grassFlowers;
	Sprite sand;
	Sprite beach;
	Sprite water;
	Sprite mountain;
	Sprite oil;
	Sprite assemblerSmall;
	Sprite assemblerLarge;
	Sprite hive;
	Sprite hiveLarge;
	Sprite splitter;
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
	Sprite num[10];
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
	} icon;
	struct {
		Sprite full;
		Sprite top;
		Sprite left;
		Sprite right;
		Sprite topLeft;
		Sprite topRight;
	} rock;
	Sprite bigAssembler;
	Sprite bigAssemblerOn;
	Sprite furnace;
} tile;
Texture tutorial[7];

void load() {
	scrung = load_texture("scrung.png"a);
	scrungPart = Sprite{ &scrung, 128, 128, 128, 128, 1 };
	tileset = load_texture("tileset_v8.png"a);
	tile.undef = Sprite{ &tileset, 0, 0, 16, 16, 1 };
	tile.grass = Sprite{ &tileset, 16, 0, 16, 16, 1 };
	tile.grassIron = Sprite{ &tileset, 32, 0, 16, 16, 1 };
	tile.grassCopper = Sprite{ &tileset, 32, 16, 16, 16, 1 };
	tile.grassFlowers = Sprite{ &tileset, 32, 32, 16, 16, 1 };
	tile.sand = Sprite{ &tileset, 0, 16, 16, 16, 1 };
	tile.beach = Sprite{ &tileset, 16, 16, 16, 16, 1 };
	tile.water = Sprite{ &tileset, 0, 8 * 16, 16, 16, 2 };
	tile.mountain = Sprite{ &tileset, 11 * 16, 13 * 16, 16, 16, 1 };
	tile.oil = Sprite{ &tileset,3 * 16, 5 * 16, 16, 16, 2 };
	tile.assemblerSmall = Sprite{ &tileset, 16, 48, 16, 16, 1 };
	tile.assemblerLarge = Sprite{ &tileset, 0, 96, 32, 32, 1 };
	tile.hive = Sprite{ &tileset, 16, 32, 16, 16, 1 };
	tile.hiveLarge = Sprite{ &tileset, 0, 64, 32, 32, 1 };
	tile.splitter = Sprite{ &tileset, 32, 48, 16, 16, 1 };
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
	for (U32 i = 0; i < 10; i++) {
		tile.num[i] = Sprite{&tileset, 16*i, 12*16, 16, 16, 1};
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

	tile.icon.belt = Sprite{ &tileset, 0 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.assembler = Sprite{ &tileset, 1 * 16, 14 * 16, 16, 16, 1 };
	tile.icon.hive = Sprite{ &tileset, 2 * 16, 14 * 16, 16, 16, 1 };

	tile.rock.topLeft = Sprite{ &tileset, 16 * 10, 11 * 16, 16, 16, 1 };
	tile.rock.top = Sprite{ &tileset, 16 * 11, 11 * 16, 16, 16, 1 };
	tile.rock.topRight = Sprite{ &tileset, 16 * 12, 11 * 16, 16, 16, 1 };
	tile.rock.left = Sprite{ &tileset, 16 * 10, 12 * 16, 16, 16, 1 };
	tile.rock.full = Sprite{ &tileset, 16 * 11, 12 * 16, 16, 16, 1 };
	tile.rock.right = Sprite{ &tileset, 16 * 12, 12 * 16, 16, 16, 1 };

	tile.assembler.downOff = Sprite{ &tileset, 128, 112, 32, 32, 1 };
	tile.assembler.downOn = Sprite{ &tileset, 160, 112, 32, 32, 1 };
	tile.assembler.upOff = Sprite{ &tileset, 192, 112, 32, 32, 1 };
	tile.assembler.upOn = Sprite{ &tileset, 224, 112, 32, 32, 1 };
	tile.assembler.leftOff = Sprite{ &tileset, 128, 144, 32, 32, 1 };
	tile.assembler.leftOn = Sprite{ &tileset, 160, 144, 32, 32, 1 };
	tile.assembler.rightOff = Sprite{ &tileset, 192, 144, 32, 32, 1 };
	tile.assembler.rightOn = Sprite{ &tileset, 224, 144, 32, 32, 1 };

	tile.bigAssembler = Sprite{ &tileset, 12 * 16, 5 * 16, 48, 32, 1 };
	tile.bigAssemblerOn = Sprite{ &tileset, 12 * 16, 6 * 16, 48, 32, 1 };
	tile.furnace = Sprite{ &tileset, 15 * 16, 13 * 16, 16, 32, 1 };

	for (U32 i = 0; i < ARRAY_COUNT(tutorial); i++) {
		tutorial[i] = load_texture(strafmt(globalArena, "tutorial_%.png"a, i));
	}
}

}