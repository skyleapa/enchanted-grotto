#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "ui_system.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count>  texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj")),
		// specify meshes of other assets here
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BRIDGE_TOP, mesh_path("bridge_top.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BRIDGE_BOTTOM, mesh_path("bridge_bottom.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::GROTTO_POOL, mesh_path("grotto_pool.obj")),
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::MUSHROOM_ACID_LAKE, mesh_path("mushroom_acid_lake.obj")),
	};

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("players/player_1.png"),
		textures_path("players/walking_a/walking_a_1.png"),
		textures_path("players/walking_a/walking_a_2.png"),
		textures_path("players/walking_a/walking_a_3.png"),
		textures_path("players/walking_a/walking_a_4.png"),
		textures_path("players/walking_d/walking_d_1.png"),
		textures_path("players/walking_d/walking_d_2.png"),
		textures_path("players/walking_d/walking_d_3.png"),
		textures_path("players/walking_d/walking_d_4.png"),
		textures_path("players/walking_s/walking_s_1.png"),
		textures_path("players/walking_s/walking_s_2.png"),
		textures_path("players/walking_s/walking_s_3.png"),
		textures_path("players/walking_s/walking_s_4.png"),
		textures_path("players/walking_w/walking_w_1.png"),
		textures_path("players/walking_w/walking_w_2.png"),
		textures_path("players/walking_w/walking_w_3.png"),
		textures_path("players/walking_w/walking_w_4.png"),
		textures_path("terrain/forest/forest_bridge.png"),
		textures_path("terrain/forest/bridge_top.png"),
		textures_path("terrain/forest/bridge_bottom.png"),
		textures_path("terrain/forest/forest_river_top.png"),
		textures_path("terrain/forest/forest_river_bottom.png"),
		textures_path("backgrounds/forest_bg.png"),
		textures_path("terrain/forest/forest_to_desert.png"),
		textures_path("terrain/forest/forest_to_mushroom.png"),
		textures_path("terrain/forest/tree_1.png"),
		textures_path("backgrounds/forest_ex_bg.png"),
		textures_path("terrain/forest/grotto_entrance.png"),
		textures_path("backgrounds/grotto_bg.png"),
		textures_path("terrain/grotto/grotto_carpet.png"),
		textures_path("terrain/grotto/grotto_cauldron.png"),
		textures_path("terrain/grotto/grotto_chest.png"),
		textures_path("terrain/grotto/grotto_mortar_pestle.png"),
		textures_path("terrain/grotto/grotto_pool.png"),
		textures_path("terrain/grotto/grotto_recipe_book.png"),
		textures_path("terrain/grotto/grotto_right_bookshelves.png"),
		textures_path("terrain/grotto/grotto_top_bookshelves.png"),
		textures_path("backgrounds/desert_bg.png"),
		textures_path("terrain/desert/desert_cactus.png"),
		textures_path("terrain/desert/desert_river.png"),
		textures_path("terrain/desert/desert_skull.png"),
		textures_path("terrain/desert/desert_tree.png"),
		textures_path("terrain/desert/desert_to_forest.png"),
		textures_path("terrain/desert/desert_sand_pile_page.png"),
		textures_path("backgrounds/mushroom_bg.png"),
		textures_path("terrain/mushroom/mushroom_acid_lake.png"),
		textures_path("terrain/mushroom/mushroom_blue.png"),
		textures_path("terrain/mushroom/mushroom_pink.png"),
		textures_path("terrain/mushroom/mushroom_purple.png"),
		textures_path("terrain/mushroom/mushroom_tall_blue.png"),
		textures_path("terrain/mushroom/mushroom_tall_pink.png"),
		textures_path("backgrounds/crystal_bg.png"),
		textures_path("terrain/crystal/crystal_1.png"),
		textures_path("terrain/crystal/crystal_2.png"),
		textures_path("terrain/crystal/crystal_3.png"),
		textures_path("terrain/crystal/crystal_4.png"),
		textures_path("terrain/crystal/crystal_minecart.png"),
		textures_path("terrain/crystal/crystal_page.png"),
		textures_path("terrain/crystal/crystal_rock.png"),
		textures_path("terrain/boundary_transparent.png"),
		textures_path("terrain/forest/bush.png"),
		textures_path("interactables/magical_fruit.png"),
		textures_path("interactables/coffee_bean.png"),
		textures_path("interactables/sap.png"),
		textures_path("interactables/magical_dust.png"),
		textures_path("interactables/textbox_magical_fruit.png"),
		textures_path("interactables/textbox_coffee_bean.png"),
		textures_path("interactables/textbox_enter_grotto.png"),
		textures_path("interactables/textbox_grotto_exit.png"),
		textures_path("interactables/textbox_cauldron.png"),
		textures_path("interactables/textbox_enter_desert.png"),
		textures_path("interactables/textbox_enter_forest.png"),
		textures_path("interactables/textbox_twig.png"),
		textures_path("interactables/textbox_magical_dust.png"),
		textures_path("enemies/ent.png"),
		textures_path("enemies/mummy.png"),
		textures_path("interactables/potion_item.png"),
		textures_path("welcome_to_grotto.png")
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("chicken"),
		shader_path("textured"),
		shader_path("background"),
		shader_path("fade"),
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();

	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the vignette shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	// Include UI system so we can specify order
	void draw(UISystem* ui_system);

	// Swap the frame buffers to display rendered content
	void swap_buffers();

	std::vector<Entity> process_render_requests();

	mat3 createProjectionMatrix();

	Entity get_screen_state_entity() { return screen_state_entity; }

	void updateViewport() { glViewport(viewport_x, viewport_y, viewport_sizex, viewport_sizey); }

	void setViewportCoords(int x, int y, int sizex, int sizey) {
		viewport_x = x, viewport_y = y, viewport_sizex = sizex, viewport_sizey = sizey;
	}

	float getRetinaScale() { return retina_scale; }

private:
	// Internal drawing functions for each entity type
	void drawGridLine(Entity entity, const mat3& projection);
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();
	void fadeScreen();

	// Viewport numbers
	int viewport_x;
	int viewport_y;
	int viewport_sizex;
	int viewport_sizey;
	float retina_scale = 1.0f; // 1.0 on windows, 2.0 on mac

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
