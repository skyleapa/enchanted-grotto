#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{
	std::string name;
};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = {0, 0};
	float angle = 0;
	vec2 velocity = {0, 0};
	vec2 scale = {10, 10};
	int moving_direction = 0;
	vec2 previous_position = {0, 0};
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity &other) { this->other = other; };
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
	float darken_screen_factor = -1;
	GLuint biome = 1; // default to forest
	std::vector<int> pressed_keys = {};
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// used to hold grid line start and end positions
struct GridLine
{
	vec2 start_pos = {0, 0};
	vec2 end_pos = {10, 10}; // default to diagonal line
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
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
	vec2 original_size = {1, 1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// =============================== ENCHANTED GROTTO COMPONENTS =========================================

// Potion Types
// IMPORTANT: Add new effects to the end of the list to not break serialization!
// IMPORTANT: Add the displayname for each PotionEffect to common.hpp EFFECT_NAME
// Type descriptions and their values:
// FAILED: A potion that did not match any of the ingredients listed in any recipe
// SPEED: Increases player speed. Value is the speed multiplier
enum class PotionEffect
{
	FAILED = 0,
	SPEED = FAILED + 1
};

// represents a potion in our game
struct Potion
{
	PotionEffect effect;
	int duration;         // in seconds
	float effectValue;    // Type-dependent value of potion 
	float quality;        // 0-1, a summary of how good the potion is
	vec3 color;
};

// Item Types
// IMPORTANT: Add new types to the end of the list to not break serialization!
// IMPORTANT: Add the displayname for each ItemType to common.hpp ITEM_NAME
enum class ItemType
{
	POTION = 0,
	COFFEE_BEANS = POTION + 1,
	MAGICAL_FRUIT = COFFEE_BEANS + 1
};

// an item that can be in an inventory
struct Item
{
	ItemType type;
	std::string name;
	bool isCollectable;
	int amount;
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
};

struct Cauldron
{
	float heatLevel;              // 0-1
	vec3 color;                   // RGB color
	bool filled;                  // Whether the cauldron has water
	int timeElapsed;              // Time elapsed since water filled and heat knob turned, in ms
	int timeSinceLastAction;      // Time elapsed since the last action, in ms
	std::vector<Action> actions;  // Records player actions
};

// a menu of our game (recipe book menu, potion making menu, grinding menu...)
struct Menu
{
	bool mouseInput; // true if we allow mouse input
	bool keyInput;	 // true if we allow key input
};

// Action types
// WAIT: Records a wait time of some constant minutes defined in common.hpp (default 5).
//       The value represents how many units of that constant wait time have been recorded.
//       e.g. A WAIT with value 6 and default wait time of 5 is a 30 minute wait
// ADD_INGREDINET: Player puts in an ingredient. The value is the index in the cauldron
//                 inventory that stores the entity ID of that item
// MODIFY_HEAT: Player modifies the heat level. Value is a float 0-1, the resulting heat level
// STIR: Player stirs. Action should be recorded when player puts down the ladle.
//       Value is the number of stirs recorded
// BOTTLE: Should always be the last action. Value ignored.
// Note that MODIFY_HEAT is always the first action, since the heat can only be modified once
// the cauldron has been filled
enum class ActionType
{
	WAIT,
	ADD_INGREDIENT,
	MODIFY_HEAT,
	STIR,
	BOTTLE
};

// An action that records a step done by the player in the cauldron
struct Action
{
	ActionType type;
	int value;
};

// A recipe-specific short format for storing ingredient requirements
struct RecipeIngredient
{
	ItemType type;
	int amount;
	float grindAmount;
};

// a recipe in our recipe book menu
struct Recipe
{
	PotionEffect effect;
	int highestQualityEffect;        // corresponds to effectValue
	int highestQualityDuration;
	vec3 finalPotionColor;
	std::vector<RecipeIngredient> ingredients;
	std::vector<Action> steps;
};

// TODO: Better Mortar and pestle representation
struct MortarAndPestle
{
	std::vector<Entity> items;
	int grindLevel;
	int itemState;
};

struct Moving
{
};

// Obstacles in our environment that the player collides with
struct Terrain
{
	// 0 - uses bottom bounding box for collisions, allows player to walk behind terrain (trees, rocks)
	//     if 0, specify the ratio of the bounding box in proportion to the sprite. Bounding box collision
	//     logic can be found in physics_system.cpp, boxes are drawn on bottom middle of sprite
	// 1 - uses full bounding box for collisions, player cannot walk into terrain at all (river)
	float collision_setting;
	float width_ratio = 1.0f;
	float height_ratio = 1.0f;
};

struct Textbox
{
	Entity targetItem;		// The item this textbox belongs to
	bool isVisible = false; // Visibility of the textbox
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
	FOREST_BRIDGE = PLAYER + 1,
	FOREST_RIVER_ABOVE = FOREST_BRIDGE + 1,
	FOREST_RIVER_BELOW = FOREST_RIVER_ABOVE + 1,
	FOREST_BG = FOREST_RIVER_BELOW + 1,
	TREE = FOREST_BG + 1,
	BUSH = TREE + 1,
	FRUIT = BUSH + 1,
	COFFEE_BEAN = FRUIT + 1,
	GROTTO_ENTRANCE = COFFEE_BEAN + 1,
	TEXTBOX_FRUIT = GROTTO_ENTRANCE + 1,
	TEXTBOX_COFFEE_BEAN = TEXTBOX_FRUIT + 1,
	TEXTBOX_ENTER_GROTTO = TEXTBOX_COFFEE_BEAN + 1,
	TEXTURE_COUNT = TEXTBOX_ENTER_GROTTO + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID
{
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	BACKGROUND = TEXTURED + 1,
	EFFECT_COUNT = BACKGROUND + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
	CHICKEN = 0,
	SPRITE = CHICKEN + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class RENDER_LAYER
{
	BACKGROUND,
	TERRAIN,
	STRUCTURE,
	ITEM,
	PLAYER
};

struct RenderRequest
{
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	RENDER_LAYER layer = RENDER_LAYER::BACKGROUND;
	int render_sub_layer = 0; // lower values are rendered above
};

enum class BIOME
{
	GROTTO = 0,
	FOREST = GROTTO + 1
};

enum class DIRECTION
{
	UP = 0,
	DOWN = 1,
	RIGHT = 2,
	LEFT = 3
};