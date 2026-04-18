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

Texture scrung;

void load() {
	scrung = load_texture("scrung.png"a);
}

}