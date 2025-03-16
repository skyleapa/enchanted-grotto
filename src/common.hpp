#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include <map>

// glfw (OpenGL)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>			   // vec2
#include <glm/ext/vector_int2.hpp> // ivec2
#include <glm/vec3.hpp>			   // vec3
#include <glm/mat3x3.hpp>		   // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) { return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name; };
inline std::string textures_path(const std::string& name) { return data_path() + "/textures/" + std::string(name); };
inline std::string audio_path(const std::string& name) { return data_path() + "/audio/" + std::string(name); };
inline std::string mesh_path(const std::string& name) { return data_path() + "/meshes/" + std::string(name); };

//
// game constants
//
const int WINDOW_WIDTH_PX = 1250;
const int WINDOW_HEIGHT_PX = 700;
const float WINDOW_RATIO = (float)WINDOW_WIDTH_PX / WINDOW_HEIGHT_PX;

const int GRID_CELL_WIDTH_PX = 50;
const int GRID_CELL_HEIGHT_PX = 50;
const int GRID_LINE_WIDTH_PX = 2;

const float PLAYER_BB_WIDTH = (float)65;
const float PLAYER_BB_HEIGHT = (float)100;
const float PlAYER_BB_GROTTO_SIZE_FACTOR = 1.8;
const float PLAYER_SPEED = (float)200;

const float TIME_UPDATE_FACTOR = 0.001f;
const float THROW_UPDATE_FACTOR = 0.3f;
const float AUTOSAVE_TIMER = 1000.f * 60; // every 60 seconds

const float TREE_WIDTH = (float)GRID_CELL_WIDTH_PX * 4;
const float TREE_HEIGHT = (float)GRID_CELL_HEIGHT_PX * 4;

const float FOREST_RIVER_ABOVE_WIDTH = (float)200;
const float FOREST_RIVER_ABOVE_HEIGHT = (float)395;

const float FOREST_RIVER_BELOW_WIDTH = (float)200;
const float FOREST_RIVER_BELOW_HEIGHT = (float)153;

const float FOREST_BRIDGE_WIDTH = (float)284;
const float FOREST_BRIDGE_HEIGHT = (float)218;

const float GROTTO_ENTRANCE_X = (float)1000;
const float GROTTO_ENTRANCE_Y = (float)100;

const float BOUNDARY_LINE_THICKNESS = 3;
const float GROTTO_ENTRANCE_WIDTH = (float)400;
const float GROTTO_ENTRANCE_HEIGHT = (float)180;

const float BUSH_WIDTH = (float)220;
const float BUSH_HEIGHT = (float)130;

const float TEXTBOX_WIDTH = (float)GRID_CELL_WIDTH_PX * 4;
const float TEXTBOX_HEIGHT = (float)GRID_CELL_HEIGHT_PX * 4;

const float ITEM_PICKUP_RADIUS = (float)100;
const float INTERACTION_RADIUS = (float)100;
const float TEXTBOX_VISIBILITY_RADIUS = (float)100;

const float DESERT_RIVER_WIDTH = (float)GRID_CELL_WIDTH_PX * 2;
const float DESERT_RIVER_HEIGHT = (float)WINDOW_HEIGHT_PX;

const float DESERT_TREE_WIDTH = (float)212 * 0.8;
const float DESERT_TREE_HEIGHT = (float)284 * 0.8;

const float DESERT_CACTUS_WIDTH = (float)164 * 0.8;
const float DESERT_CACTUS_HEIGHT = (float)196 * 0.8;

const float DESERT_FOREST_TRANSITION_WIDTH = (float)140 * 1.1;
const float DESERT_FOREST_TRANSITION_HEIGHT = (float)153 * 1.1;

const float DESERT_SKULL_WIDTH = (float)160 * 0.8;
const float DESERT_SKULL_HEIGHT = (float)110 * 0.8;

const float DESERT_PAGE_WIDTH = (float)54 * 0.8;
const float DESERT_PAGE_HEIGHT = (float)37 * 0.8;

const float CAULDRON_WATER_WIDTH = (float)354 * 0.9;
const float CAULDRON_WATER_HEIGHT = (float)337 * 0.9;
const vec2 CAULDRON_WATER_POS = vec2(622, 289);

const float ENT_WIDTH = (float)90;
const float ENT_HEIGHT = (float)130;

const float MUMMY_WIDTH = (float)45;
const float MUMMY_HEIGHT = (float)85;

const float DETECTION_RADIUS = (float)200;  // Enemy starts moving & attacking
const float FOLLOWING_RADIUS = (float)300;  // Enemy stops attacking if outside this

const float ENEMY_SPEED = (float)110;

// Inventory bar 
const float BAR_WIDTH = (float)450.0f;
const float BAR_HEIGHT = (float)60.0f;
const float BAR_X = (float)((WINDOW_WIDTH_PX - BAR_WIDTH) / 2.0f);
const float BAR_Y = (float)(WINDOW_HEIGHT_PX - BAR_HEIGHT - 20.f); // 20 from bottom


// Item and potion constants. The enums are declared here instead of in components.hpp
// because this file is included in components, not the other way around - otherwise,
// it would result in compilation errors because the name constants would be referring
// to enums that have not been declared yet.
//
// Additionally, the structs that are included here can also be considered constants,
// since they have no associated component container in the registry and are only
// used for comparisons or storage in other components.

