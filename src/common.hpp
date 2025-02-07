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
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};

//
// game constants
//
const int WINDOW_WIDTH_PX = 1250;
const int WINDOW_HEIGHT_PX = 700;

const int GRID_CELL_WIDTH_PX = 50;
const int GRID_CELL_HEIGHT_PX = 50;
const int GRID_LINE_WIDTH_PX = 2;

const int TOWER_TIMER_MS = 1000;	// number of milliseconds between tower shots
const int MAX_TOWERS_START = 5;

const int INVADER_HEALTH = 50;
const int INVADER_SPAWN_RATE_MS = 2 * 1000;

const int PROJECTILE_DAMAGE = 10;

// These are hard coded to the dimensions of the entity's texture

// invaders are 64x64 px, but cells are 60x60
const float INVADER_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float INVADER_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

// towers are 64x64 px, but cells are 60x60
const float TOWER_BB_WIDTH = (float)GRID_CELL_WIDTH_PX;
const float TOWER_BB_HEIGHT = (float)GRID_CELL_HEIGHT_PX;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
