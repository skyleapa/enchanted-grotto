#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_set>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{
	std::string name;
	float cooldown = 0.f; // defaults to 0, but when ammo is tossed, will have a 1000 ms cooldown
	float health = PLAYER_MAX_HEALTH;
	float damage_cooldown = 0.f; // cooldown before player can take damage again to prevent insta death
	std::vector<Entity> active_effects = {}; // list of active consumed potions
	float speed_multiplier = 1.f;
	float effect_multiplier = 1.f;
	float defense = 1.f;
	vec2 load_position = vec2(0, 0);
	float walking_timer = 0.f;
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
	GLuint switching_to_biome = 0; // track biome that is being switched to
	float fade_status = 0; // 0 - before fade out, 1 after fade out, 2 - after fade in
	GLuint from_biome = 2;
	int tutorial_state = 0;
	bool tutorial_step_complete = true;
	float autosave_timer = AUTOSAVE_TIMER;
	std::vector<std::string> killed_enemies = {};
	std::vector<std::string> unlocked_biomes = {};
	bool first_game_load = true;
	bool play_ending = false;
	bool saved_grotto = false;
	bool ending_text_shown = false;
	float fog_intensity = 1.5f;
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

enum class BIOME
{
	GROTTO = 0,
	FOREST = GROTTO + 1,
	FOREST_EX = FOREST + 1,
	DESERT = FOREST_EX + 1,
	MUSHROOM = DESERT + 1,
	CRYSTAL = MUSHROOM + 1,
	BLANK = CRYSTAL + 1,
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
	bool canRespawn = true;
	BIOME lastBiome;
	std::string persistentID = ""; // Unique identifier for respawn tracking
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
	int selection = 0; // index that corresponds to the selected item indexed in items, zero-indexed
};

struct Cauldron
{
	vec3 color = DEFAULT_COLOR;       // The water entity to send the color to
	int heatLevel = 0;                // 0-100
	bool filled = false;              // Whether the cauldron has water
	int timeElapsed = 0;              // Time elapsed since water filled and heat knob turned, in ms
	int timeSinceLastAction = 0;      // Time elapsed since the last action, in ms. -1 means
	int colorElapsed = 0;             // Time in ms for color updates
	int stirFlash = 0;                // Time remaining for stir flash
	std::vector<Action> actions;      // Records player actions
	Entity water;                     // We technically only need 1 of these globally but this was easier so whatever
	bool is_boiling = false;
	int num_stirs = 0;
	// If stir quality ever gets added, a penalty can be recorded here
};

// a menu of our game (recipe book menu, potion making menu, grinding menu...)
struct Menu
{
	bool mouseInput; // true if we allow mouse input
	bool keyInput;	 // true if we allow key input
};

struct MortarAndPestle
{
	bool grinded = false;
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
	std::string text;
	vec2 pos;
};

struct Chest {
	// Empty struct, just used to identify chest entities
};

struct Enemy {
	float health;
	float max_health;
	int attack_radius;
	vec2 start_pos;
	int state; // uses enum class ENEMY_STATE
	int can_move;
	float wander_timer = 10.0f;  // 10-second random movement before returning
	std::string name; // gets passed into killed_enemies
	float attack_damage;
	float dot_damage = 0.0f;
	float dot_timer = 0.0f;
	float dot_duration = 0.0f;
	PotionEffect dot_effect = PotionEffect::WATER;
	std::string persistentID = ""; // Unique identifier for respawn tracking
};

struct Guardian {
	PotionEffect unlock_potion; // the potion that will "unlock the biome"
	vec2 exit_direction = { 0,0 };

	// Dialogues
	std::string hint_dialogue;
	std::string wrong_potion_dialogue;
	std::string success_dialogue;
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

struct WelcomeScreen {

};

struct DelayedMovement {
	vec2 velocity;
	float delay_ms;
};

struct TexturedEffect {
	float animation_timer = 0.f;
	bool done_growing = false;
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
	// player
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

