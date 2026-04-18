#pragma once
#include "Win32.h"
#include "Resources.h"
#include "Graphics.h"
#include "TileSpace.h" 

namespace Cyber5eagull {

void keyboard_callback(Win32::Key key, Win32::ButtonState state) {
}
void mouse_callback(Win32::MouseButton button, Win32::MouseValue state) {
}

void render() {
	memset(Win32::framebuffer, 0, Win32::framebufferWidth * Win32::framebufferHeight * sizeof(RGBA8));
	F32 time = fractf64(current_time_seconds());
	Graphics::blit_texture(Resources::scrung, 0, I32((sinf32(time) * 0.5F + 0.5F) * 100.0F), 2);
}

U32 run_cyber5eagull() {
	timeBeginPeriod(1);
	if (!Win32::init(1920 / 2, 1080 / 2, keyboard_callback, mouse_callback)) {
		abort("Window init failed"a);
	}
	
	// init
	World::set_tile_space(V2U32{ 0, 0 }, V2U32{ 32, 32 });
	Resources::load();
	Win32::show_window();


	while (!Win32::windowShouldClose) {
		Win32::poll_events();
		render();
		InvalidateRect(Win32::window, NULL, FALSE);
		UpdateWindow(Win32::window);
	}

	timeEndPeriod(1);
	return 0;
}

}
