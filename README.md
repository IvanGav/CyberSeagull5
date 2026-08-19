# Cyber5eagull

Made for Chico ACM-W Hackathon Spring 2026. By team GULLGAME.

## How to run?

It only runs on Windows since we directly use Win32 api. Open the solution in VS and click "run".
- If you see an error on first launch, check a box to ignore it; it's all good and not a real error.

You can also download the release (executable) from github.

## How to play?

Goal:
- Assemble 20 cyber seagulls and send them away
- **The factory must grow**

Controls:
- Left Click on a resource to start harvesting it (they're finite!)
	- Also works on Beach and Belt tiles!
- Right Click to cancel a command
- Shift + Left Drag OR Middle Drag to pan camera
    - Or just move the mouse close to the edge of the screen!
- Scroll Wheel to zoom in/out
- Space to see hive range
	- Can only place new hives near old hives
- Tab for build menu
	- With a selected building, Left Click/Drag to build
	- With a selected building, Right Drag to remove (gets refunded) (if doesn't work, select a different building)
	- With a selected building, Escape to deselect (or Q on empty tile)
- Right Click on an assembler to change recipes (Left Click to confirm)
- Left Click on an Inventory Resource (to the left) to select it. After that, Left Click on a Belt to order a bee to place it down.
    - The bee will keep placing that item down until you tell it to stop!
- Q to pick the currently selected building
- R to rotate a building that you're about to place
    - Shift + R to rotate in CCW direction
- Debug/Creative
	- CapsLock + Tab for creative menu
	- CapsLock + E for 50 of each item
	- CapsLock + R to regenerate the world

## Other

Locations of specific things in the source code:
- Item recipes in `Recipe.h`
- Textures in `Resources.h`
- Build costs in `BeeDemo.h` at line 200-ish
- Build menu items in `EditorInteractions.h` - `BuildMenuEntry buildMenuEntries[]`
- Creating tile tasks in `BeeDemo.h` - `make_task_for_tile`
- Resources left on a tile in `BeeDemo.h` - `U8 *Remaining[TerrainGen::MAX_WORLD_MAP_TILES]{};`