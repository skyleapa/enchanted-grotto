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
inline std::string game_state_path(const std::string& name) { return data_path() + "/game_states/v1/" + std::string(name); };

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

const float DETECTION_RADIUS = (float)200;  // Enemy starts moving & attacking
const float FOLLOWING_RADIUS = (float)300;  // Enemy stops attacking if outside this

const float ENEMY_SPEED = (float)110;

const int THROW_DISTANCE = 300; // Player throw dist in pixels

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
	GLOWSPORE = CRYSTAL_MEPH + 1
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
// TENACITY: Increases inventory capacity
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
	TENACITY = REGEN + 1,
	POISON = TENACITY + 1,
	RESISTANCE = POISON + 1,
	SATURATION = RESISTANCE + 1,
	ALKALESCENCE = SATURATION + 1,
	CLARITY = ALKALESCENCE + 1,
	REJUVENATION = CLARITY + 1
};

const std::unordered_map<PotionEffect, std::string> EFFECT_NAMES = {
	{PotionEffect::FAILED, "Failed"},
	{PotionEffect::WATER, "Water"},
	{PotionEffect::SPEED, "Speed"},
	{PotionEffect::HEALTH, "Health"},
	{PotionEffect::DAMAGE, "Damage"},
	{PotionEffect::MOLOTOV, "Molotov"},
	{PotionEffect::REGEN, "Regen"},
	{PotionEffect::TENACITY, "Tenacity"},
	{PotionEffect::POISON, "Poison"},
	{PotionEffect::RESISTANCE, "Resistance"},
	{PotionEffect::SATURATION, "Saturation"},
	{PotionEffect::ALKALESCENCE, "Alkalescence"},
	{PotionEffect::CLARITY, "Clarity"},
	{PotionEffect::REJUVENATION, "Rejuvenation"}
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
		3.5f,  // highestQualityEffect - maximum speed multiplier
		150,   // highestQualityDuration - maximum duration in seconds
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
		}
	},
	{
		PotionEffect::HEALTH,
		40.0f,  // highestQualityEffect - maximum health restored
		0,      // highestQualityDuration - instant effect, no duration
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
		}
	},
	{
		PotionEffect::DAMAGE,
		50.0f,  // highestQualityEffect - maximum damage to enemies
		0,      // highestQualityDuration - instant effect, no duration
		vec3(100, 0, 100), // finalPotionColor - purple color
		{
			{ ItemType::BLIGHTLEAF, 2, 0.0f }, // ingredients
			{ ItemType::STORM_BARK, 1, 0.0f }  // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 100 }, // high heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add blightleaf
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 1 }, // add storm bark
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 3 }           // wait 15 seconds
		}
	},
	{
		PotionEffect::MOLOTOV,
		25.0f,  // highestQualityEffect - burn damage per second
		25,     // highestQualityDuration - burn duration in seconds
		vec3(255, 100, 0), // finalPotionColor - orange/fire color
		{
			{ ItemType::POTION, 1, 0.0f },       // TODO: 1x damage potion
			{ ItemType::MUMMY_BANDAGES, 2, 0.0f }, // ingredients
			{ ItemType::PETRIFIED_BONE, 4, 1.0f }  // ingredients (must be ground)
		},
		{
			{ ActionType::MODIFY_HEAT, 25 },  // low heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add damage potion
			{ ActionType::ADD_INGREDIENT, 1 }, // add mummy bandages
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::ADD_INGREDIENT, 2 }, // add petrified bones
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 4 }           // wait 20 seconds
		}
	},
	{
		PotionEffect::REGEN,
		5.0f,   // highestQualityEffect - health regen per second
		30,     // highestQualityDuration - regen duration in seconds
		vec3(200, 50, 50), // finalPotionColor - pink-red color
		{
			{ ItemType::POTION, 1, 0.0f },       // TODO: 1x health potion
			{ ItemType::HEALING_LILY, 2, 0.0f }, // ingredients
			{ ItemType::EVERFERN, 1, 0.0f }     // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add health potion
			{ ActionType::ADD_INGREDIENT, 1 }, // add healing lily
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::ADD_INGREDIENT, 2 }, // add everfern
			{ ActionType::WAIT, 3 },          // wait 15 seconds
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 3 }           // wait 15 seconds
		}
	},
	{
		PotionEffect::TENACITY,
		3.0f,   // highestQualityEffect - additional inventory slots
		0,      // highestQualityDuration - permanent effect
		vec3(75, 150, 75), // finalPotionColor - green color
		{
			{ ItemType::CACTUS_EXTRACT, 1, 0.0f }, // ingredients
			{ ItemType::POTION, 2, 0.0f },         // 2x regen potion
			{ ItemType::PETRIFIED_BONE, 2, 0.0f }  // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 25 },  // low heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add cactus extract
			{ ActionType::WAIT, 1 },          // wait 5 seconds
			{ ActionType::ADD_INGREDIENT, 1 }, // add regen potions
			{ ActionType::ADD_INGREDIENT, 2 }, // add petrified bones
			{ ActionType::STIR, 4 },          // stir 4 times
			{ ActionType::WAIT, 4 }           // wait 20 seconds
		}
	},
	{
		PotionEffect::POISON,
		8.0f,   // highestQualityEffect - poison damage per second
		20,     // highestQualityDuration - poison duration in seconds
		vec3(0, 150, 0), // finalPotionColor - green color
		{
			{ ItemType::POTION, 1, 0.0f },        // TODO:1x damage potion
			{ ItemType::BLIGHTLEAF, 2, 0.0f },    // ingredients
			{ ItemType::DOOMCAP, 2, 0.0f }      // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add damage potion
			{ ActionType::ADD_INGREDIENT, 1 }, // add blightleaf
			{ ActionType::WAIT, 1 },          // wait 5 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add doomcap
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 5 }           // wait 25 seconds
		}
	},
	{
		PotionEffect::RESISTANCE,
		0.5f,   // highestQualityEffect - damage reduction multiplier (0.5 = 50% reduced damage)
		30,     // highestQualityDuration - resistance duration in seconds
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
		}
	},
	{
		PotionEffect::SATURATION,
		2.0f,   // highestQualityEffect - potion effect multiplier (2x at max quality)
		300,    // highestQualityDuration - duration in seconds
		vec3(200, 100, 200), // finalPotionColor - purple/pink color
		{
			{ ItemType::GALEFRUIT, 2, 0.0f },   // ingredients
			{ ItemType::STORM_BARK, 2, 0.0f },  // ingredients
			{ ItemType::EVERFERN, 1, 0.0f }     // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::ADD_INGREDIENT, 1 }, // add storm bark
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add everfern
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 0 }, // add galefruit
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 5 }           // wait 25 seconds
		}
	},
	{
		PotionEffect::ALKALESCENCE,
		10.0f,  // highestQualityEffect - reactive strength
		20,     // highestQualityDuration - duration in seconds
		vec3(200, 200, 255), // finalPotionColor - light blue color
		{
			{ ItemType::CACTUS_EXTRACT, 1, 0.0f }, // ingredients
			{ ItemType::COFFEE_BEANS, 3, 0.0f },    // ingredients (swiftbean)
			{ ItemType::STORM_BARK, 1, 0.0f }      // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 100 }, // high heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add cactus extract
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 1 }, // add swiftbean
			{ ActionType::STIR, 4 },          // stir 4 times
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add storm bark
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 4 }           // wait 20 seconds
		}
	},
	{
		PotionEffect::CLARITY,
		30.0f,  // highestQualityEffect - visibility duration
		60,     // highestQualityDuration - duration in seconds
		vec3(220, 220, 100), // finalPotionColor - yellow color
		{
			{ ItemType::STORM_BARK, 2, 0.0f },  // ingredients
			{ ItemType::GLOWSHROOM, 3, 0.0f },  // ingredients
			{ ItemType::GALEFRUIT, 1, 0.0f }    // ingredients
		},
		{
			{ ActionType::MODIFY_HEAT, 25 },  // low heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add storm bark
			{ ActionType::WAIT, 1 },          // wait 5 seconds
			{ ActionType::ADD_INGREDIENT, 1 }, // add glowshroom
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add galefruit
			{ ActionType::STIR, 2 },          // stir 2 times
			{ ActionType::WAIT, 3 }           // wait 15 seconds
		}
	},
	{
		PotionEffect::REJUVENATION,
		100.0f, // highestQualityEffect - ultimate healing power
		60,     // highestQualityDuration - duration in seconds
		vec3(255, 215, 0),  // finalPotionColor - golden color
		{
			{ ItemType::QUARTZMELON, 1, 0.0f },   // ingredients
			{ ItemType::CRYSTAL_SHARD, 1, 0.0f }, // ingredients (will be meph later)
			{ ItemType::BLIGHTLEAF, 1, 0.0f },    // ingredients
			{ ItemType::HEALING_LILY, 1, 0.0f },  // ingredients
			{ ItemType::GLOWSHROOM, 2, 0.0f },    // ingredients
			{ ItemType::POTION, 1, 0.0f }         // TODO: 1x regen potion
		},
		{
			{ ActionType::MODIFY_HEAT, 50 },  // medium heat
			{ ActionType::ADD_INGREDIENT, 0 }, // add quartzmelon
			{ ActionType::ADD_INGREDIENT, 1 }, // add crystal shard
			{ ActionType::WAIT, 2 },          // wait 10 seconds
			{ ActionType::ADD_INGREDIENT, 2 }, // add blightleaf
			{ ActionType::ADD_INGREDIENT, 3 }, // add healing lily
			{ ActionType::STIR, 3 },          // stir 3 times
			{ ActionType::ADD_INGREDIENT, 4 }, // add glowshroom
			{ ActionType::ADD_INGREDIENT, 5 }, // add regen potion
			{ ActionType::STIR, 4 },          // stir 4 times
			{ ActionType::WAIT, 6 }           // wait 30 seconds
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
	// PotionEffect::TENACITY, // to be implemented in M4
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