// Item Types and names
// IMPORTANT: Add new types to the end of the list to not break serialization!
// IMPORTANT: Add information for each ItemType to ITEM_INFO in common.cpp
enum class ItemType
{
	POTION = 0,
	COFFEE_BEANS = POTION + 1,
	MAGICAL_FRUIT = COFFEE_BEANS + 1,
	GROTTO_ENTRANCE = MAGICAL_FRUIT + 1,
	CAULDRON = GROTTO_ENTRANCE + 1,
	MORTAR_PESTLE = CAULDRON + 1,
	CHEST = MORTAR_PESTLE + 1,
	RECIPE_BOOK = CHEST + 1,
	GROTTO_EXIT = RECIPE_BOOK + 1,
	DESERT_ENTRANCE = GROTTO_EXIT + 1,
	FOREST_ENTRANCE = DESERT_ENTRANCE + 1,
	SAP = FOREST_ENTRANCE + 1,
	MAGICAL_DUST = SAP + 1
};

// Potion Types and names
// IMPORTANT: Add new effects to the end of the list to not break serialization!
// IMPORTANT: Add the displayname for each PotionEffect to EFFECT_NAME
// Type descriptions and their values:
// FAILED: A potion that did not match any of the ingredients listed in any recipe
// WATER: When a player bottles a cauldron that doesn't have any ingredients
// SPEED: Increases player speed. Value is the speed multiplier - 1
enum class PotionEffect
{
	FAILED = 0,
	WATER = FAILED + 1,
	SPEED = WATER + 1
};

const std::unordered_map<PotionEffect, std::string> EFFECT_NAMES = {
	{PotionEffect::FAILED, "Failed"},
	{PotionEffect::WATER, "Water"},
	{PotionEffect::SPEED, "Speed"}
};

// Action types
// WAIT: Records a wait time of some constant minutes defined in common.hpp (default 5).
//       The value represents how many units of that constant wait time have been recorded.
//       e.g. A WAIT with value 6 and default wait time of 5 is a 30 minute wait
// ADD_INGREDINET: Player puts in an ingredient. The value is the index in the cauldron
//                 inventory that stores the entity ID of that item
// MODIFY_HEAT: Player modifies the heat level. Value is an int 1-100, the resulting heat level
// STIR: Player stirs. Action should be recorded when player puts down the ladle.
//       Value is the number of stirs recorded
enum class ActionType
{
	WAIT,
	ADD_INGREDIENT,
	MODIFY_HEAT,
	STIR
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

// A recipe in our recipe book menu
struct Recipe
{
	PotionEffect effect;
	float highestQualityEffect;        // corresponds to effectValue
	int highestQualityDuration;
	vec3 finalPotionColor;
	std::vector<RecipeIngredient> ingredients;
	std::vector<Action> steps;
};

const std::vector<Recipe> RECIPES = {
	{
		PotionEffect::SPEED,
		3.0f,  // highestQualityEffect
		180,   // highestQualityDuration
		vec3(255, 157, 35), // finalPotionColor
		{
			{ ItemType::COFFEE_BEANS, 5, 1.0f }, // ingredients
			{ ItemType::MAGICAL_FRUIT, 3, 0.0f }
		},
		{
			{ ActionType::MODIFY_HEAT, 100 }, // steps
			{ ActionType::WAIT, 2 },
			{ ActionType::ADD_INGREDIENT, 0 },
			{ ActionType::ADD_INGREDIENT, 1 },
			{ ActionType::STIR, 3 },
			{ ActionType::WAIT, 6 }
		}
	}
};

enum class ENEMY_STATE
{
	IDLE = 0,
	ATTACK = IDLE + 1,
	WANDER = ATTACK + 1,
	RETURN = WANDER + 1
};

// Default time represented by each "WAIT" action, in ms
const int DEFAULT_WAIT = 5000;

// Potion settings
const vec3 DEFAULT_COLOR = vec3(116, 204, 244);
const float MIN_POTENCY_PERCENTAGE = 0.1;
const float MIN_DURATION_PERCENTAGE = 0.05;

// Cauldron color settings
const int COLOR_FADE_DURATION = 5000;

// Recipe penalty settings
// If potion difficulty > 1, potions are harder to make good quality and vice versa
// If potion difficulty = 0, potions are always the highest quality
// Each penalty is multiplied by the difference in value to the recipe
const float POTION_DIFFICULTY = 1.0f;
const float INGREDIENT_TYPE_PENALTY = 0.5f;
const float INGREDIENT_AMOUNT_PENALTY = 0.1f;
const float INGREDIENT_GRIND_PENALTY = 1.0f;
const float STIR_PENALTY = 0.3f;
const float WAIT_PENALTY = 0.2f;
const float HEAT_PENALTY = 0.01f; // Heat is measured 1-100

enum class TUTORIAL {
	WELCOME_SCREEN = 0,
	MOVEMENT = WELCOME_SCREEN + 1,
	COLLECT_ITEMS = MOVEMENT + 1,
	ATTACK_ENEMY = COLLECT_ITEMS + 1,
	ENTER_GROTTO = ATTACK_ENEMY + 1,
	INTERACT_CAULDRON = ENTER_GROTTO + 1,
	SET_HEAT = INTERACT_CAULDRON + 1,
	ADD_INGREDIENT = SET_HEAT + 1,
	STIR = ADD_INGREDIENT + 1,
	BOTTLE = STIR + 1,
	EXIT_MENU = BOTTLE + 1,
	COMPLETE = EXIT_MENU + 1
};

enum class SOUND_CHANNEL {
	GENERAL = -1, // this indicates choosing any available channel
	BGM = 0,
	HEAT_BOIL_AUDIO_CHANNEL = BGM + 1
};

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform
{
	mat3 mat = { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
