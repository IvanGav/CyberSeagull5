#pragma once
#include "drillengine/WASAPIInterface.h"
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
#include "Sounds.h"
#include "AntSystem.h"

namespace Cyber5eagull {

static constexpr U32 WORLD_WIDTH = 64u;
static constexpr U32 WORLD_HEIGHT = 64u;
static constexpr U32 START_HIVE_SHORE_OFFSET_X = 8u;
static constexpr I32 DEFAULT_WORLD_TILE_SCALE = 4;
static constexpr I32 MIN_WORLD_TILE_SCALE = 1;
static constexpr I32 MAX_WORLD_TILE_SCALE = 200;
static constexpr F32 CAMERA_EDGE_SCROLL_PIXELS = 20.0F;
static constexpr F32 CAMERA_SCROLL_SPEED = 500.0F;
static constexpr F32 CAMERA_WASD_MOVE_SPEED = 1000.0F;
static constexpr F32 SHIFT_SCROLL_PAN_TILES = 2.5F;
static constexpr F32 MAX_OUT_OF_BOUNDS_VIEW_PERCENTAGE = 0.5F;

F64 lastFrameTime = 0.0;
F32 dt = 0.0F;

V2F camera{};
I32 worldTileScale = DEFAULT_WORLD_TILE_SCALE;
V2U32 hiveTile{};
U32 activeEditingLayer = 0;

U32 totalCyberSeagullsOnShip = 0;
const U32 TARGET_CYBERSEAGULL_COUNT = 20;
F32 shipOffset = 0.0F;
const F32 SHIP_SPEED = 5.0F;
B32 gameOver = B32_FALSE;

I32 world_tile_pixels() {
	return 16 * worldTileScale;
}

F32 world_tile_pixels_f32() {
	return F32(world_tile_pixels());
}

void clamp_camera() {
	F32 maxOutOfBoundsViewWidth = Win32::framebufferWidth * MAX_OUT_OF_BOUNDS_VIEW_PERCENTAGE;
	F32 maxOutOfBoundsViewHeight = Win32::framebufferHeight * MAX_OUT_OF_BOUNDS_VIEW_PERCENTAGE;
	F32 maxCameraX = max(F32(I32(World::size.x) * world_tile_pixels() - Win32::framebufferWidth) + maxOutOfBoundsViewWidth, -maxOutOfBoundsViewWidth);
	F32 maxCameraY = max(F32(I32(World::size.y) * world_tile_pixels() - Win32::framebufferHeight) + maxOutOfBoundsViewHeight, -maxOutOfBoundsViewHeight);
	camera.x = clamp(camera.x, -maxOutOfBoundsViewWidth, maxCameraX);
	camera.y = clamp(camera.y, -maxOutOfBoundsViewHeight, maxCameraY);
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
	AntSystem::update(dt);

	EditorInteraction::update_drag_interactions();

	if (totalCyberSeagullsOnShip >= TARGET_CYBERSEAGULL_COUNT) {
		gameOver = B32_TRUE;
		shipOffset += dt * SHIP_SPEED;
	}

	V2F mouse = Win32::get_mouse();
	if (!EditorInteraction::cameraDragActive) {
		if (mouse.x < CAMERA_EDGE_SCROLL_PIXELS) {
			camera.x -= dt * CAMERA_SCROLL_SPEED;
		}
		if (mouse.x > F32(Win32::framebufferWidth) - CAMERA_EDGE_SCROLL_PIXELS) {
			camera.x += dt * CAMERA_SCROLL_SPEED;
		}
		if (mouse.y < CAMERA_EDGE_SCROLL_PIXELS) {
			camera.y -= dt * CAMERA_SCROLL_SPEED;
		}
		if (mouse.y > F32(Win32::framebufferHeight) - CAMERA_EDGE_SCROLL_PIXELS) {
			camera.y += dt * CAMERA_SCROLL_SPEED;
		}
	}

	// Crative Toolkit keybind enable
	if(Win32::keyboardState[Win32::KEY_BACKTICK]){
		CreativeToolkit::tilesheetVisible = !CreativeToolkit::tilesheetVisible;
	}

	if (Win32::keyboardState[Win32::KEY_W]) {
		camera.y -= CAMERA_WASD_MOVE_SPEED * dt;
	}
	if (Win32::keyboardState[Win32::KEY_A]) {
		camera.x -= CAMERA_WASD_MOVE_SPEED * dt;
	}
	if (Win32::keyboardState[Win32::KEY_S]) {
		camera.y += CAMERA_WASD_MOVE_SPEED * dt;
	}
	if (Win32::keyboardState[Win32::KEY_D]) {
		camera.x += CAMERA_WASD_MOVE_SPEED * dt;
	}
	clamp_camera();
}

void display_tooltip(Resources::Texture& tooltip, I32 x, I32 y) {
	x = max(0, x - max(0, I32(x + tooltip.width * TOOLTIP_SCALE - Win32::framebufferWidth)));
	y = max(0, y - max(0, I32(y + tooltip.height * TOOLTIP_SCALE - Win32::framebufferHeight)));
	Graphics::blit_texture_cutout(tooltip, x, y, TOOLTIP_SCALE);
}


Resources::Texture* currentTooltip;
V2I tooltipTopLeft;

U32 tutorialIndex = 0;
U32 gameWinIndex = 0;
I32 tutorialScale = 3;

void render() {
	F64 currentFrameTime = current_time_seconds();
	memset(Win32::framebuffer, 0, Win32::framebufferWidth * Win32::framebufferHeight * sizeof(RGBA8));
	currentTooltip = nullptr;
	tooltipTopLeft = V2I{ -1, -1 };
	World::render(camera, worldTileScale);
	Factory::render(worldTileScale, activeEditingLayer);
	CreativeToolkit::render_world_preview(camera, worldTileScale, currentFrameTime);
	if (Win32::keyboardState[Win32::KEY_SPACE]) {
		BeeDemo::render_hive_ranges(camera, worldTileScale);
	}
	BeeDemo::render_task_markers(camera, worldTileScale);
	BeeDemo::render_hives(camera, worldTileScale);
	AntSystem::render_hills(camera, worldTileScale);
	BeeDemo::render_bees(camera, worldTileScale, currentFrameTime);
	AntSystem::render_ants(camera, worldTileScale, currentFrameTime);
	Factory::render_ui(worldTileScale);
	Inventory::draw_inv();
	EditorInteraction::render_item_build_menu();
	CreativeToolkit::render_ui();
	SelectUI::draw();
	if (currentTooltip) {
		V2F mouse = Win32::get_mouse();
		if (tooltipTopLeft.x == -1) {
			tooltipTopLeft = V2I{ I32(mouse.x) + 16, I32(mouse.y) + 16 };
		}
		display_tooltip(*currentTooltip, tooltipTopLeft.x, tooltipTopLeft.y);
	}
	if (tutorialIndex < ARRAY_COUNT(Resources::tutorial)) {
		Resources::Texture& tut = Resources::tutorial[tutorialIndex];
		Graphics::blit_texture_cutout(tut, max(0, (I32(Win32::framebufferWidth) - I32(tut.width) * tutorialScale) / 2), max(0, I32(Win32::framebufferHeight) - I32(tut.height) * tutorialScale), tutorialScale);
	}
	if (Cyber5eagull::gameOver && gameWinIndex < 1) {
		Resources::Texture& win = Resources::winMessage;
		Graphics::blit_texture_cutout(win, max(0, (I32(Win32::framebufferWidth) - I32(win.width) * tutorialScale) / 2), max(0, I32(Win32::framebufferHeight) - I32(win.height) * tutorialScale), tutorialScale);
	}
	if (Win32::keyboardState[Win32::KEY_C]) {
		Resources::Texture& ctrls = Resources::controlsOverlay;
		Graphics::blit_texture_cutout(ctrls, max(0, (I32(Win32::framebufferWidth) - I32(ctrls.width) * tutorialScale) / 2), 0, tutorialScale);
	}
	lastFrameTime = currentFrameTime;
}

void mouse_callback(Win32::MouseButton button, Win32::MouseValue state) {
	if (tutorialIndex < ARRAY_COUNT(Resources::tutorial) && state.state == Win32::BUTTON_STATE_UP) {
		Resources::Texture& tut = Resources::tutorial[tutorialIndex];
		F32 x = F32(max(0, (I32(Win32::framebufferWidth) - I32(tut.width) * tutorialScale) / 2));
		F32 y = F32(max(0, I32(Win32::framebufferHeight) - I32(tut.height) * tutorialScale));
		Rng2F32 tutorialBox{ x, y, x + tut.width * tutorialScale, y + tut.height * tutorialScale };
		if (tutorialBox.contains_point(Win32::get_mouse())) {
			tutorialIndex++;
		}
	}
	if (Cyber5eagull::gameOver && gameWinIndex < 1 && state.state == Win32::BUTTON_STATE_UP) {
		Resources::Texture& tut = Resources::tutorial[tutorialIndex];
		F32 x = F32(max(0, (I32(Win32::framebufferWidth) - I32(tut.width) * tutorialScale) / 2));
		F32 y = F32(max(0, I32(Win32::framebufferHeight) - I32(tut.height) * tutorialScale));
		Rng2F32 winBox{ x, y, x + tut.width * tutorialScale, y + tut.height * tutorialScale };
		if (winBox.contains_point(Win32::get_mouse())) {
			gameWinIndex++;
		}
	}
	EditorInteraction::mouse_callback(button, state);
}

HANDLE audioThread;
B32 audioThreadShouldShutdown;

void fill_audio_buffer(F32* buffer, U32 numSamples, U32 numChannels, F32 timeAmount) {
	Sounds::mix_into_buffer(buffer, numSamples, numChannels, timeAmount);
	Sounds::audioPlaybackTime += timeAmount;
}

DWORD WINAPI audio_thread_func(LPVOID) {
	WASAPIInterface::init_wasapi(fill_audio_buffer);
	while (!audioThreadShouldShutdown) {
		WASAPIInterface::do_audio();
	}
	return 0;
}

U32 run_cyber5eagull() {
	timeBeginPeriod(1);
	if (!Win32::init(U32(1920 / 2), U32(1080 / 2), EditorInteraction::keyboard_callback, mouse_callback)) {
		abort("Window init failed"a);
	}
	lastFrameTime = current_time_seconds();

	Resources::load();
	Sounds::load_sources();
	audioThread = CreateThread(NULL, 64 * KILOBYTE, audio_thread_func, NULL, 0, NULL);
	if (audioThread == NULL) {
		printf("Failed to create audio thread, code: %\n"a, Win32::ErrCode{ GetLastError() });
		return 1;
	}
	Recipe::init();
	//SelectUI::debug_selections(); // TODO debug selections for now; don't need this later
	Inventory::init();
	World::init(V2U{ WORLD_WIDTH, WORLD_HEIGHT });
	Factory::init();
	CreativeToolkit::init_ui();
	worldTileScale = DEFAULT_WORLD_TILE_SCALE;
	U32 startHiveX = World::size.x > 2u ? min(START_HIVE_SHORE_OFFSET_X, World::size.x - 2u) : 0u;
	hiveTile = V2U32{ startHiveX, World::size.y / 2u };
	BeeDemo::init(hiveTile);
	AntSystem::init(hiveTile);
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

	audioThreadShouldShutdown = true;
	if (WaitForSingleObject(audioThread, INFINITE) == WAIT_FAILED) {
		printf("Failed to join audio thread, code:  %\n"a, Win32::ErrCode{ GetLastError() });
	} else {
		CloseHandle(audioThread);
	}
	Win32::destroy();

	timeEndPeriod(1);
	return 0;
}

}
