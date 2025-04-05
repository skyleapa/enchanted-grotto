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
inline std::string game_state_path(const std::string& name) { return data_path() + "/game_states/v2/" + std::string(name); };

const std::string GAME_STATE_FILE = "game_state.json";


// 0 = lower quality (higher FPS), 1 = higher quality (computer fan go brrr)
const int WATER_QUALITY_LEVEL = 1;

//
// game constants
//

const bool ADMIN_FLAG = false;

const int WINDOW_WIDTH_PX = 1250;
const int WINDOW_HEIGHT_PX = 700;
const float WINDOW_RATIO = (float)WINDOW_WIDTH_PX / WINDOW_HEIGHT_PX;

const int GRID_CELL_WIDTH_PX = 50;
const int GRID_CELL_HEIGHT_PX = 50;
const int GRID_LINE_WIDTH_PX = 2;

const float PLAYER_BB_WIDTH = (float)75;
const float PLAYER_BB_HEIGHT = (float)110;
const float PlAYER_BB_GROTTO_SIZE_FACTOR = 1.8;
const float PLAYER_SPEED = (float)200;
const float PLAYER_MAX_HEALTH = (float)100;
const float PLAYER_DYING = (float)20;
const float PLAYER_DAMAGE_COOLDOWN = (float)1000.f;
const float PLAYER_THROW_COOLDOWN = (float)1000.f;

const float GUARDIAN_SPEED = (float)100;

const float TIME_UPDATE_FACTOR = 0.001f;
const float THROW_UPDATE_FACTOR = 0.3f;
const float AUTOSAVE_TIMER = 1000.f * 60; // every 60 seconds

const float TREE_WIDTH = (float)165;
const float TREE_HEIGHT = (float)200;

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

const float DESERT_FOREST_TRANSITION_WIDTH = (float)140;
const float DESERT_FOREST_TRANSITION_HEIGHT = (float)153;

const float DESERT_SKULL_WIDTH = (float)160 * 0.8;
const float DESERT_SKULL_HEIGHT = (float)110 * 0.8;

const float DESERT_PAGE_WIDTH = (float)54 * 0.8;
const float DESERT_PAGE_HEIGHT = (float)37 * 0.8;

const float FOREST_TO_MUSHROOM_WIDTH = (float)132;
const float FOREST_TO_MUSHROOM_HEIGHT = (float)96;

const float GENERIC_ENTRANCE_WIDTH = (float)100;
const float GENERIC_ENTRANCE_HEIGHT = (float)100;

const float MUSHROOM_ACID_LAKE_WIDTH = (float)828;
const float MUSHROOM_ACID_LAKE_HEIGHT = (float)233;

const float MUSHROOM_WIDTH = (float)230;
const float MUSHROOM_HEIGHT = (float)191;

const float MUSHROOM_TALL_WIDTH = (float)230;
const float MUSHROOM_TALL_HEIGHT = (float)285;

const float CRYSTAL_1_WIDTH = (float)148;
const float CRYSTAL_1_HEIGHT = (float)211;

const float CRYSTAL_2_WIDTH = (float)169;
const float CRYSTAL_2_HEIGHT = (float)234;

const float CRYSTAL_3_WIDTH = (float)104;
const float CRYSTAL_3_HEIGHT = (float)219;

const float CRYSTAL_4_WIDTH = (float)72;
const float CRYSTAL_4_HEIGHT = (float)92;

const float CRYSTAL_MINECART_WIDTH = (float)172;
const float CRYSTAL_MINECART_HEIGHT = (float)148;

const float CRYSTAL_PAGE_WIDTH = (float)19;
const float CRYSTAL_PAGE_HEIGHT = (float)19;

const float CRYSTAL_ROCK_WIDTH = (float)153;
const float CRYSTAL_ROCK_HEIGHT = (float)208;

const float CAULDRON_WATER_WIDTH = (float)354 * 0.9;
const float CAULDRON_WATER_HEIGHT = (float)337 * 0.9;

const float ENT_WIDTH = (float)90;
const float ENT_HEIGHT = (float)130;

const float MUMMY_WIDTH = (float)45;
const float MUMMY_HEIGHT = (float)85;

const float DESERT_GUARDIAN_WIDTH = (float)160;
const float DESERT_GUARDIAN_HEIGHT = (float)150;

