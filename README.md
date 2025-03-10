# Enchanted Grotto

## Build & Run Instructions
Both systems take in a `game|test` argument:
- `game` builds and runs the full game
- `test` builds and runs tests for item serialization and potion comparison mechanics
- If no arguments are supplied, the scripts default to `game`

### Windows:
```bash
.\run [game|test]
```
Note that on some Windows configurations, CMake may place the game executable in the `/build/Debug` folder. If the script fails, the executable is most likely there and can be run directly.

### Linux/macOS:
Requires `freetype` to be installed. On macOS, use `brew install freetype`.
```bash
chmod +x run.sh
./run.sh [game|test]
```

# Milestone Features
## Milestone 1

### Required Elements:
1. Textured Geometry:
- All our game sprites and background environments are textured assets.
- The drawing order is implemented in `std::vector<Entity> RenderSystem::process_render_requests`, with documentation for how the different rendering layers work.The order of rendering is specified in the ENUM `RENDER_LAYER` in components.hpp.
- Rendering is done in render_system files, texture_paths are specified in render_system.hpp. They are also added to the ENUM `TEXTURE_ASSET_ID`, and utilized in world_init.cpp when added as a renderRequest.

2. Basic 2D transformations:
- The assets in our game are scaled to a certain size, and transform, scale and rotation can be applied to entities in the game, in void RenderSystem::drawTexturedMesh(Entity entity, const mat3& projection).

3. Key-frame/state interpolation:
- When entering the grotto, a fade in and out non-linear effect is played to transition.
- Fade fragment and vertex shaders are added to achieve the fading effect. From render_system.cpp, we get the current screen texture, and the darken_screen_factor which comes from our screen state. We pass in the texcoord from the vertex shader. 
- Our fragment shader gets the current screen’s in_color using texture(screen_texture, texcoord). If the darken_screen_factor is 0, we keep the fade completely transparent since its background texture is black. However, when the factor begins to increase above 0, this indicates that we are changing between biomes so we use linear interpolation with the mix function to interpolate between the screen’s current color and fully opaque black. 
- The alpha channel of the color returned by the shader is updated to match the darken_screen_factor to change the transparency of the fade.

4. Keyboard/mouse control:
- Keyboard control is done in world_systems.cpp, on_key() function. 
Movement with WASD:
- If multiple keys are pressed, the player will move in the direction of the most recently pressed key. If the key corresponding to the direction that the player is currently moving is released other keys are being released, the player will move in the direction of the second most recently pressed key and work its way back until no more keys are pressed.
Interaction with F:
- Item pickup: handled in world_systems.cpp,  handle_player_interaction() function
Enter grotto: handled in world_systems.cpp,  handle_entrance_interaction

5. Random/coded action:
- After an item is picked up, it respawns in 5-15 seconds at the same location.
- This is done in world_system.cpp, handle_item_respawn().

6. Well-defined game-space boundaries:
- The player is unable to walk past the bounds of the forest, or into the river unless on the bridge. The player cannot leave the grotto bounds, or go into the wall of the grotto
- The positions are currently hardcoded for each biome and can be found in biome_boundaries of common.cpp which define the position and scale of the boundary lines. When the world system is initialized, it will render the boundaries based on the current biome the player is in.

7. Simple collision detection & resolution (e.g. between square sprites):
- We are using AABB collision detection, with a terrain component that has fields collision_setting, width_ratio and height_ratio to draw a bounding box centered at the bottom middle of the entity.
0 Collision detection is handled in physics_system.cpp, bool collides() for each step of our physics system. Collision resolution is in WorldSystem::handle_collisions(), which prevents players from walking into entity bounding boxes

10. Test plan:
Uploaded to `/doc/test-plan.docx`

11. Bug report:
Uploaded to `/doc/bug-report.xlsx`

12. Demo video
https://www.youtube.com/watch?v=Tv99njp9Pmo

## Creative Elements: 
### Software Engineering: Reloadability (19)
Player inventory state is being persisted and reloaded in `item_system.cpp` through use of the `nlohmann/json` JSON library, as approved in this private Piazza post: https://piazza.com/class/m5kml0k6fxs2sz/post/203.

We are saving all Item-associated components in the player’s inventory, serializing it into the JSON format, and persisting it in `build/game_state.json` before the game closes through the `saveGameState` function. When the game relaunches, we read and restore entities & components to our registry based on that file from our `loadGameState` function.

To observe reloadability working directly in the game, refer to the test plan. The window title header of the game also reflects persisted inventory information.

### Quality & User Experience (UX): Basic Integrated Assets (24)
Assets are digitally-drawn with Procreate (a graphics editor app). They are added into the game as background and interactive objects (magical fruit, cocoa bean). 

