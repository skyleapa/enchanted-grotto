#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
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
inline std::string shader_path(const std::string &name) { return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name; };
inline std::string textures_path(const std::string &name) { return data_path() + "/textures/" + std::string(name); };
inline std::string audio_path(const std::string &name) { return data_path() + "/audio/" + std::string(name); };
inline std::string mesh_path(const std::string &name) { return data_path() + "/meshes/" + std::string(name); };

//
// game constants
//
const int WINDOW_WIDTH_PX = 1250;
const int WINDOW_HEIGHT_PX = 700;

const int GRID_CELL_WIDTH_PX = 50;
const int GRID_CELL_HEIGHT_PX = 50;
const int GRID_LINE_WIDTH_PX = 2;

const float PLAYER_BB_WIDTH = (float)65;
const float PLAYER_BB_HEIGHT = (float)100;
const float PLAYER_SPEED = (float)100;

const float TREE_WIDTH = (float)GRID_CELL_WIDTH_PX * 4;
const float TREE_HEIGHT = (float)GRID_CELL_HEIGHT_PX * 4;

const float FOREST_RIVER_ABOVE_WIDTH = (float)200;
const float FOREST_RIVER_ABOVE_HEIGHT = (float)395;

const float FOREST_RIVER_BELOW_WIDTH = (float)200;
const float FOREST_RIVER_BELOW_HEIGHT = (float)153;

const float FOREST_BRIDGE_WIDTH = (float)284;
const float FOREST_BRIDGE_HEIGHT = (float)218;

const float GROTTO_ENTRANCE_WIDTH = (float)400;
const float GROTTO_ENTRANCE_HEIGHT = (float)180;

const float BUSH_WIDTH = (float)GRID_CELL_WIDTH_PX * 5.5;
const float BUSH_HEIGHT = (float)GRID_CELL_HEIGHT_PX * 5.5;

const float FRUIT_WIDTH = (float)GRID_CELL_WIDTH_PX * 1.5;
const float FRUIT_HEIGHT = (float)GRID_CELL_HEIGHT_PX * 1.8;

const float COFFEE_BEAN_WIDTH = (float)GRID_CELL_WIDTH_PX * 0.9;
const float COFFEE_BEAN_HEIGHT = (float)GRID_CELL_HEIGHT_PX * 0.9;

const float TEXTBOX_WIDTH = (float)GRID_CELL_WIDTH_PX * 4;
const float TEXTBOX_HEIGHT = (float)GRID_CELL_HEIGHT_PX * 4;

const float ITEM_PICKUP_RADIUS = (float)70;
const float TEXTBOX_VISIBILITY_RADIUS = (float)70;

// Item and potion names
const std::unordered_map<ItemType, std::string> ITEM_NAMES = {
	{ItemType::POTION, "Potion"},
	{ItemType::COFFEE_BEANS, "Coffee Beans"},
	{ItemType::MAGICAL_FRUIT, "Magical Fruit"}
};

const std::unordered_map<PotionEffect, std::string> EFFECT_NAMES = {
	{PotionEffect::FAILED, "Failed"},
	{PotionEffect::SPEED, "Speed"}
};

// RECIPE LIST
const std::vector<Recipe> RECIPES = {
	{
		.effect = PotionEffect::SPEED,
		.highestQualityEffect = 3.0,
		.highestQualityDuration = 180,
		.finalPotionColor = vec3(0, 255, 255),
		.ingredients = {
			{.type=ItemType::COFFEE_BEANS, .amount=5, .grindAmount=1},
			{.type=ItemType::MAGICAL_FRUIT, .amount=3, .grindAmount=0},
		},
		.steps = {
			{.type = ActionType::MODIFY_HEAT, .value = 1},
			{.type = ActionType::WAIT, .value = 2},
			{.type = ActionType::ADD_INGREDIENT, .value = 0},
			{.type = ActionType::ADD_INGREDIENT, .value = 1},
			{.type = ActionType::STIR, .value = 3},
			{.type = ActionType::WAIT, .value = 6},
			{.type = ActionType::BOTTLE},
		}
	}
};

// Default time represented by each "WAIT" action
const int DEFAULT_WAIT = 5;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform
{
	mat3 mat = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}}; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
