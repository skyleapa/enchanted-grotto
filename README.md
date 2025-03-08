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