	// forest
	FOREST_BRIDGE = PLAYER_WALKING_W_4 + 1,
	FOREST_BRIDGE_TOP = FOREST_BRIDGE + 1,
	FOREST_BRIDGE_BOTTOM = FOREST_BRIDGE_TOP + 1,
	FOREST_RIVER_TOP = FOREST_BRIDGE_BOTTOM + 1,
	FOREST_RIVER_BOTTOM = FOREST_RIVER_TOP + 1,
	FOREST_BG = FOREST_RIVER_BOTTOM + 1,
	FOREST_TO_DESERT = FOREST_BG + 1,
	FOREST_TO_MUSHROOM = FOREST_TO_DESERT + 1,
	TREE = FOREST_TO_MUSHROOM + 1,

	// forest expansion
	FOREST_EX_BG = TREE + 1,

	// grotto
	GROTTO_ENTRANCE = FOREST_EX_BG + 1,
	GROTTO_BG = GROTTO_ENTRANCE + 1,
	GROTTO_CARPET = GROTTO_BG + 1,
	GROTTO_CAULDRON = GROTTO_CARPET + 1,
	GROTTO_CHEST = GROTTO_CAULDRON + 1,
	GROTTO_MORTAR_PESTLE = GROTTO_CHEST + 1,
	GROTTO_POOL = GROTTO_MORTAR_PESTLE + 1,
	GROTTO_RECIPE_BOOK = GROTTO_POOL + 1,
	GROTTO_RIGHT_BOOKSHELF = GROTTO_RECIPE_BOOK + 1,
	GROTTO_TOP_BOOKSHELF = GROTTO_RIGHT_BOOKSHELF + 1,

	// desert
	DESERT_BG = GROTTO_TOP_BOOKSHELF + 1,
	DESERT_CACTUS = DESERT_BG + 1,
	DESERT_RIVER = DESERT_CACTUS + 1,
	DESERT_SKULL = DESERT_RIVER + 1,
	DESERT_TREE = DESERT_SKULL + 1,
	DESERT_TO_FOREST = DESERT_TREE + 1,
	DESERT_SAND_PILE_PAGE = DESERT_TO_FOREST + 1,

	// mushroom 
	MUSHROOM_BG = DESERT_SAND_PILE_PAGE + 1,
	MUSHROOM_ACID_LAKE = MUSHROOM_BG + 1,
	MUSHROOM_BLUE = MUSHROOM_ACID_LAKE + 1,
	MUSHROOM_PINK = MUSHROOM_BLUE + 1,
	MUSHROOM_PURPLE = MUSHROOM_PINK + 1,
	MUSHROOM_TALL_BLUE = MUSHROOM_PURPLE + 1,
	MUSHROOM_TALL_PINK = MUSHROOM_TALL_BLUE + 1,

	// crystal
	CRYSTAL_BG = MUSHROOM_TALL_PINK + 1,
	CRYSTAL_1 = CRYSTAL_BG + 1,
	CRYSTAL_2 = CRYSTAL_1 + 1,
	CRYSTAL_3 = CRYSTAL_2 + 1,
	CRYSTAL_4 = CRYSTAL_3 + 1,
	CRYSTAL_MINECART = CRYSTAL_4 + 1,
	CRYSTAL_PAGE = CRYSTAL_MINECART + 1,
	CRYSTAL_ROCK = CRYSTAL_PAGE + 1,

	BOUNDARY_LINE = CRYSTAL_ROCK + 1,

