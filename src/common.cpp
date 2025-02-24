#include "common.hpp"
#include "tinyECS/components.hpp"

// Note, we could also use the functions from GLM but we write the transformations here to show the uderlying math
void Transform::scale(vec2 scale)
{
	mat3 S = { {scale.x, 0.f, 0.f}, {0.f, scale.y, 0.f}, {0.f, 0.f, 1.f} };
	mat = mat * S;
}

void Transform::rotate(float radians)
{
	float c = cosf(radians);
	float s = sinf(radians);
	mat3 R = { {c, s, 0.f}, {-s, c, 0.f}, {0.f, 0.f, 1.f} };
	mat = mat * R;
}

void Transform::translate(vec2 offset)
{
	mat3 T = { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {offset.x, offset.y, 1.f} };
	mat = mat * T;
}

bool gl_has_errors()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return false;

	while (error != GL_NO_ERROR)
	{
		const char* error_str = "";
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		fprintf(stderr, "OpenGL: %s", error_str);
		error = glGetError();
		assert(false);
	}

	return true;
}

const std::map<int, std::vector<std::tuple<vec2, vec2>>> biome_boundaries = {
	{(int)BIOME::GROTTO,
	 {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX * 4), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
	  std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
	  std::make_tuple(vec2(0, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
	  std::make_tuple(vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
	  std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 20.5, GRID_CELL_HEIGHT_PX * 13.6), vec2(190, BOUNDARY_LINE_THICKNESS)), // door
	  std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 13.30, GRID_CELL_HEIGHT_PX * 7.5), vec2(GRID_CELL_WIDTH_PX * 3.7, GRID_CELL_HEIGHT_PX * 1.5)), // cauldron 
	  std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 4.75, GRID_CELL_HEIGHT_PX * 6.35), vec2(460, BOUNDARY_LINE_THICKNESS)), // tools bottom
	  std::make_tuple(vec2(475, 260), vec2(BOUNDARY_LINE_THICKNESS, 110)) // tools right
	}},
	{(int)BIOME::FOREST,
	 {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX * 1.3), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
	  std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
	  std::make_tuple(vec2(0, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
	  std::make_tuple(vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
	  std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 2), vec2(GRID_CELL_WIDTH_PX * 5, BOUNDARY_LINE_THICKNESS)), // grotto bottom
	  std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 17.2, GRID_CELL_HEIGHT_PX), vec2(BOUNDARY_LINE_THICKNESS, GRID_CELL_HEIGHT_PX * 2)), // grotto left
	  std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 22.8, GRID_CELL_HEIGHT_PX), vec2(BOUNDARY_LINE_THICKNESS, GRID_CELL_HEIGHT_PX * 2))}} }; // grotto right

const std::vector<std::tuple<vec2, vec2, float, GLuint, int>> grotto_static_entity_pos = {
	{vec2({GRID_CELL_WIDTH_PX * 20.5, GRID_CELL_HEIGHT_PX * 9}), vec2({180, 310}), 0, (GLuint)TEXTURE_ASSET_ID::GROTTO_CARPET, 0},
	{vec2({GRID_CELL_WIDTH_PX * 21, GRID_CELL_HEIGHT_PX * 3}), vec2({335, 260}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_TOP_BOOKSHELF, 1},
	{vec2({GRID_CELL_WIDTH_PX * 24.2, GRID_CELL_HEIGHT_PX * 9.1}), vec2({90, 429}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_RIGHT_BOOKSHELF, 1},
	{vec2({GRID_CELL_WIDTH_PX * 4.8, GRID_CELL_HEIGHT_PX * 11.9}), vec2({510, 215}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_POOL, 1} };