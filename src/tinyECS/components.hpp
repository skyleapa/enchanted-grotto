#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_set>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{
	std::string name;
	int throw_distance = 150; // pixels
	float cooldown = 0.f; // defaults to 0, but when ammo is tossed, will have a 1000 ms cooldown
};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 10, 10 };
	vec2 previous_position = { 0, 0 };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug
{
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = 0;
	GLuint biome = 1; // default to forest
	bool is_switching_biome = false;
	GLuint switching_to_biome = 1; // track biome that is being switched to
	float fade_status = 0; // 0 - before fade out, 1 after fade out, 2 - after fade in
	bool game_over = false;
	GLuint from_biome = 1;
	int tutorial_state = 0;
	bool tutorial_step_complete = true;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// used to hold grid line start and end positions
struct GridLine
{
	vec2 start_pos = { 0, 0 };
	vec2 end_pos = { 10, 10 }; // default to diagonal line
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
	float counter_ms = 3000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = { 1, 1 };
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// =============================== ENCHANTED GROTTO COMPONENTS =========================================

// represents a potion in our game
// note that cauldron entities may have this component too, so
// add an if condition if potions need to be looped through
struct Potion
{
	PotionEffect effect;
	int duration;         // in seconds
	float effectValue;    // Type-dependent value of potion 
	float quality;        // 0-1, a summary of how good the potion is
	vec3 color;
};

// an item that can be in an inventory
struct Item
{
	ItemType type;
	std::string name;
	bool isCollectable;
	int amount;
	float respawnTime = 0.0f;
	vec2 originalPosition;
	bool is_ammo = false;
};

// an item that can be added to potions
struct Ingredient
{
	float grindLevel; // -1 = ungrindable, 0 = ungrinded, 1 = full
};

// allows entities to have a storage system
struct Inventory
{
	std::vector<Entity> items;
	int capacity;
	bool isFull;
	int selection = 0; // index that corresponds to the selected item indexed in items
};

struct Cauldron
{
	vec3 color = DEFAULT_COLOR;       // The water entity to send the color to
	int heatLevel = 0;                // 0-100
	bool filled = false;              // Whether the cauldron has water
	int timeElapsed = 0;              // Time elapsed since water filled and heat knob turned, in ms
	int timeSinceLastAction = 0;      // Time elapsed since the last action, in ms. -1 means
	int colorElapsed = 0;             // Time in ms for color updates
	std::vector<Action> actions;      // Records player actions
	// If stir quality ever gets added, a penalty can be recorded here
};

// a menu of our game (recipe book menu, potion making menu, grinding menu...)
struct Menu
{
	bool mouseInput; // true if we allow mouse input
	bool keyInput;	 // true if we allow key input
};

// TODO: Better Mortar and pestle representation
struct MortarAndPestle
{
	std::vector<Entity> items;
	int grindLevel;
	int itemState;
};


// Obstacles in our environment that the player collides with
struct Terrain
{
	// 0 - uses bottom bounding box for collisions, allows player to walk behind terrain (trees, rocks)
	//     if 0, specify the ratio of the bounding box in proportion to the sprite. Bounding box collision
	//     logic can be found in physics_system.cpp, boxes are drawn on bottom middle of sprite
	// 1 - uses full bounding box for collisions, player cannot walk into terrain at all (river)
	// 2 - no collision box at all (a separate mesh entity is used)
	// 3 - this is the mesh entity we are using for collision, doesn't use AABB in physics_system
	float collision_setting;
	float width_ratio = 1.0f;
	float height_ratio = 1.0f;
};

struct Entrance
{
	GLuint target_biome = 0;
};

struct Textbox
{
	Entity targetItem;		// The item this textbox belongs to
	bool isVisible = false; // Visibility of the textbox
};

struct Chest {
	// Empty struct, just used to identify chest entities
};

struct Enemy {
	int health;
	int attack_radius;
	vec2 start_pos;
	int state; // uses enum class ENEMY_STATE
	int can_move;
	float wander_timer = 10.0f;  // 10-second random movement before returning
};

struct Ammo {
	vec2 start_pos;
	vec2 target; // mouse click direction at max of player's throwable radius
	bool is_fired = false;
	int damage = 0;
};

struct DecisionTreeNode {
	std::function<bool()> condition;  // Condition to check
	ENEMY_STATE trueState;            // Next state if condition is true
	ENEMY_STATE falseState;           // Next state if condition is false

	DecisionTreeNode(std::function<bool()> cond, ENEMY_STATE tState, ENEMY_STATE fState)
		: condition(cond), trueState(tState), falseState(fState) {
	}
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID
{
	PLAYER = 0,
	PLAYER_WALKING_A_1 = PLAYER + 1,
	PLAYER_WALKING_A_2 = PLAYER_WALKING_A_1 + 1,
	PLAYER_WALKING_A_3 = PLAYER_WALKING_A_2 + 1,
	PLAYER_WALKING_A_4 = PLAYER_WALKING_A_3 + 1,
	PLAYER_WALKING_D_1 = PLAYER_WALKING_A_4 + 1,
	PLAYER_WALKING_D_2 = PLAYER_WALKING_D_1 + 1,
	PLAYER_WALKING_D_3 = PLAYER_WALKING_D_2 + 1,
	PLAYER_WALKING_D_4 = PLAYER_WALKING_D_3 + 1,
	PLAYER_WALKING_S_1 = PLAYER_WALKING_D_4 + 1,
	PLAYER_WALKING_S_2 = PLAYER_WALKING_S_1 + 1,
	PLAYER_WALKING_S_3 = PLAYER_WALKING_S_2 + 1,
	PLAYER_WALKING_S_4 = PLAYER_WALKING_S_3 + 1,
	PLAYER_WALKING_W_1 = PLAYER_WALKING_S_4 + 1,
	PLAYER_WALKING_W_2 = PLAYER_WALKING_W_1 + 1,
	PLAYER_WALKING_W_3 = PLAYER_WALKING_W_2 + 1,
	PLAYER_WALKING_W_4 = PLAYER_WALKING_W_3 + 1,
	FOREST_BRIDGE = PLAYER_WALKING_W_4 + 1,
	FOREST_BRIDGE_TOP = FOREST_BRIDGE + 1,
	FOREST_BRIDGE_BOTTOM = FOREST_BRIDGE_TOP + 1,
	FOREST_RIVER_TOP = FOREST_BRIDGE_BOTTOM + 1,
	FOREST_RIVER_BOTTOM = FOREST_RIVER_TOP + 1,
	FOREST_BG = FOREST_RIVER_BOTTOM + 1,
	FOREST_TO_DESERT = FOREST_BG + 1,
	TREE = FOREST_TO_DESERT + 1,
	GROTTO_ENTRANCE = TREE + 1,
	GROTTO_BG = GROTTO_ENTRANCE + 1,
	GROTTO_CARPET = GROTTO_BG + 1,
	GROTTO_CAULDRON = GROTTO_CARPET + 1,
	GROTTO_CHEST = GROTTO_CAULDRON + 1,
	GROTTO_MORTAR_PESTLE = GROTTO_CHEST + 1,
	GROTTO_POOL = GROTTO_MORTAR_PESTLE + 1,
	GROTTO_RECIPE_BOOK = GROTTO_POOL + 1,
	GROTTO_RIGHT_BOOKSHELF = GROTTO_RECIPE_BOOK + 1,
	GROTTO_TOP_BOOKSHELF = GROTTO_RIGHT_BOOKSHELF + 1,
	DESERT_BG = GROTTO_TOP_BOOKSHELF + 1,
	DESERT_CACTUS = DESERT_BG + 1,
	DESERT_RIVER = DESERT_CACTUS + 1,
	DESERT_SKULL = DESERT_RIVER + 1,
	DESERT_TREE = DESERT_SKULL + 1,
	DESERT_TO_FOREST = DESERT_TREE + 1,
	DESERT_SAND_PILE_PAGE = DESERT_TO_FOREST + 1,
	BOUNDARY_LINE = DESERT_SAND_PILE_PAGE + 1,
	BUSH = BOUNDARY_LINE + 1,
	FRUIT = BUSH + 1,
	COFFEE_BEAN = FRUIT + 1,
	TEXTBOX_FRUIT = COFFEE_BEAN + 1,
	TEXTBOX_COFFEE_BEAN = TEXTBOX_FRUIT + 1,
	TEXTBOX_ENTER_GROTTO = TEXTBOX_COFFEE_BEAN + 1,
	TEXTBOX_GROTTO_EXIT = TEXTBOX_ENTER_GROTTO + 1,
	TEXTBOX_CAULDRON = TEXTBOX_GROTTO_EXIT + 1,
	TEXTBOX_ENTER_DESERT = TEXTBOX_CAULDRON + 1,
	TEXTBOX_ENTER_FOREST = TEXTBOX_ENTER_DESERT + 1,
	ENT = TEXTBOX_ENTER_FOREST + 1,
	MUMMY = ENT + 1,
	TEXTURE_COUNT = MUMMY + 1,
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;


// this has to be under TEXTURE_ASSET_ID so it recognizes it as a type
struct Animation {
	std::vector<TEXTURE_ASSET_ID> frames;
	float frame_time; // time for each frame
	float elapsed_time; // time since the last time we switched frames
	int current_frame;
};

enum class EFFECT_ASSET_ID
{
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	BACKGROUND = TEXTURED + 1,
	FADE = BACKGROUND + 1,
	EFFECT_COUNT = FADE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
	CHICKEN = 0,
	SPRITE = CHICKEN + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	BRIDGE_TOP = SCREEN_TRIANGLE + 1,
	BRIDGE_BOTTOM = BRIDGE_TOP + 1,
	GEOMETRY_COUNT = BRIDGE_BOTTOM + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class RENDER_LAYER
{
	BACKGROUND,
	TERRAIN,
	STRUCTURE,
	PLAYER,
	ITEM,
	UI
};

struct RenderRequest
{
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	RENDER_LAYER layer = RENDER_LAYER::BACKGROUND;
	int render_sub_layer = 0; // lower values are rendered above
	bool is_visible = true;
};

enum class BIOME
{
	GROTTO = 0,
	FOREST = GROTTO + 1,
	BLANK = FOREST + 1,
	DESERT = BLANK + 1,
};

enum class DIRECTION
{
	UP = 0,
	DOWN = 1,
	RIGHT = 2,
	LEFT = 3
};