const float MUSHROOM_GUARDIAN_WIDTH = (float)160;
const float MUSHROOM_GUARDIAN_HEIGHT = (float)150;

const float CRYSTAL_GUARDIAN_WIDTH = (float)140;
const float CRYSTAL_GUARDIAN_HEIGHT = (float)160;

const float PEDESTAL_WIDTH = (float)82;
const float PEDESTAL_HEIGHT = (float)117;

const float DETECTION_RADIUS = (float)200;  // Enemy starts moving & attacking
const float FOLLOWING_RADIUS = (float)300;  // Enemy stops attacking if outside this

const float ENEMY_SPEED = (float)110;

const int THROW_DISTANCE = 300; // Player throw dist in pixels

// volume ranges from 0 to 128
const int MUSIC_VOLUME = 64; //64;
const int MUSIC_VOLUME_LOWER = 32; //32;

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
// IMPORTANT: Add information for each ItemType to ITEM_INFO in components.hpp
enum class ItemType
{
	POTION = 0,
	COFFEE_BEANS = POTION + 1,
	GALEFRUIT = COFFEE_BEANS + 1,
	GROTTO_ENTRANCE = GALEFRUIT + 1,
	CAULDRON = GROTTO_ENTRANCE + 1,
	MORTAR_PESTLE = CAULDRON + 1,
	CHEST = MORTAR_PESTLE + 1,
	RECIPE_BOOK = CHEST + 1,
	GROTTO_EXIT = RECIPE_BOOK + 1,
	FOREST_TO_FOREST_EX_ENTRANCE = GROTTO_EXIT + 1,
	FOREST_EX_TO_FOREST_ENTRANCE = FOREST_TO_FOREST_EX_ENTRANCE + 1,
	FOREST_TO_DESERT_ENTRANCE = FOREST_EX_TO_FOREST_ENTRANCE + 1,
	DESERT_TO_FOREST_ENTRANCE = FOREST_TO_DESERT_ENTRANCE + 1,
	FOREST_TO_MUSHROOM_ENTRANCE = DESERT_TO_FOREST_ENTRANCE + 1,
	MUSHROOM_TO_FOREST_ENTRANCE = FOREST_TO_MUSHROOM_ENTRANCE + 1,
	MUSHROOM_TO_CRYSTAL_ENTRANCE = MUSHROOM_TO_FOREST_ENTRANCE + 1,
	CRYSTAL_TO_MUSHROOM_ENTRANCE = MUSHROOM_TO_CRYSTAL_ENTRANCE + 1,
	CRYSTAL_TO_FOREST_EX_ENTRANCE = CRYSTAL_TO_MUSHROOM_ENTRANCE + 1,
	FOREST_EX_TO_CRYSTAL_ENTRANCE = CRYSTAL_TO_FOREST_EX_ENTRANCE + 1,
	SAP = FOREST_EX_TO_CRYSTAL_ENTRANCE + 1,
	MAGICAL_DUST = SAP + 1,
	EVERFERN = MAGICAL_DUST + 1,
	BLIGHTLEAF = EVERFERN + 1,
	STORM_BARK = BLIGHTLEAF + 1,
	MUMMY_BANDAGES = STORM_BARK + 1,
	PETRIFIED_BONE = MUMMY_BANDAGES + 1,
	HEALING_LILY = PETRIFIED_BONE + 1,
	CACTUS_PULP = HEALING_LILY + 1,
	GLOWSHROOM = CACTUS_PULP + 1,
	DOOMCAP = GLOWSHROOM + 1,
	CRYSTAL_SHARD = DOOMCAP + 1,
	QUARTZMELON = CRYSTAL_SHARD + 1,
	CRYSTABLOOM = QUARTZMELON + 1,
	STORM_SAP = CRYSTABLOOM + 1,
	CACTUS_EXTRACT = STORM_SAP + 1,
	SWIFT_POWDER = CACTUS_EXTRACT + 1,
	BONE_DUST = SWIFT_POWDER + 1,
	CRYSTAL_MEPH = BONE_DUST + 1,
	GLOWSPORE = CRYSTAL_MEPH + 1,
	DESERT_GUARDIAN = GLOWSPORE + 1,
	MUSHROOM_GUARDIAN = DESERT_GUARDIAN + 1,
	CRYSTAL_GUARDIAN = MUSHROOM_GUARDIAN + 1,
	MASTER_POTION_PEDESTAL = CRYSTAL_GUARDIAN + 1
};

