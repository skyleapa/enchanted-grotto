// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <unordered_set>

#include "physics_system.hpp"

// create the world
WorldSystem::WorldSystem() : points(0)
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (chicken_dead_sound != nullptr)
		Mix_FreeChunk(chicken_dead_sound);
	if (chicken_eat_sound != nullptr)
		Mix_FreeChunk(chicken_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE); // GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Enchanted Grotto", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
	auto mouse_button_pressed_redirect = [](GLFWwindow *wnd, int _button, int _action, int _mods)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	return window;
}

bool WorldSystem::start_and_load_sounds()
{

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());
	chicken_eat_sound = Mix_LoadWAV(audio_path("chicken_eat.wav").c_str());

	if (background_music == nullptr || chicken_dead_sound == nullptr || chicken_eat_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("music.wav").c_str(),
				audio_path("chicken_dead.wav").c_str(),
				audio_path("chicken_eat.wav").c_str());
		return false;
	}
	return true;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{

	this->renderer = renderer_arg;

	// start playing background music indefinitely
	std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_music, -1);

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{

	// handle switching biomes
	ScreenState &screen = registry.screenStates.components[0];
	Entity &player = registry.players.entities[0];
	assert(registry.motions.has(player)); // TODO: remove assert on submission
	Motion &player_motion = registry.motions.get(player);

	if (screen.is_switching_biome)
	{
		if (screen.freeze_timer > 0)
		{
			screen.freeze_timer -= elapsed_ms_since_last_update; // fade will occur here
			if (screen.darken_screen_factor <= 1) screen.darken_screen_factor += elapsed_ms_since_last_update * 0.001f;
		}

		if (screen.freeze_timer <= 1000)
		{
			// at this point screen is fully black then fades in
			if (screen.biome != screen.switching_to_biome)
			{ // restart with new biome only once
				screen.biome = screen.switching_to_biome;
				restart_game();
				screen.darken_screen_factor = 1;
				if (screen.biome == (GLuint)BIOME::GROTTO)
				{
					player_motion.scale = {PLAYER_BB_WIDTH * 1.3, PLAYER_BB_HEIGHT * 1.3}; // make player larger in grotto
				}
				else
				{
					player_motion.scale = {PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT};
				}
			}
			if (screen.darken_screen_factor >= 0) screen.darken_screen_factor -= elapsed_ms_since_last_update * 0.001f;
		}
		else if (screen.freeze_timer <= 0)
		{
			screen.darken_screen_factor = 0;
			screen.is_switching_biome = false;
			screen.freeze_timer = 2000;
			Entity &player = registry.players.entities[0];
			Motion &player_motion = registry.motions.get(player);
			player_motion.velocity = {PLAYER_SPEED, PLAYER_SPEED};
			screen.pressed_keys = {};
		}
	}

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i)
	{
		Motion &motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f)
		{
			if (!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// move the character
	if (registry.moving.has(player) && registry.motions.has(player))
	{
		player_motion.previous_position = player_motion.position;
		float delta = elapsed_ms_since_last_update * 0.001f;

		if (player_motion.moving_direction == (int)DIRECTION::UP)
		{
			player_motion.position.y -= player_motion.velocity.y * delta;
		}
		else if (player_motion.moving_direction == (int)DIRECTION::DOWN)
		{
			player_motion.position.y += player_motion.velocity.y * delta;
		}
		else if (player_motion.moving_direction == (int)DIRECTION::RIGHT)
		{
			player_motion.position.x += player_motion.velocity.x * delta;
		}
		else if (player_motion.moving_direction == (int)DIRECTION::LEFT)
		{
			player_motion.position.x -= player_motion.velocity.x * delta;
		}
	}

	// assert(registry.screenStates.components.size() <= 1);
	// ScreenState &screen = registry.screenStates.components[0];

	float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities)
	{
		// progress timer
		DeathTimer &counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms)
		{
			min_counter_ms = counter.counter_ms;
		}
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset the game speed
	current_speed = 1.f;

	points = 0;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();

	int grid_line_width = GRID_LINE_WIDTH_PX;

	// create grid lines if they do not already exist
	if (grid_lines.size() == 0)
	{
		// vertical lines
		int cell_width = GRID_CELL_WIDTH_PX;
		for (int col = 0; col < 24 + 1; col++)
		{
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(col * cell_width, 0), vec2(grid_line_width, 2 * WINDOW_HEIGHT_PX)));
		}

		// horizontal lines
		int cell_height = GRID_CELL_HEIGHT_PX;
		for (int col = 0; col < 13 + 1; col++)
		{
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(0, col * cell_height), vec2(2 * WINDOW_WIDTH_PX, grid_line_width)));
		}
	}

	if (registry.players.components.size() == 0)
	{
		createPlayer(renderer, vec2(GROTTO_ENTRANCE_X, GROTTO_ENTRANCE_Y + 50));
	}

	int biome = registry.screenStates.components[0].biome;
	if (biome == (GLuint)BIOME::FOREST)
	{
		create_forest();
	}
	else if (biome == (GLuint)BIOME::GROTTO)
	{
	}
}

