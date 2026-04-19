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
	Sprite assemblerSmall;
	Sprite assemblerLarge;
	Sprite hive;
	Sprite hiveLarge;
	Sprite splitter;
	Sprite merger;
	Sprite beeFly;
	Sprite beeMine;
	struct {
		Sprite right;
		Sprite left;
		Sprite up;
		Sprite down;
		Sprite bottomToRight;
		Sprite bottomToLeft;
		Sprite leftToUp;
		Sprite leftToDown;
		Sprite rightToUp;
		Sprite rightToDown;
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
	} item;
} tile;

void load() {
	scrung = load_texture("scrung.png"a);
	scrungPart = Sprite{ &scrung, 128, 128, 128, 128, 1 };
	tileset = load_texture("tileset_v3.png"a);
	tile.undef = Sprite{ &tileset, 0, 0, 16, 16, 1 };
	tile.grass = Sprite{ &tileset, 16, 0, 16, 16, 1 };
	tile.grassIron = Sprite{ &tileset, 32, 0, 16, 16, 1 };
	tile.grassCopper = Sprite{ &tileset, 32, 16, 16, 16, 1 };
	tile.grassFlowers = Sprite{ &tileset, 32, 32, 16, 16, 1 };
	tile.sand = Sprite{ &tileset, 0, 16, 16, 16, 1 };
	tile.beach = Sprite{ &tileset, 16, 16, 16, 16, 1 };
	tile.water = Sprite{ &tileset, 0, 32, 16, 16, 1 };
	tile.assemblerSmall = Sprite{ &tileset, 16, 48, 16, 16, 1 };
	tile.assemblerLarge = Sprite{ &tileset, 0, 96, 32, 32, 1 };
	tile.hive = Sprite{ &tileset, 16, 32, 16, 16, 1 };
	tile.hiveLarge = Sprite{ &tileset, 0, 64, 16, 16, 1 };
	tile.splitter = Sprite{ &tileset, 32, 48, 16, 16, 1 };
	tile.merger = Sprite{ &tileset, 32, 64, 16, 16, 1 };
	tile.beeFly = Sprite{ &tileset, 0, 160, 16, 16, 4 };
	tile.beeMine = Sprite{ &tileset, 0, 144, 16, 16, 4 };
	tile.belt.right = Sprite{ &tileset, 64, 0, 16, 16, 3 };
	tile.belt.left = Sprite{ &tileset, 64, 160, 16, 16, 3 };
	tile.belt.up = Sprite{ &tileset, 64, 32, 16, 16, 3 };
	tile.belt.down = Sprite{ &tileset, 64, 176, 16, 16, 3 };
	tile.belt.bottomToRight = Sprite{ &tileset, 64, 16, 16, 16, 3 };
	tile.belt.bottomToLeft = Sprite{ &tileset, 64, 64, 16, 16, 3 };
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
	tile.item.copperCable = Sprite{ &tileset, 3*16, 13 * 16, 16, 16, 1 };
	tile.item.ironPlate = Sprite{ &tileset, 4*16, 13 * 16, 16, 16, 1 };
}

}