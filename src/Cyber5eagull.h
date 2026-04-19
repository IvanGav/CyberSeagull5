#pragma once
#include "Win32.h"
#include "Resources.h"
#include "Graphics.h"
#include "World.h"
#include "Bee.h"
#include "BeeTasks.h"
#include "TileSpace.h"
#include "Inventory.h"
#include "Factory.h"

namespace Cyber5eagull {

static constexpr I32 DEFAULT_WORLD_TILE_SCALE = 4;
static constexpr I32 MIN_WORLD_TILE_SCALE = 1;
static constexpr I32 MAX_WORLD_TILE_SCALE = 200;
static constexpr F32 CAMERA_EDGE_SCROLL_PIXELS = 20.0F;
static constexpr F32 CAMERA_SCROLL_SPEED = 500.0F;
static constexpr F32 DEMO_BEE_SPEED = 1.0F;

F64 lastFrameTime = 0.0;
F32 dt = 0.0F;

V2F camera{};
I32 worldTileScale = DEFAULT_WORLD_TILE_SCALE;
V2U32 hiveTile{};
V2U32 selectedTile{};
B32 hasSelectedTile = B32_FALSE;
B32 placingConveyor = B32_FALSE;
Direction2 lastConveyorInputSide;
Direction2 lastConveyorOutputSide;
Factory::MachineHandle lastConveyor;
Bee::Bee demoBee{};

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

V2F32 world_to_screen(V2F32 worldPosition) {
	return worldPosition * world_tile_pixels_f32() - camera;
}

V2I world_to_screen_px(V2F32 worldPosition) {
	V2F screen = world_to_screen(worldPosition);
	return V2I{ I32(floorf32(screen.x)), I32(floorf32(screen.y)) };
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

void send_bee_to_tile(V2U32 tile) {
	selectedTile = tile;
	hasSelectedTile = B32_TRUE;
	demoBee.assign_task(BeeTasks::make_generic_task(tile, 0.0F, B32_TRUE, B32_FALSE));
}

void keyboard_callback(Win32::Key key, Win32::ButtonState state) {
}

void mouse_callback(Win32::MouseButton button, Win32::MouseValue state) {
	if (button == Win32::MOUSE_BUTTON_WHEEL) {
		I32 zoomDelta = state.scroll > 0.0F ? 1 : (state.scroll < 0.0F ? -1 : 0);
		if (zoomDelta != 0) {
			zoom_camera_at_screen(zoomDelta, Win32::get_mouse());
		}
		return;
	}

	if (button == Win32::MOUSE_BUTTON_LEFT && state.state == Win32::BUTTON_STATE_DOWN) {
		V2U32 clickedTile{};
		if (mouse_to_tile(&clickedTile)) {
			if (Win32::keyboardState[Win32::KEY_CTRL]) {
				lastConveyorInputSide = DIRECTION2_LEFT;
				lastConveyorOutputSide = DIRECTION2_RIGHT;
				Factory::MachineDef belt = Factory::get_belt(lastConveyorInputSide, lastConveyorOutputSide);
				lastConveyor = Factory::try_place_machine(clickedTile, belt);
				placingConveyor = true;
			} else {
				send_bee_to_tile(clickedTile);
			}
		}
	} else if (button == Win32::MOUSE_BUTTON_LEFT && state.state == Win32::BUTTON_STATE_UP) {
		placingConveyor = false;
	}
	
	if (button == Win32::MOUSE_BUTTON_RIGHT && state.state == Win32::BUTTON_STATE_DOWN) {
		hasSelectedTile = B32_FALSE;
		demoBee.cancel_task();
	}
}

void render_tile_marker(V2U32 tile) {
	V2F32 screenTopLeft = world_to_screen(TileSpace::tile_to_world(tile));
	Graphics::blit_sprite_cutout(Resources::tile.splitter, I32(roundf32(screenTopLeft.x)), I32(roundf32(screenTopLeft.y)), worldTileScale, 0);
}

void render_hive() {
	V2F32 screenTopLeft = world_to_screen(TileSpace::tile_to_world(hiveTile));
	Graphics::blit_sprite_cutout(Resources::tile.hive, I32(roundf32(screenTopLeft.x)), I32(roundf32(screenTopLeft.y)), worldTileScale, 0);
}

void render_bee(F64 frameTimeSeconds) {
	Resources::Sprite& beeSprite = demoBee.state == BeeTasks::State::STATE_WORKING ? Resources::tile.beeMine : Resources::tile.beeFly;
	F32 animTurns = fractf64(frameTimeSeconds * 6.0);
	U32 animFrame = U32(animTurns * F32(beeSprite.animFrames)) % beeSprite.animFrames;
	V2F32 beeScreenCenter = world_to_screen(demoBee.position);
	V2F32 beeScreenTopLeft = beeScreenCenter - V2F32{ F32(beeSprite.width * worldTileScale) * 0.5F, F32(beeSprite.height * worldTileScale) * 0.5F };
	Graphics::blit_sprite_cutout(beeSprite, I32(roundf32(beeScreenTopLeft.x)), I32(roundf32(beeScreenTopLeft.y)), worldTileScale, animFrame);
}

void update() {
	if (Win32::keyboardState[Win32::KEY_CTRL] && Win32::mouseButtonState[Win32::MOUSE_BUTTON_RIGHT]) {
		V2U clickedTile{};
		if (mouse_to_tile(&clickedTile)) {
			Factory::remove_machine(clickedTile);
		}
	}
	if (placingConveyor) {
		if (Win32::keyboardState[Win32::KEY_CTRL] && lastConveyor.get()) {
			V2U clickedTile{};
			if (mouse_to_tile(&clickedTile)) {
				U32 prevId;
				if (World::get_tile(&prevId, clickedTile.x, clickedTile.y) == World::TILE_GRASS && prevId == World::MACHINE_NULL_ID) {
					Factory::Machine* mach = lastConveyor.machine;
					V2U machPos = mach->pos;
					Direction2 newDirection = DIRECTION2_INVALID;
					if (clickedTile == V2U{ machPos.x + 1, machPos.y }) {
						newDirection = DIRECTION2_RIGHT;
					} else if (clickedTile == V2U{ machPos.x - 1, machPos.y }) {
						newDirection = DIRECTION2_LEFT;
					} else if (clickedTile == V2U{ machPos.x, machPos.y + 1 }) {
						newDirection = DIRECTION2_BACK;
					} else if (clickedTile == V2U{ machPos.x, machPos.y - 1 }) {
						newDirection = DIRECTION2_FRONT;
					}
					if (newDirection != DIRECTION2_INVALID && newDirection != lastConveyorInputSide) {
						Factory::remove_machine(mach);
						Factory::MachineDef belt = Factory::get_belt(lastConveyorInputSide, newDirection);
						Factory::try_place_machine(machPos, belt);
						belt = Factory::get_belt(DIRECTION2_OPPOSITE[newDirection], newDirection);
						lastConveyorInputSide = DIRECTION2_OPPOSITE[newDirection];
						lastConveyorOutputSide = newDirection;
						lastConveyor = Factory::try_place_machine(clickedTile, belt);
					}
				}
			}
		} else {
			placingConveyor = false;
		}
	}
}

void render() {
	F64 currentFrameTime = current_time_seconds();
	dt = min(F32(currentFrameTime - lastFrameTime), 0.1F);
	V2F mouse = Win32::get_mouse();
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
	clamp_camera();
	demoBee.update(dt);

	memset(Win32::framebuffer, 0, Win32::framebufferWidth * Win32::framebufferHeight * sizeof(RGBA8));
	World::render(camera, worldTileScale);
	Factory::render(worldTileScale);
	render_hive();
	if (hasSelectedTile) {
		render_tile_marker(selectedTile);
	}
	render_bee(currentFrameTime);
	Inventory::draw_inv();
	lastFrameTime = currentFrameTime;
}

U32 run_cyber5eagull() {
	timeBeginPeriod(1);
	if (!Win32::init(1920 / 2, 1080 / 2, keyboard_callback, mouse_callback)) {
		abort("Window init failed"a);
	}
	lastFrameTime = current_time_seconds();
	
	Resources::load();
	World::init(V2U{ 40, 40 });
	Factory::init();
	worldTileScale = DEFAULT_WORLD_TILE_SCALE;
	hiveTile = V2U32{ World::size.x / 2, World::size.y / 2 };
	demoBee = Bee::Bee{ hiveTile, DEMO_BEE_SPEED };
	center_camera_on_tile(hiveTile);
	Win32::show_window();
	Inventory::init_inv(4);
	Inventory::inv[3] = 1969;

	while (!Win32::windowShouldClose) {
		Inventory::inv[0] = (U32)(camera.x * 10);
		Inventory::inv[1] = (U32)(camera.y * 10);
		Inventory::inv[2] = (U32)(dt * 10000);
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