## Additional Features:
### Potion System Backend
- The backend of our potion making system has been implemented to greatly facilitate our progress in M2.
- Recipes are defined in `common.hpp` and include a list of required ingredients, as well as a list of actions that must be followed in order to create the highest quality potion of that type.
- The `potion_system.hpp` file defines several functions that can be used to update the state of cauldron entities, and fetch a potion from a cauldron if needed.
- Every time a cauldron is updated, the system calculates the type and quality of the potion currently in the cauldron, updating its color and fading the cauldron’s current color towards the updated color.
- The exact method used to calculate the type and quality of a potion based on the given list of recipes is documented in the header function `PotionSystem#updatePotion`.


## Milestone 2

**IMPORTANT (Kevin): If you previously ran Milestone 1, there are breaking changes to our persistence structure for Milestone 2. You will need to remove the `/build/game_state.json` file for the game to run.**

## Build & Run Instructions
Both systems take in a `game|test` argument:
- `game` builds and runs the full game
- `test` builds and runs tests for item serialization and potion comparison mechanics
- If no arguments are supplied, the scripts default to `game`

### Windows:
```bash
.\run [game|test]
```

### Linux/macOS:
```bash
chmod +x run.sh
./run.sh [game|test]
```

### Required Elements:
1. Improved AI:
Enemy behavior is based on a decision tree with four states: Attack, Wander, Return, and Idle. In Attack, the enemy chases when the player gets too close. If the player gets away, the enemy switches to Wander and moves around randomly for a short time, then it enters Return, heading back to its spawn point. It stays in Idle until the player comes back into range.
2. Improved Sprite Animations:
The player now has a walking animation, so that as you move around the WASD keys, you can see the player animation. This is handled in WorldSystem::updatePlayerWalkAndAnimation(), and the frames for the assets are hand drawn.
3. Improved Gameplay and Assets:
We’ve included more background assets and sprites to support our game, such as the new desert, enemies and potion making menu.
Players are able to interact with the cauldron and follow a series of steps to make a potion, improving the gameplay.
4. Mesh-based Collisions
The bridge now uses mesh-based collision detection and resolution. The top of the bridge and the bottom of the bridge have defined mesh .obj files that are rendered onto the screen. When the player’s bounding box intersects with any triangle in the mesh, this will be detected as a collision in physics_system, using barycentric coordinates to see if a line of the box intersects with the triangle
5. Gameplay Tutorial
Uses RmlUi to display tutorial steps and updates when the player has completed the step
	
Welcome message and WASD movement - complete on pressing any WASD key
Fruit collection - complete on collecting 3 magical fruits or having 3 in inventory
Combat and inventory selection - complete on defeating the enemy with 3 fruits
Entering grotto - complete on interacting with grotto entrance
Open potion making menu - complete on interacting with cauldron
Heat knob - turn to H
Add 5 coffee beans and 3 fruits
Stirring - complete on picking up the ladle and successfully drawing a circle in cauldron to stir 3 times
Pick up empty bottle and left click on cauldron to fill up a potion
6. FPS Counter
Displayed in the title and updates every 500ms.
7. 2 minutes of non-repetitive gameplay
Gameplay is illustrated by our demo video and the test plan walkthrough.
8. Minimal lag
Demonstrated by FPS counter in the screen title.
9. Updated test plan
Uploaded to `/doc/test-plan.docx`
10. Updated bug list
Uploaded to `/doc/bug-report.xlsx`
11. Demo video
TODO: include YouTube link

## Creative Elements: 
### Software Engineering: External Integration (20) - RmlUi
We integrated RmlUi to facilitate integral game UIs such as the potion making menus, tutorials, and inventory bar. This integration will continue to remain in the game, supporting additional features like game HUDs and our inventory chest menu. RmlUi was approved by Kevin Huang in tutorial, based on our private Piazza post: https://piazza.com/class/m5kml0k6fxs2sz/post/271

RmlUi also packages FreeType for use in our game.

Relevant RmlUi code can be seen in `ui_system.cpp`, `ui_system.hpp`, `rmlui_render_interface.cpp`, and `rmlui_system_interface.cpp`. 

RmlUi GitHub: https://github.com/mikke89/RmlUi 

### User Interface (UI) & Input/Output (IO): Mouse Gestures (22) - RmlUi
Users can now use their mouse to interact with the potion making menu. Mouse gestures include clicking on the ladle to pick it up, then dragging it into the cauldron which will update the ladle sprite. Then users can move their mouse in a circular motion to “stir” the cauldron. Users can also interact with the heat knob with their mouse by left clicking and dragging the knob. Users can also click on the empty potion bottle and drag it over the cauldron to bottle a potion.


