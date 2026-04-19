#pragma once
#include "Win32.h"
#include "Resources.h"
#include "Graphics.h"
#include "World.h"
#include "Inventory.h"

namespace Cyber5eagull {

F64 lastFrameTime;
F32 dt;

V2F camera;

void keyboard_callback(Win32::Key key, Win32::ButtonState state) {
}
void mouse_callback(Win32::MouseButton button, Win32::MouseValue state) {
}

void render() {
	F64 currentFrameTime = current_time_seconds();
	dt = min(F32(currentFrameTime - lastFrameTime), 1.0F);
	V2F mouse = Win32::get_mouse();
	F32 camSpeed = 200.0F;
	if (mouse.x < 20.0F) {
		camera.x -= dt * camSpeed;
	}
	if (mouse.x > Win32::framebufferWidth -  20.0F) {
		camera.x += dt * camSpeed;
	}
	if (mouse.y < 20.0F) {
		camera.y -= dt * camSpeed;
	}
	if (mouse.y > Win32::framebufferHeight - 20.0F) {
		camera.y += dt * camSpeed;
	}

	memset(Win32::framebuffer, 0, Win32::framebufferWidth * Win32::framebufferHeight * sizeof(RGBA8));
	F32 time = fractf64(current_time_seconds());
	World::render(camera, 4);
	Graphics::blit_sprite_cutout(Resources::tile.beeFly, I32((cosf32(time) * 0.5F + 0.5F) * 100.0F) + 20, I32((sinf32(time) * 0.5F + 0.5F) * 100.0F) + 20, 4, U32(time * 32.0F) % Resources::tile.beeMine.animFrames);
	Graphics::blit_sprite(Resources::tile.grassCopper, I32(mouse.x), I32(mouse.y), 4, 0);
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
	Win32::show_window();
	Inventory::init();
	Inventory::inv[3] = 1969;


	while (!Win32::windowShouldClose) {
		Inventory::inv[0] = (U32)(camera.x * 10);
		Inventory::inv[1] = (U32)(camera.y * 10);
		Inventory::inv[2] = (U32)(dt * 10000);
		World::beach_update(dt);
		swap(&frameArena, &lastFrameArena);
		frameArena.reset();
		Win32::poll_events();
		render();
		InvalidateRect(Win32::window, NULL, FALSE);
		UpdateWindow(Win32::window);
	}

	timeEndPeriod(1);
	return 0;
}

}