	// interactables
	BUSH = BOUNDARY_LINE + 1,
	FRUIT = BUSH + 1,
	COFFEE_BEAN = FRUIT + 1,
	SAP = COFFEE_BEAN + 1,
	MAGICAL_DUST = SAP + 1,
	EVERFERN = MAGICAL_DUST + 1,
	BLIGHTLEAF = EVERFERN + 1,
	STORM_BARK = BLIGHTLEAF + 1,
	GALEFRUIT = STORM_BARK + 1,
	MUMMY_BANDAGE = GALEFRUIT + 1,
	PETRIFIED_BONE = MUMMY_BANDAGE + 1,
	HEALING_LILY = PETRIFIED_BONE + 1,
	CACTUS_PULP = HEALING_LILY + 1,
	GLOWSHROOM = CACTUS_PULP + 1,
	DOOMCAP = GLOWSHROOM + 1,
	CRYSTABLOOM = DOOMCAP + 1,
	CRYSTAL_SHARD = CRYSTABLOOM + 1,
	QUARTZMELON = CRYSTAL_SHARD + 1,
	STORM_SAP = QUARTZMELON + 1,
	CACTUS_EXTRACT = STORM_SAP + 1,
	SWIFT_POWDER = CACTUS_EXTRACT + 1,
	BONE_DUST = SWIFT_POWDER + 1,
	CRYSTAL_MEPH = BONE_DUST + 1,
	GLOWSPORE = CRYSTAL_MEPH + 1,

	// enemies
	ENT = GLOWSPORE + 1,
	MUMMY = ENT + 1,
	GUARDIAN_DESERT = MUMMY + 1,
	GUARDIAN_SHROOMLAND = GUARDIAN_DESERT + 1,
	GUARDIAN_CRYSTAL = GUARDIAN_SHROOMLAND + 1,
	CRYSTAL_BUG = GUARDIAN_CRYSTAL + 1,
	EVIL_MUSHROOM = CRYSTAL_BUG + 1,