// Potion Types and names
// IMPORTANT: Add new effects to the end of the list to not break serialization!
// IMPORTANT: Add the displayname for each PotionEffect to EFFECT_NAME
// Type descriptions and their values:
// FAILED: A potion that did not match any of the ingredients listed in any recipe
// WATER: When a player bottles a cauldron that doesn't have any ingredients
// SPEED: Increases player speed. Value is the speed multiplier - 1
// HEALTH: Restores player health points
// DAMAGE: Causes damage to enemies when thrown
// MOLOTOV: Damages enemies in an area over time (fire effect)
// REGEN: Regenerates player health over time
// POISON: Creates poisonous area when thrown
// RESISTANCE: Reduces damage taken by player
// SATURATION: Increases potency of other potions
// ALKALESCENCE: Reacts with acidic substances
// CLARITY: Reveals hidden objects or ingredients
// REJUVENATION: Master potion with ultimate healing properties
enum class PotionEffect
{
	FAILED = 0,
	WATER = FAILED + 1,
	SPEED = WATER + 1,
	HEALTH = SPEED + 1,
	DAMAGE = HEALTH + 1,
	MOLOTOV = DAMAGE + 1,
	REGEN = MOLOTOV + 1,
	POISON = REGEN + 1,
	RESISTANCE = POISON + 1,
	SATURATION = RESISTANCE + 1,
	ALKALESCENCE = SATURATION + 1,
	CLARITY = ALKALESCENCE + 1,
	REJUVENATION = CLARITY + 1
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
// If type is POTION, then amount is read as a PotionEffect.
struct RecipeIngredient
{
	ItemType type;
	int amount;
	float grindAmount;
};

// A recipe in our recipe book menu
// Note that baseEffect and baseDuration correspond to values for a quality of 0
// Since basic potions start out with a quality of 20%, this value needs to be
// 1 STEP LOWER than the minimum specified in the doc. For example, for a speed potion
// with effects of 1.5-2-2.5-3-3.5, the baseEffect would be 1, not 1.5.
// This future proofs for if more quality steps are added.
struct Recipe
{
	PotionEffect effect;
	float baseEffect;
	float highestQualityEffect;        // corresponds to effectValue
	int baseDuration;
	int highestQualityDuration;
	vec3 finalPotionColor;
	std::vector<RecipeIngredient> ingredients;
	std::vector<Action> steps;
	std::string name;                  // Name of the potion
	std::string description;           // Description in recipe book
};

// Potions are accepted as recipe ingredients, but their amount value is used as the PotionEffect.
// To add multiple of the same potion in a row, simply define a new RecipeIngredient with the same
// potion and add 2 actions each pointing to one of the potions.
const std::vector<Recipe> RECIPES = {
	{
		PotionEffect::SPEED,
		1.0f, 3.5f,  // highestQualityEffect - maximum speed multiplier
		0, 150,   // highestQualityDuration - maximum duration in seconds
		vec3(255, 157, 35), // finalPotionColor - green-ish color
		{
			{ ItemType::COFFEE_BEANS, 2, 0.0f }, // ingredients (renamed to Swiftbeans in display)
			{ ItemType::GALEFRUIT, 2, 0.0f }  // ingredients (using the new GALEFRUIT type)
		},
		{
			{ ActionType::MODIFY_HEAT, 100 }, // high heat
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 0 }, // add swiftbeans
			{ ActionType::ADD_INGREDIENT, 1 }, // add galefruit
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 6 }           // wait 30 seconds
		},
		"Potion of Speed",
		"[Consumable] Increases your movement speed for a limited time."
	},
	{
		PotionEffect::HEALTH,
		10.0f, 40.0f,  // highestQualityEffect - maximum health restored
		0, 0,      // highestQualityDuration - instant effect, no duration
		vec3(220, 0, 0), // finalPotionColor - red color
		{
			{ ItemType::EVERFERN, 3, 0.0f } // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::WAIT, 1 },          // wait 5 seconds
			{ ActionType::ADD_INGREDIENT, 0 }, // add everfern
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 4 }           // wait 20 seconds
		},
		"Potion of Healing",
		"[Consumable] A basic potion that instantly replenishes your health."
	},
	{
		PotionEffect::DAMAGE,
		20.0f, 100.0f,  // highestQualityEffect - maximum damage to enemies
		0, 0,      // highestQualityDuration - instant effect, no duration
		vec3(100, 0, 100), // finalPotionColor - purple color
		{
			{ ItemType::BLIGHTLEAF, 1, 0.0f }, // ingredients
			{ ItemType::STORM_BARK, 1, 0.0f },  // ingredients
			{ ItemType::STORM_SAP, 1, 0.0f }
		},
		{
			{ ActionType::MODIFY_HEAT, 100 }, // high heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add blightleaf
			{ ActionType::ADD_INGREDIENT, 1 }, // add storm bark
			{ ActionType::ADD_INGREDIENT, 2 }, // add storm sap
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 1 }           // wait 5 seconds
		},
		"Potion of Harming",
		"[Throwable] A damaging potion that can be thrown at enemies."
	},
	{
		PotionEffect::MOLOTOV,
		0.0f, 25.0f,  // highestQualityEffect - burn damage per second
		0, 25,     // highestQualityDuration - burn duration in seconds
		vec3(255, 100, 0), // finalPotionColor - orange/fire color
		{
			{ ItemType::POTION, (int)PotionEffect::DAMAGE, 0.0f },
			{ ItemType::MUMMY_BANDAGES, 2, 0.0f }, // ingredients
			{ ItemType::PETRIFIED_BONE, 1, 0.0f }, // ingredients
			{ ItemType::BONE_DUST, 2, 0.0f }  // ingredients
		},
		{
			{ ActionType::ADD_INGREDIENT, 0 }, // add damage potion
			{ ActionType::MODIFY_HEAT, 25 },  // low heat
			{ ActionType::ADD_INGREDIENT, 1 }, // add mummy bandages
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::ADD_INGREDIENT, 2 }, // add petrified bones
			{ ActionType::ADD_INGREDIENT, 3 }, // add bone dust
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 4 }           // wait 20 seconds
		},
		"Molotov Cocktail",
		"[Throwable] Creates a burning area when thrown."
	},
	{
		PotionEffect::REGEN,
		2.5f, 5.0f,   // highestQualityEffect - health regen per second
		5, 30,     // highestQualityDuration - regen duration in seconds
		vec3(200, 50, 50), // finalPotionColor - pink-red color
		{
			{ ItemType::POTION, (int)PotionEffect::HEALTH, 0.0f },
			{ ItemType::HEALING_LILY, 2, 0.0f }, // ingredients
			{ ItemType::CACTUS_PULP, 1, 0.0f }     // ingredients
		},
		{
			{ ActionType::ADD_INGREDIENT, 0 }, // add health potion
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::ADD_INGREDIENT, 1 }, // add healing lily
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::ADD_INGREDIENT, 2 }, // add cactus pulp
			{ ActionType::WAIT, 3 },          // wait 15 seconds
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 3 }           // wait 15 seconds
		},
		"Potion of Regeneration",
		"[Consumable] Gradually restores health over time."
	},
	{
		PotionEffect::POISON,
		0.0f, 8.0f,   // highestQualityEffect - poison damage per second
		0, 20,     // highestQualityDuration - poison duration in seconds
		vec3(0, 150, 0), // finalPotionColor - green color
		{
			{ ItemType::POTION, (int)PotionEffect::DAMAGE, 0.0f },
			{ ItemType::BLIGHTLEAF, 2, 0.0f },    // ingredients
			{ ItemType::DOOMCAP, 2, 0.0f }      // ingredients
		},
		{
			{ ActionType::ADD_INGREDIENT, 0 }, // add damage potion
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::ADD_INGREDIENT, 1 }, // add blightleaf
			{ ActionType::WAIT, 1 },          // wait 5 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add doomcap
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 5 }           // wait 25 seconds
		},
		"Potion of Poison",
		"[Throwable] Creates a poisonous area."
	},
	{
		PotionEffect::RESISTANCE,
		0.0f, 0.8f,   // highestQualityEffect - damage reduction multiplier (0.5 = 50% reduced damage)
		0, 60,     // highestQualityDuration - resistance duration in seconds
		vec3(150, 150, 150), // finalPotionColor - silver/gray color
		{
			{ ItemType::CRYSTAL_SHARD, 2, 0.0f }, // ingredients
			{ ItemType::CACTUS_EXTRACT, 3, 0.0f } // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 100 }, // high heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add crystal shards
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 1 }, // add cactus extract
			{ ActionType::STIR, 5 },          // stir 5 times
			{ ActionType::WAIT, 6 }           // wait 30 seconds
		},
		"Potion of Resistance",
		"[Consumable] Reduces damage taken for a limited time."
	},
	{
		PotionEffect::SATURATION,
		1.0f, 2.0f,   // highestQualityEffect - potion effect multiplier (2x at max quality)
		0, 300,    // highestQualityDuration - duration in seconds
		vec3(200, 100, 200), // finalPotionColor - purple/pink color
		{
			{ ItemType::GALEFRUIT, 2, 0.0f },   // ingredients
			{ ItemType::STORM_BARK, 2, 0.0f },  // ingredients
			{ ItemType::EVERFERN, 1, 0.0f },     // ingredients
			{ ItemType::SWIFT_POWDER, 2, 0.0f },  // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::ADD_INGREDIENT, 1 }, // add storm bark
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add everfern
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 0 }, // add galefruit
			{ ActionType::ADD_INGREDIENT, 3 }, // add swift powder
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 5 }           // wait 25 seconds
		},
		"Potion of Saturation",
		"[Consumable] Temporarily increases the potency of other potions. Restores hydration to enemies that are very dry."
	},
	{
		PotionEffect::ALKALESCENCE,
		0.0f, 10.0f,  // highestQualityEffect - reactive strength
		0, 20,     // highestQualityDuration - duration in seconds
		vec3(200, 200, 255), // finalPotionColor - light blue color
		{
			{ ItemType::CACTUS_EXTRACT, 1, 0.0f }, // ingredients
			{ ItemType::COFFEE_BEANS, 3, 0.0f },    // ingredients (swiftbean)
			{ ItemType::STORM_BARK, 1, 0.0f },      // ingredients
			{ ItemType::MUMMY_BANDAGES, 1, 0.0f }, // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 100 }, // high heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add cactus extract
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 1 }, // add swiftbean
			{ ActionType::ADD_INGREDIENT, 3 }, // add mummy bandages
			{ ActionType::STIR, 4 },          // stir 4 times
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add storm bark
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 4 }           // wait 20 seconds
		},
		"Potion of Alkalescence",
		"[Throwable] Reacts with acidic substances. Neutralizes enemies that are highly acidic, as well as acidic mushrooms."
	},
	{
		PotionEffect::CLARITY,
		0.0f, 30.0f,  // highestQualityEffect - visibility duration
		0, 60,     // highestQualityDuration - duration in seconds
		vec3(220, 220, 100), // finalPotionColor - yellow color
		{
			{ ItemType::STORM_SAP, 2, 0.0f },  // ingredients
			{ ItemType::GLOWSHROOM, 3, 0.0f },  // ingredients
			{ ItemType::GLOWSPORE, 1, 0.0f },    // ingredients
			{ ItemType::DOOMCAP, 1, 0.0f }    // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 25 },  // low heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add storm sap
			{ ActionType::WAIT, 1 },          // wait 5 seconds
			{ ActionType::ADD_INGREDIENT, 1 }, // add glowshroom
			{ ActionType::ADD_INGREDIENT, 3 }, // add doomcap
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add glowspore
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 3 }           // wait 15 seconds
		},
		"Potion of Clarity",
		"[Throwable] Reveals hidden objects or ingredients. May be potent against enemies that are averse to the light."
	},
	{
		PotionEffect::REJUVENATION,
		0.0f, 100.0f, // highestQualityEffect - ultimate healing power
		0, 60,     // highestQualityDuration - duration in seconds
		vec3(255, 215, 0),  // finalPotionColor - golden color
		{
			{ ItemType::POTION, (int)PotionEffect::REGEN, 0.0f },
			{ ItemType::QUARTZMELON, 1, 0.0f },   // ingredients
			{ ItemType::CRYSTAL_MEPH, 1, 0.0f }, // ingredients
			{ ItemType::BLIGHTLEAF, 1, 0.0f },    // ingredients
			{ ItemType::HEALING_LILY, 1, 0.0f },  // ingredients
			{ ItemType::GLOWSHROOM, 2, 0.0f }     // ingredients
		},
		{
			{ ActionType::ADD_INGREDIENT, 0 }, // add regen potion
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::ADD_INGREDIENT, 1 }, // add quartzmelon
			{ ActionType::ADD_INGREDIENT, 2 }, // add crystal meph
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 3 }, // add blightleaf
			{ ActionType::ADD_INGREDIENT, 4 }, // add healing lily
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::ADD_INGREDIENT, 5 }, // add glowshroom
			{ ActionType::STIR, 4 },          // stir 4 times
			{ ActionType::WAIT, 6 }           // wait 30 seconds
		},
		"Potion of Rejuvenation",
		"[???] The ultimate healing potion."
	}
};

