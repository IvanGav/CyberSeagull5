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
- To pan camera, any of these work!
	- Shift + Left Drag
	- Middle Mouse Drag
	- WASD
    - Move the mouse close to the edge of the screen
- Scroll Wheel to zoom in/out
- Space to see hive range
	- Can only place new hives near old hives
- Tab for build menu
	- With a selected building, Left Click/Drag to build
	- With a selected building, Right Drag to remove (gets refunded) (if doesn't work, try holding Right on grass and drag over the building you want to delete)
	- With a selected building, Escape to deselect (or Q on empty tile)
- Right Click on an assembler to change recipes (Left Click to confirm)
- Left Click on an Inventory Resource (to the left) to select it. After that, Left Click on a Belt to order a bee to place it down.
    - The bee will keep placing that item down until you tell it to stop!
- Q to pick the currently hovered-over building
- R to rotate a building that you're about to place
    - Shift + R to rotate in CCW direction
- Debug/Creative
	- CapsLock + Tab for creative menu
	- CapsLock + E for 50 of each item
	- CapsLock + R to regenerate the world

Items:
- You can mine `Iron Ore` (gray), `Copper Ore` (orange) or `Pollen` (when mining flowers)
- 3 `Pollen` get turned into 1 `Honey` when there are spare bees in the hive
- `Seagull`, `Feather` and `Uranium` can be obtained from the beach
- `Iron Ingot`, `Copper Wire`, `Gear`, `Circuit`, `Uranium Fuel Cell`, `Camera` and `Cyber Seagull` can be crafted

Buildings:
- Conveyor Belts - Transport items placed on them
    - Bees that mine ore, will automatically place the mined ore on adjacent belts
- Splitters - Accept items from any direction and distribute them equally into all directions
- Chutes (Down Arrow) - Accept items from any direction and send to the Output that's directly under the Chute (on the underground level)
- Elevator (Up Arrow) - Accept items from any direction and send to the Output that's directly above the Elevator (on the ground level)
- Output - Accepts itms from Chutes and Elevators that are directly above or under the Output
- Furnace - Smelts items
- Small Assembler - Assembles simple recipes
- Large Assembler - Aseembles advanced recipes
- Camera - Collect items from shore in a 5x4 range
- Small/Large Hive - Expand the range where bees can work

Recipes:
- Smelting
	- 2 `Iron Ore` smelts into 1 `Iron Ingot` over 7 seconds
	- 3 `Copper Ore` smelts into 1 `Copper Wire` over 4 seconds
- Simple Crafting
    - 5 `Iron Ingot` craft into 3 `Gear` over 2 seconds
	- 4 `Copper Wire` craft into 2 `Circuit` over 8 seconds
- Advanced Crafting
    - 5 `Iron Ingot`, 3 `Uranium` and 1 `Circuit` craft into 1 `Uranium Fuel Cell` over 20 seconds
	- 2 `Iron Ingot`, 8 `Gear` and 2 `Circuit` craft into 1 `Camera` over 15 seconds
	- 1 `Uranium Fuel Cell`, 2 `Camera` and 1 `Seagull` craft into 1 `Cyber Seagull` over 15 seconds

## Other

Locations of specific things in the source code:
- Item recipes in `Recipe.h`
- Textures in `Resources.h`
- Build costs in `BeeDemo.h` at line 200-ish
- Build menu items in `EditorInteractions.h` - `BuildMenuEntry buildMenuEntries[]`
- Creating tile tasks in `BeeDemo.h` - `make_task_for_tile`
- Resources left on a tile in `BeeDemo.h` - `U8 *Remaining[TerrainGen::MAX_WORLD_MAP_TILES]{};`