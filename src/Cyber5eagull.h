#pragma once
#include "Win32.h"
#include "Resources.h"
#include "Graphics.h"
#include "World.h"
#include "TileSpace.h"
#include "Inventory.h"
#include "Factory.h"
#include "BeeDemo.h"
#include "CreativeToolkit.h"
#include "SelectUI.h"
#include "EditorInteraction.h"
#include "Recipe.h"

namespace Cyber5eagull {

static constexpr U32 WORLD_WIDTH = 64u;
static constexpr U32 WORLD_HEIGHT = 64u;
static constexpr U32 START_HIVE_SHORE_OFFSET_X = 8u;
static constexpr I32 DEFAULT_WORLD_TILE_SCALE = 4;
static constexpr I32 MIN_WORLD_TILE_SCALE = 1;
static constexpr I32 MAX_WORLD_TILE_SCALE = 200;
static constexpr F32 CAMERA_EDGE_SCROLL_PIXELS = 20.0F;
static constexpr F32 CAMERA_SCROLL_SPEED = 500.0F;
static constexpr F32 SHIFT_SCROLL_PAN_TILES = 2.5F;

F64 lastFrameTime = 0.0;
F32 dt = 0.0F;

V2F camera{};
I32 worldTileScale = DEFAULT_WORLD_TILE_SCALE;
V2U32 hiveTile{};

I32 world_tile_pixels() {
	return 16 * worldTileScale;
}

F32 world_tile_pixels_f32() {
	return F32(world_tile_pixels());
}

void clamp_camera() {
	F32 maxCameraX = max(F32(I32(World::size.x) * world_tile_pixels() - Win32::framebufferWidth), 0.0F);
	F32 maxCameraY = max(F32(I32(World::size.y) * world_tile_pixels() - Win32::framebufferHeight), 0.0F);
	camera.x = clamp(camera.x, 0.0F, maxCameraX);
	camera.y = clamp(camera.y, 0.0F, maxCameraY);
}

V2F32 screen_to_world(V2F32 screenPosition) {
	return (screenPosition + camera) / world_tile_pixels_f32();
}

V2I tile_to_screen_px(V2U tile) {
	return V2I{ I32(tile.x) * world_tile_pixels() - I32(floorf32(camera.x)), I32(tile.y) * world_tile_pixels() - I32(floorf32(camera.y)) };
}

void center_camera_on_tile(V2U32 tile) {
	V2F32 tileCenterPixels = TileSpace::tile_to_world_center(tile) * world_tile_pixels_f32();
	camera = tileCenterPixels - V2F32{ F32(Win32::framebufferWidth) * 0.5F, F32(Win32::framebufferHeight) * 0.5F };
	clamp_camera();
}

void zoom_camera_at_screen(I32 zoomDelta, V2F32 screenAnchor) {
	I32 newScale = clamp(worldTileScale + zoomDelta, MIN_WORLD_TILE_SCALE, MAX_WORLD_TILE_SCALE);
	if (newScale == worldTileScale) {
		return;
	}

	V2F32 worldAnchor = screen_to_world(screenAnchor);
	worldTileScale = newScale;
	camera = worldAnchor * world_tile_pixels_f32() - screenAnchor;
	clamp_camera();
}

void pan_camera_horizontally(I32 direction) {
	camera.x += F32(direction) * world_tile_pixels_f32() * SHIFT_SCROLL_PAN_TILES;
	clamp_camera();
}

B32 mouse_to_tile(V2U32* tileOut) {
	V2F32 mouse = Win32::get_mouse();
	V2F32 world = screen_to_world(mouse);
	I32 tileX = I32(floorf32(world.x));
	I32 tileY = I32(floorf32(world.y));
	if (tileX < 0 || tileY < 0 || tileX >= I32(World::size.x) || tileY >= I32(World::size.y)) {
		return B32_FALSE;
	}
	*tileOut = V2U32{ U32(tileX), U32(tileY) };
	return B32_TRUE;
}

void update() {
	F64 currentFrameTime = current_time_seconds();
	dt = min(F32(currentFrameTime - lastFrameTime), 0.1F);

	Factory::update(dt);
	World::beach_update(dt);
	BeeDemo::update(dt);
	EditorInteraction::update_drag_interactions();

	V2F mouse = Win32::get_mouse();
	if (!EditorInteraction::cameraDragActive) {
		if (mouse.x < CAMERA_EDGE_SCROLL_PIXELS) {
			camera.x -= dt * CAMERA_SCROLL_SPEED;
		}
		if (mouse.x > Win32::framebufferWidth - CAMERA_EDGE_SCROLL_PIXELS) {
			camera.x += dt * CAMERA_SCROLL_SPEED;
		}
		if (mouse.y < CAMERA_EDGE_SCROLL_PIXELS) {
			camera.y -= dt * CAMERA_SCROLL_SPEED;
		}
		if (mouse.y > Win32::framebufferHeight - CAMERA_EDGE_SCROLL_PIXELS) {
			camera.y += dt * CAMERA_SCROLL_SPEED;
		}
	}
	clamp_camera();
}

void render() {
	F64 currentFrameTime = current_time_seconds();
	memset(Win32::framebuffer, 0, Win32::framebufferWidth * Win32::framebufferHeight * sizeof(RGBA8));
	World::render(camera, worldTileScale);
	Factory::render(worldTileScale);
	if (Win32::keyboardState[Win32::KEY_H]) {
		BeeDemo::render_hive_ranges(camera, worldTileScale);
	}
	BeeDemo::render_task_markers(camera, worldTileScale);
	BeeDemo::render_conveyors(camera, worldTileScale, currentFrameTime);
	BeeDemo::render_hives(camera, worldTileScale);
	BeeDemo::render_bees(camera, worldTileScale, currentFrameTime);
	Inventory::draw_inv();
	CreativeToolkit::render_ui();
	//Graphics::box(100, 50, 350, 200, 4, RGBA8{ 250,250,250,255 }, RGBA8{ 50,250,50,255 });
	SelectUI::draw();
	lastFrameTime = currentFrameTime;
}

U32 run_cyber5eagull() {
	timeBeginPeriod(1);
	if (!Win32::init(U32(1920 / 2), U32(1080 / 2), EditorInteraction::keyboard_callback, EditorInteraction::mouse_callback)) {
		abort("Window init failed"a);
	}
	lastFrameTime = current_time_seconds();

	Resources::load();
	Recipe::init();
	SelectUI::debug_selections(); // TODO debug selections for now; don't need this later
	Inventory::init();
	World::init(V2U{ WORLD_WIDTH, WORLD_HEIGHT });
	Factory::init();
	worldTileScale = DEFAULT_WORLD_TILE_SCALE;
	hiveTile = V2U32{ min(START_HIVE_SHORE_OFFSET_X, World::size.x > 2u ? World::size.x - 2u : 0u), World::size.y / 2u };
	BeeDemo::init(hiveTile);
	center_camera_on_tile(hiveTile);
	Win32::show_window();

	while (!Win32::windowShouldClose) {
		swap(&frameArena, &lastFrameArena);
		frameArena.reset();
		Win32::poll_events();
		update();
		render();
		InvalidateRect(Win32::window, NULL, FALSE);
		UpdateWindow(Win32::window);
	}

	timeEndPeriod(1);
	return 0;
}

}