void WorldSystem::create_forest()
{
	// create forest bridge
	createForestBridge(renderer, vec2(307, 485));

	// create forest river
	createForestRiver(renderer, vec2(307, 0));

	// create trees if they don't exist
	if (trees.size() == 0)
	{
		trees.push_back(createTree(renderer, vec2(GRID_CELL_WIDTH_PX * 11, GRID_CELL_HEIGHT_PX * 3)));
		trees.push_back(createTree(renderer, vec2(GRID_CELL_WIDTH_PX * 19, GRID_CELL_HEIGHT_PX * 10)));
		trees.push_back(createTree(renderer, vec2(GRID_CELL_WIDTH_PX * 23, GRID_CELL_HEIGHT_PX * 11)));
	}

	createGrottoEntrance(renderer, vec2(990, 90));
}

void WorldSystem::handle_collisions()
{
	// get our player entity
	if (registry.players.entities.empty())
		return;
	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity))
		return;

	Motion &player_motion = registry.motions.get(player_entity);

	for (Entity collision_entity : registry.collisions.entities)
	{
		if (!registry.collisions.has(collision_entity))
			continue;
		Collision &collision = registry.collisions.get(collision_entity);

		Entity terrain_entity = (collision_entity == player_entity) ? collision.other : collision_entity;

		if (!registry.terrains.has(terrain_entity) || !registry.motions.has(terrain_entity))
			continue;

		Terrain &terrain = registry.terrains.get(terrain_entity);
		vec2 movement = player_motion.position - player_motion.previous_position;

		if (std::abs(movement.x) > std::abs(movement.y))
		{
			// horizontal collision -> stop X movement, allow Y movement
			player_motion.position.x = player_motion.previous_position.x;
		}
		else
		{
			// vertical collision -> stop Y movement, allow X movement
			player_motion.position.y = player_motion.previous_position.y;
		}

		// when collision_setting == 1, entire area should be unwalkable (rivers)
		if (terrain.collision_setting == 1.0f)
		{
			player_motion.position = player_motion.previous_position;
		}
	}

	// Clear all collisions after processing this step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
	{
		close_window();
	}

	// Debugging - not used in A1, but left intact for the debug lines
	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_RELEASE)
		{
			if (debugging.in_debug_mode)
			{
				debugging.in_debug_mode = false;
			}
			else
			{
				debugging.in_debug_mode = true;
			}
		}
	}

	if (registry.screenStates.components[0].is_switching_biome || (key != GLFW_KEY_W && key != GLFW_KEY_S && key != GLFW_KEY_D && key != GLFW_KEY_A))
	{
		return;
	}

	Entity player = registry.players.entities[0]; // Assume only one player entity
	if (!registry.motions.has(player))
	{
		return;
	}
	Motion &player_motion = registry.motions.get(player);
	std::vector<int> &pressed_keys = registry.screenStates.components[0].pressed_keys;

	if (action == GLFW_PRESS)
	{
		pressed_keys.push_back(key); // Add key to set
		if (!registry.moving.has(player))
		{
			registry.moving.emplace(player); // Register movement
		}

		// TODO: map enums with key to direction
		if (key == GLFW_KEY_W)
		{
			player_motion.moving_direction = (int)DIRECTION::UP;
		}
		else if (key == GLFW_KEY_S)
		{
			player_motion.moving_direction = (int)DIRECTION::DOWN;
		}
		else if (key == GLFW_KEY_D)
		{
			player_motion.moving_direction = (int)DIRECTION::RIGHT;
		}
		else if (key == GLFW_KEY_A)
		{
			player_motion.moving_direction = (int)DIRECTION::LEFT;
		}
	}
	if (action == GLFW_RELEASE)
	{
		pressed_keys.erase(
			std::remove(pressed_keys.begin(), pressed_keys.end(), key),
			pressed_keys.end());

		if (pressed_keys.empty() && registry.moving.has(player))
		{
			registry.moving.remove(player); // Stop movement only when all keys are released
		}
		else
		{
			int old_key = pressed_keys[pressed_keys.size() - 1]; // get the last direction
			if (old_key == GLFW_KEY_W)
			{
				player_motion.moving_direction = (int)DIRECTION::UP;
			}
			else if (old_key == GLFW_KEY_S)
			{
				player_motion.moving_direction = (int)DIRECTION::DOWN;
			}
			else if (old_key == GLFW_KEY_D)
			{
				player_motion.moving_direction = (int)DIRECTION::RIGHT;
			}
			else if (old_key == GLFW_KEY_A)
			{
				player_motion.moving_direction = (int)DIRECTION::LEFT;
			}
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{

	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods)
{
	// on button press
	if (action == GLFW_PRESS)
	{
		int tile_x = (int)(mouse_pos_x / GRID_CELL_WIDTH_PX);
		int tile_y = (int)(mouse_pos_y / GRID_CELL_HEIGHT_PX);

		std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
		std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;
	}
}