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
struct Motion {
	vec2  position = { 0, 0 };
	float angle    = 0;
	vec2  velocity = { 0, 0 };
	vec2  scale    = { 10, 10 };
	int moving_direction = 0;
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
struct Debug {
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
struct GridLine {
	vec2 start_pos = {  0,  0 };
	vec2 end_pos   = { 10, 10 };	// default to diagonal line
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
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// =============================== ENCHANTED GROTTO COMPONENTS =========================================
// represents a potion in our game
struct Potion
{
	int effect; // NOTE: we can turn effect into an enum
	int duration;
	vec3 color;
	float quality;
};

// an item that can be in an inventory
struct Item
{
	int id;
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
	// commented out temporarily as Item results in a compilation error
	// std::unordered_map<Item, int> items; // map of the item to the amount 
	std::vector<Entity> items;
	int capacity;
	bool isFull;
};

struct Cauldron
{
	std::vector<Item> items; // could this also be Inventory?
	int heatLevel;
	int stirLevel;
	vec3 color;
};

// a menu of our game (recipe book menu, potion making menu, grinding menu...)
struct Menu
{
	bool mouseInput; // true if we allow mouse input
	bool keyInput; // true if we allow key input
};

// a recipe in our recipe book menu
struct Recipe
{
	std::string potionName;
	int potionEffect;
	int highestQualityEffect;
	int highestQualityDuration;
	vec3 finalPotionColor;
	// commented out temporarily as Ingredient results in a compilation error
	// std::unordered_map<Ingredient, int> ingredients; // the ingredient and quantity
	int steps; // TODO: determine what this field should be
};

struct MortarAndPestle
{
	std::vector<Item> items; // could this also be Inventory?
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

enum class TEXTURE_ASSET_ID {
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
	TEXTURE_COUNT = GROTTO_ENTRANCE + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	CHICKEN = EGG + 1,
	TEXTURED = CHICKEN + 1,
	BACKGROUND = TEXTURED + 1,
	EFFECT_COUNT = BACKGROUND + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	CHICKEN = 0,
	SPRITE = CHICKEN + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class RENDER_LAYER {
    BACKGROUND,
    TERRAIN,
    STRUCTURE,
	ITEM,
    PLAYER
};

struct RenderRequest {
	TEXTURE_ASSET_ID   used_texture  = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID    used_effect   = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	RENDER_LAYER layer = RENDER_LAYER::BACKGROUND;
	int render_sub_layer = 0; // lower values are rendered above
};

enum class BIOME {
	GROTTO = 0,
	FOREST = GROTTO + 1
};

enum class DIRECTION {
	UP = 0,
	DOWN = 1,
	RIGHT = 2,
	LEFT = 3
};