	// extras
	MASTER_POTION_PEDESTAL = EVIL_MUSHROOM + 1,
	POTION = MASTER_POTION_PEDESTAL + 1,
	WELCOME_TO_GROTTO = POTION + 1,
	CAULDRON_WATER = WELCOME_TO_GROTTO + 1,
	POTION_OF_REJUVENATION = CAULDRON_WATER + 1,
	GLOW_EFFECT = POTION_OF_REJUVENATION + 1,
	TEXTURE_COUNT = GLOW_EFFECT + 1,
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
	WATER_A = FADE + 1,  // don't add anything between the waters pls pls
	WATER_B = WATER_A + 1,
	WATER_C = WATER_B + 1,
	WATER_FINAL = WATER_C + 1,
	FOG = WATER_FINAL + 1,
	EFFECT_COUNT = FOG + 1
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
	GROTTO_POOL = BRIDGE_BOTTOM + 1,
	MUSHROOM_ACID_LAKE = GROTTO_POOL + 1,
	WATER_QUAD = MUSHROOM_ACID_LAKE + 1,
	GEOMETRY_COUNT = WATER_QUAD + 1
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

enum class DIRECTION
{
	UP = 0,
	DOWN = 1,
	RIGHT = 2,
	LEFT = 3
};

// STORES ALL INFO ABOUT COLLECTABLE ITEMS
struct ItemInfo {
	std::string name;
	vec2 size;
	TEXTURE_ASSET_ID texture;
	std::string texture_path;
	bool grindable;
};

const std::unordered_map<ItemType, ItemInfo> ITEM_INFO = {
	{
		ItemType::POTION, {
			"Potion",
			vec2(0, 0),
			TEXTURE_ASSET_ID::POTION,
			"interactables/potion_item.png",
			false}},
	{
		ItemType::COFFEE_BEANS, {
			"Swiftbean",
			vec2((float)GRID_CELL_HEIGHT_PX * 0.9, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::COFFEE_BEAN,
			"interactables/coffee_bean.png",
			true}},
	{
		ItemType::GALEFRUIT, {
			"Galefruit",
			vec2((float)GRID_CELL_WIDTH_PX * 0.7, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::GALEFRUIT,
			"interactables/galefruit.png",
			false}},
	{
		ItemType::SAP, {
			"Sap",
			vec2((float)GRID_CELL_WIDTH_PX * 1.5, (float)GRID_CELL_HEIGHT_PX * 1.8),
			TEXTURE_ASSET_ID::SAP,
			"interactables/sap.png",
			true}},
	{
		ItemType::MAGICAL_DUST, {
			"Magical Dust",
			vec2((float)GRID_CELL_WIDTH_PX * 0.9, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::MAGICAL_DUST,
			"interactables/magical_dust.png",
			true}},
	{
		ItemType::EVERFERN, {
			"Everfern",
			vec2((float)GRID_CELL_WIDTH_PX * 1.1, (float)GRID_CELL_HEIGHT_PX * 1.4),
			TEXTURE_ASSET_ID::EVERFERN,
			"interactables/everfern.png",
			false}},
	{
		ItemType::BLIGHTLEAF, {
			"Blightleaf",
			vec2((float)GRID_CELL_WIDTH_PX, (float)GRID_CELL_HEIGHT_PX * 1.2),
			TEXTURE_ASSET_ID::BLIGHTLEAF,
			"interactables/blightleaf.png",
			false}},
	{
		ItemType::STORM_BARK, {
			"Storm Bark",
			vec2((float)GRID_CELL_WIDTH_PX * 1.1, (float)GRID_CELL_HEIGHT_PX * 1.2),
			TEXTURE_ASSET_ID::STORM_BARK,
			"interactables/storm_bark.png",
			true}},
	{
		ItemType::MUMMY_BANDAGES, {
			"Mummy Bandages",
			vec2((float)GRID_CELL_WIDTH_PX, (float)GRID_CELL_HEIGHT_PX * 1.2),
			TEXTURE_ASSET_ID::MUMMY_BANDAGE,
			"interactables/mummy_bandage.png",
			false}},
	{
		ItemType::PETRIFIED_BONE, {
			"Petrified Bone",
			vec2((float)GRID_CELL_WIDTH_PX, (float)GRID_CELL_HEIGHT_PX * 1.2),
			TEXTURE_ASSET_ID::PETRIFIED_BONE,
			"interactables/petrified_bone.png",
			true}},
	{
		ItemType::HEALING_LILY, {
			"Healing Lily",
			vec2((float)GRID_CELL_WIDTH_PX, (float)GRID_CELL_HEIGHT_PX * 1.1),
			TEXTURE_ASSET_ID::HEALING_LILY,
			"interactables/healing_lily.png",
			false}},
	{
		ItemType::CACTUS_PULP, {
			"Cactus Pulp",
			vec2((float)GRID_CELL_WIDTH_PX * 0.5, (float)GRID_CELL_HEIGHT_PX * 0.6),
			TEXTURE_ASSET_ID::CACTUS_PULP,
			"interactables/cactus_pulp.png",
			true}},
	{
		ItemType::CACTUS_EXTRACT, {
			"Cactus Extract",
			vec2((float)GRID_CELL_WIDTH_PX * 0.6, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::CACTUS_EXTRACT,
			"interactables/cactus_extract.png",
			false}},
	{
		ItemType::GLOWSHROOM, {
			"Glowshroom",
			vec2((float)GRID_CELL_WIDTH_PX * 1.1, (float)GRID_CELL_HEIGHT_PX * 1.4),
			TEXTURE_ASSET_ID::GLOWSHROOM,
			"interactables/glowshroom.png",
			true}},
	{
		ItemType::DOOMCAP, {
			"Doomcap",
			vec2((float)GRID_CELL_WIDTH_PX, (float)GRID_CELL_HEIGHT_PX * 1.2),
			TEXTURE_ASSET_ID::DOOMCAP,
			"interactables/doomspore.png",
			false}},
	{
		ItemType::CRYSTAL_SHARD, {
			"Crystal Shard",
			vec2((float)GRID_CELL_WIDTH_PX, (float)GRID_CELL_HEIGHT_PX * 1.3),
			TEXTURE_ASSET_ID::CRYSTAL_SHARD,
			"interactables/crystal_shard.png",
			true}},
	{
		ItemType::QUARTZMELON, {
			"Quartzmelon",
			vec2((float)GRID_CELL_WIDTH_PX * 1.3, (float)GRID_CELL_HEIGHT_PX),
			TEXTURE_ASSET_ID::QUARTZMELON,
			"interactables/quartzmelon.png",
			false}},
	{
		ItemType::CRYSTABLOOM, {
			"Crystabloom",
			vec2((float)GRID_CELL_WIDTH_PX * 1.2, (float)GRID_CELL_HEIGHT_PX * 1.5),
			TEXTURE_ASSET_ID::CRYSTABLOOM,
			"interactables/crystabloom.png",
			false}},

	{
		ItemType::SWIFT_POWDER, {
			"Swift Powder",
			vec2((float)GRID_CELL_WIDTH_PX * 0.9, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::SWIFT_POWDER,
			"interactables/swift_powder.png",
			false} },

	{
		ItemType::STORM_SAP, {
			"Storm Sap",
			vec2((float)GRID_CELL_WIDTH_PX * 0.6, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::STORM_SAP,
			"interactables/storm_sap.png",
			false} },

	{
		ItemType::BONE_DUST, {
			"Bone Dust",
			vec2((float)GRID_CELL_WIDTH_PX * 0.9, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::BONE_DUST,
			"interactables/bone_dust.png",
			false} },

	{
		ItemType::GLOWSPORE, {
			"Glowspore",
			vec2((float)GRID_CELL_WIDTH_PX * 0.9, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::GLOWSPORE,
			"interactables/glowspore.png",
			false} },

	{
		ItemType::CRYSTAL_MEPH, {
			"Crystal Meph",
			vec2((float)GRID_CELL_WIDTH_PX * 0.9, (float)GRID_CELL_HEIGHT_PX * 0.9),
			TEXTURE_ASSET_ID::CRYSTAL_MEPH,
			"interactables/crystal_meph.png",
			false} },
};

const std::unordered_map<ItemType, std::vector<BIOME>> itemRespawnBiomes = {
	// Forest
	{ ItemType::COFFEE_BEANS,   { BIOME::FOREST, BIOME::FOREST_EX } },
	{ ItemType::GALEFRUIT,      { BIOME::FOREST, BIOME::FOREST_EX } },
	{ ItemType::EVERFERN,       { BIOME::FOREST, BIOME::FOREST_EX } },
	{ ItemType::BLIGHTLEAF,     { BIOME::FOREST, BIOME::FOREST_EX } },
	{ ItemType::STORM_BARK,     { BIOME::FOREST, BIOME::FOREST_EX } },

	// Desert
	{ ItemType::PETRIFIED_BONE, { BIOME::DESERT } },
	{ ItemType::HEALING_LILY,   { BIOME::DESERT } },
	{ ItemType::CACTUS_PULP,    { BIOME::DESERT } },

	// Mushroom
	{ ItemType::GLOWSHROOM,    { BIOME::MUSHROOM } },
	{ ItemType::DOOMCAP,        { BIOME::MUSHROOM } },

	// Crystal Mountains
	{ ItemType::CRYSTABLOOM,    { BIOME::CRYSTAL } },
	{ ItemType::CRYSTAL_SHARD,  { BIOME::CRYSTAL } },
	{ ItemType::QUARTZMELON,    { BIOME::CRYSTAL } }
};

// damage flash only to be applied to player and enemies
struct DamageFlash {
	float flash_value = 1.f; // defaults to 0 for no flash, and 1 for red tint
	bool kill_after_flash = false;
};

struct Regeneration {
	float heal_amount = 0.f;
	float timer = REGEN_TIMER;
};