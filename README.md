# Cyber5eagull

Made for Chico ACM-W Hackathon Spring 2026. By team GULLGAME.

## How to run?

It only runs on Windows since we directly use Win32 api. Open the solution in VS and click "run".
- If you see an error on first launch, check a box to ignore it; it's all good and not a real error.

You can also download the release from github (whenever it's posted).

## How to play?

Goal:
- Assemble a cyber seagull
- **The factory must grow**

Controls:
- Left Click on a resource to start harvesting it (they're finite!)
	- Also works on Beach and Belt tiles!
- Left Click to cancel a command
- Shift + Left Drag OR Middle Drag to pan camera (or just move the mouse close to the edge of the screen)
- Scroll Wheel to zoom in/out
- H to see hive range
	- Can only place new hives near old hives
- B/I for build menu
	- With a selected building, Left Click/Drag to build
	- With a selected building, Right Drag to remove (gets refunded) (if doesn't work, select a different building)
	- With a selected building, Escape to deselect
- Right Click on an assembler to change recipes (Left Click to confirm)
- Left Click on an Inventory Resource (to the left) to select it. After that, Left Click on a Belt to order a bee to place it down.
- Debug/Creative
	- Tilde for creative menu
	- Backslash for 50 of each item
	- Shift + R to regenerate the world

## TODO
- ~~Make beach tiles auto-renew (collect again after it's just collected, just like ore)~~
- ~~Fix the bug where you put a bee on an ore, then place a belt next~~
- Add junctions (Everything is hooked up, just make them unique in Factory.h search for `// TODO add junctions`)
- ~~Make all hives unkillable in survival~~ (maybe shouldn't make *all* unkillable? idk)
- Make ores away from spawn be more rich
- ~~Make beehives only placeable within current beehive range~~
- ~~Make belts free~~
- Fix furnace rendering
- ~~When deconstructing machines, refund items in them~~

When all of that's done, we will do a `Hackathon+` release.

## Other

Locations of specific things in the source code:
- Item recipes in `Recipe.h`
- Textures in `Resources.h`
- Build costs in `BeeDemo.h` at line 200-ish
- Build menu items in `EditorInteractions.h` - `BuildMenuEntry buildMenuEntries[]`
- Creating tile tasks in `BeeDemo.h` - `make_task_for_tile`
- Resources left on a tile in `BeeDemo.h` - `U8 *Remaining[TerrainGen::MAX_WORLD_MAP_TILES]{};`