// Info for potion quality levels
// threshold: MINIMUM quality required to achieve this level
// normalized_quality: The quality that this level of potion will be normalized to
// name: The name to add to the front of the potion item
// star_texture_path: kinda obvious but if you are reading this have a nice day thank you
struct PotionQuality
{
	float threshold;
	float normalized_quality;
	std::string name;
	std::string star_texture_path;
};

// Define potion qualities from HIGHEST to LOWEST
// A quality that does not meet any defined threshold will be considered FAILED
const std::vector<PotionQuality> POTION_QUALITIES = {
	{ 0.95f, 1.0f, "Superior", "interactables/potion_quality/potion_quality_superior.png" },
	{ 0.75f, 0.8f, "Opulent", "interactables/potion_quality/potion_quality_opulent.png" },
	{ 0.55f, 0.6f, "Refined", "interactables/potion_quality/potion_quality_refined.png" },
	{ 0.35f, 0.4f, "Mellow", "interactables/potion_quality/potion_quality_mellow.png" },
	{ 0.15f, 0.2f, "Basic", "interactables/potion_quality/potion_quality_basic.png" }
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

// Cauldron settings
const int COLOR_FADE_DURATION = 5000;
const float CAULDRON_D = 316;               // cauldron is a circle, this is diameter
const vec2 CAULDRON_WATER_POS = vec2(0.4976f, 0.5757f); // center of cauldron relative to window
const int STIR_FLASH_DURATION = 1000;
const float WATER_FPS = 120.f;           // The default FPS to normalize water sim to

// Ladle offset coords for mouse and cauldron center
const vec2 LADLE_OFFSET = vec2(25, -55);

// The maximum UI degree change (both pos and neg) of the heat knob
const int MAX_KNOB_DEGREE = 60;

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

const float REGEN_TIMER = 1000.f;

enum class TUTORIAL {
	WELCOME_SCREEN = 0,
	TOGGLE_TUTORIAL = WELCOME_SCREEN + 1,
	RECIPE_BOOK = TOGGLE_TUTORIAL + 1,
	FLIP_PAGE = RECIPE_BOOK + 1,
	EXIT_GROTTO = FLIP_PAGE + 1,
	COLLECT_ITEMS = EXIT_GROTTO + 1,
	ENTER_GROTTO = COLLECT_ITEMS + 1,
	MORTAR_PESTLE = ENTER_GROTTO + 1,
	GRIND_BARK = MORTAR_PESTLE + 1,
	INTERACT_CAULDRON = GRIND_BARK + 1,
	SET_HEAT = INTERACT_CAULDRON + 1,
	ADD_INGREDIENTS = SET_HEAT + 1,
	STIR = ADD_INGREDIENTS + 1,
	WAIT = STIR + 1,
	BOTTLE = WAIT + 1,
	THROW_POTION = BOTTLE + 1,
	POTION_EFFECT = THROW_POTION + 1,
	COMPLETE = POTION_EFFECT + 1
};

enum class SOUND_CHANNEL {
	GENERAL = -1, // this indicates choosing any available channel
	BGM = 0,
	BOILING = BGM + 1,
	MENU = BOILING + 1
};

const std::vector<PotionEffect> throwable_potions = {
	PotionEffect::WATER,
	PotionEffect::DAMAGE,
	PotionEffect::MOLOTOV,
	PotionEffect::POISON,
	PotionEffect::CLARITY,
	PotionEffect::FAILED
};

const std::vector<PotionEffect> consumable_potions = {
	PotionEffect::SPEED,
	PotionEffect::HEALTH,
	PotionEffect::REGEN,
	PotionEffect::RESISTANCE,
	PotionEffect::SATURATION,
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

bool isUselessEffect(PotionEffect effect);
