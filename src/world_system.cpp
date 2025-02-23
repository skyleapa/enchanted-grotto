// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "common.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <unordered_set>

#include "physics_system.hpp"

// create the world
WorldSystem::WorldSystem()
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
	void glfw_err_cb(int error, const char* desc)
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
GLFWwindow* WorldSystem::create_window()
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
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3)
		{ ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1)
		{ ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods)
		{ ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };

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

void WorldSystem::init(RenderSystem* renderer_arg)
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
	ScreenState& screen = registry.screenStates.components[0];
	if (registry.players.entities.size() < 1)
		return true;
	Entity& player = registry.players.entities[0];
	if (!registry.motions.has(player))
		return true;
	Motion& player_motion = registry.motions.get(player);

	if (screen.is_switching_biome)
	{
		if (screen.fade_status == 0)
		{
			screen.darken_screen_factor += elapsed_ms_since_last_update * TIME_UPDATE_FACTOR;
			if (screen.darken_screen_factor >= 1)
				screen.fade_status = 1; // after fade out
		}
		else if (screen.fade_status == 1)
		{
			if (screen.biome != screen.switching_to_biome)
			{
				screen.biome = screen.switching_to_biome;
				restart_game();
				screen.darken_screen_factor = 1;
				if (screen.biome == (GLuint)BIOME::GROTTO)
				{
					player_motion.scale = { PLAYER_BB_WIDTH * PlAYER_BB_GROTTO_SIZE_FACTOR, PLAYER_BB_HEIGHT * PlAYER_BB_GROTTO_SIZE_FACTOR };
					player_motion.position = vec2({ player_motion.position.x, GRID_CELL_HEIGHT_PX * 11 }); // bring player to front of door
				}
				else
				{
					player_motion.scale = { PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT };
				}
			}
			screen.darken_screen_factor -= elapsed_ms_since_last_update * TIME_UPDATE_FACTOR;
			if (screen.darken_screen_factor <= 0)
				screen.fade_status = 2; // after fade in
		}
		else
		{
			// complete biome switch
			screen.darken_screen_factor = 0;
			screen.is_switching_biome = false;
			screen.fade_status = 0;
			screen.pressed_keys = {};
		}
		return true; // don't need to do any other steps
	}
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i)
	{
		Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f)
		{
			if (!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// move the character
	if (registry.moving.has(player))
	{
		player_motion.previous_position = player_motion.position;
		float delta = elapsed_ms_since_last_update * TIME_UPDATE_FACTOR;

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

	update_textbox_visibility();
	handle_item_respawn(elapsed_ms_since_last_update);

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
		create_grotto();
	}
}

void WorldSystem::create_forest()
{
	// create boundaries
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::FOREST))
	{
		create_boundary_line(renderer, position, scale);
	}

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

	createGrottoEntrance(renderer, vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 1), 7, "Grotto Entrance");

	createBush(renderer, vec2(GRID_CELL_WIDTH_PX * 11, GRID_CELL_HEIGHT_PX * 11.5));

	createFruit(renderer, vec2(GRID_CELL_WIDTH_PX * 10, GRID_CELL_HEIGHT_PX * 3), 1, "Magical Fruit", 1);
	createFruit(renderer, vec2(GRID_CELL_WIDTH_PX * 19, GRID_CELL_HEIGHT_PX * 10), 2, "Magical Fruit", 1);
	createFruit(renderer, vec2(GRID_CELL_WIDTH_PX * 23, GRID_CELL_HEIGHT_PX * 11), 3, "Magical Fruit", 1);

	createCoffeeBean(renderer, vec2(GRID_CELL_WIDTH_PX * 11, GRID_CELL_HEIGHT_PX * 11.5), 4, "Coffee Bean", 1);
	createCoffeeBean(renderer, vec2(GRID_CELL_WIDTH_PX * 9.9, GRID_CELL_HEIGHT_PX * 12.1), 5, "Coffee Bean", 1);
	createCoffeeBean(renderer, vec2(GRID_CELL_WIDTH_PX * 12, GRID_CELL_HEIGHT_PX * 12.7), 6, "Coffee Bean", 1);
}

void WorldSystem::create_grotto()
{
	// positions are according to sample grotto interior
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::GROTTO))
	{
		create_boundary_line(renderer, position, scale);
	}

	for (const auto& [position, size, rotation, texture, layer] : grotto_static_entity_pos) {
		create_grotto_static_entities(renderer, position, size, rotation, texture, layer);
	}

	create_cauldron(renderer, vec2({ GRID_CELL_WIDTH_PX * 13.35, GRID_CELL_HEIGHT_PX * 5.85 }), vec2({ 175, 280 }), 8, "Cauldron");
	createMortarPestle(renderer, vec2({ GRID_CELL_WIDTH_PX * 7.5, GRID_CELL_HEIGHT_PX * 5.22 }), vec2({ 213, 141 }), 9, "Mortar and Pestle");
	createRecipeBook(renderer, vec2({ GRID_CELL_WIDTH_PX * 4.15, GRID_CELL_HEIGHT_PX * 5.05 }), vec2({ 108, 160 }), 10, "Recipe Book");
	createChest(renderer, vec2({ GRID_CELL_WIDTH_PX * 1.35, GRID_CELL_HEIGHT_PX * 5.2 }), vec2({ 100, 150 }), 11, "Chest");
}

void WorldSystem::handle_collisions()
{
	// Collision Resolution - Ensure player cannot go into bounding box, but can move around it

	// get our player entity
	if (registry.players.entities.empty())
		return;
	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity))
		return;

	Motion& player_motion = registry.motions.get(player_entity);

	for (Entity collision_entity : registry.collisions.entities)
	{
		if (!registry.collisions.has(collision_entity))
			continue;
		Collision& collision = registry.collisions.get(collision_entity);

		Entity terrain_entity = (collision_entity == player_entity) ? collision.other : collision_entity;

		if (!registry.terrains.has(terrain_entity) || !registry.motions.has(terrain_entity))
			continue;

		Terrain& terrain = registry.terrains.get(terrain_entity);
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

	if (action == GLFW_PRESS && key == GLFW_KEY_F)
	{
		handle_player_interaction();
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
	Motion& player_motion = registry.motions.get(player);
	std::vector<int>& pressed_keys = registry.screenStates.components[0].pressed_keys;

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

void WorldSystem::handle_player_interaction()
{
	// Check if player exists
	if (registry.players.entities.empty())
		return;

	Entity player = registry.players.entities[0]; // Assume single-player
	if (!registry.motions.has(player) || !registry.inventories.has(player))
		return;

	Motion& player_motion = registry.motions.get(player);

	if (registry.items.entities.empty())
		return;

	for (Entity item : registry.items.entities)
	{
		if (!registry.items.has(item) || !registry.motions.has(item))
			continue;

		Motion& item_motion = registry.motions.get(item);
		Item& item_info = registry.items.get(item);

		// Check if item is collectable or is an interactable entrance
		if (!item_info.isCollectable && !registry.entrances.has(item))
			continue;

		float distance = glm::distance(player_motion.position, item_motion.position);

		// If item is not in pickup range, continue
		if (distance > ITEM_PICKUP_RADIUS)
			continue;

		// Handle interaction
		bool handle_textbox = false;
		if (registry.entrances.has(item))
		{
			handle_textbox = handle_entrance_interaction(item);
		}
		else if (item_info.isCollectable)
		{
			handle_textbox = handle_item_pickup(player, item);
		}

		if (handle_textbox)
		{
			// Set textbox to invisible
			for (Entity textbox : registry.textboxes.entities)
			{
				if (registry.textboxes.get(textbox).targetItem == item)
				{
					registry.textboxes.get(textbox).isVisible = false;
					break;
				}
			}

			// Remove visual components of the item
			if (registry.motions.has(item) && !registry.entrances.has(item))
				registry.motions.remove(item);
			if (registry.renderRequests.has(item) && !registry.entrances.has(item))
				registry.renderRequests.remove(item);
			return;
		}
	}
	return;
}

bool WorldSystem::handle_entrance_interaction(Entity entrance_entity)
{
	Entrance& entrance = registry.entrances.get(entrance_entity);
	if (entrance.target_biome == (GLuint)BIOME::GROTTO)
	{
		ScreenState& state = registry.screenStates.components[0];
		state.is_switching_biome = true;
		state.switching_to_biome = (GLuint)BIOME::GROTTO;
	}
	return true;
}

bool WorldSystem::handle_item_pickup(Entity player, Entity item)
{
	Inventory& player_inventory = registry.inventories.get(player);
	Item& item_info = registry.items.get(item);

	// If inventory is full, return false
	if (player_inventory.items.size() >= player_inventory.capacity)
		return false;

	item_info.originalPosition = registry.motions.get(item).position;
	player_inventory.items.push_back(item);

	// Set a random respawn time (5-15 seconds)
	item_info.respawnTime = (rand() % 10000 + 5000);

	// Hide item by removing motion & render components
	if (registry.motions.has(item))
		registry.motions.remove(item);

	if (registry.renderRequests.has(item))
		registry.renderRequests.remove(item);

	// Hide the textbox
	for (Entity textbox : registry.textboxes.entities)
	{
		if (registry.textboxes.get(textbox).targetItem == item)
		{
			registry.textboxes.get(textbox).isVisible = false;
			if (registry.renderRequests.has(textbox)) {
				registry.renderRequests.remove(textbox);
			}
			break;
		}
	}

	return true;
}


void WorldSystem::handle_item_respawn(float elapsed_ms)
{
	for (Entity item : registry.items.entities)
	{
		Item& item_info = registry.items.get(item);

		if (item_info.respawnTime <= 0)
			continue;

		item_info.respawnTime -= elapsed_ms;

		if (item_info.respawnTime > 0)
			return;

		// Respawn item at its original position
		Motion& motion = registry.motions.emplace(item);
		motion.position = item_info.originalPosition;
		motion.angle = 180.f;
		motion.velocity = { 0, 0 };
		motion.scale = (item_info.name == "Magical Fruit") 
			? vec2(FRUIT_WIDTH, FRUIT_HEIGHT) 
			: vec2(COFFEE_BEAN_WIDTH, COFFEE_BEAN_HEIGHT);

		// Restore render request
		registry.renderRequests.insert(
			item,
			{
				(item_info.name == "Magical Fruit") ? TEXTURE_ASSET_ID::FRUIT : TEXTURE_ASSET_ID::COFFEE_BEAN,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_LAYER::ITEM
			}
		);

		// Restore textbox visibility
		for (Entity textbox : registry.textboxes.entities)
		{
			if (registry.textboxes.get(textbox).targetItem != item)
				continue;

			Textbox& textboxComp = registry.textboxes.get(textbox);
			textboxComp.isVisible = true;

			RenderRequest renderRequest = getTextboxRenderRequest(textboxComp);
			registry.renderRequests.insert(textbox, renderRequest);
			break;
		}

		// Reset respawn time
		item_info.respawnTime = 0;
	}
}

void WorldSystem::update_textbox_visibility()
{
	if (registry.players.entities.empty())
		return;

	Entity player = registry.players.entities[0];
	if (!registry.motions.has(player))
		return;

	Motion& player_motion = registry.motions.get(player);

	for (Entity item : registry.items.entities)
	{
		if (!registry.items.has(item) || !registry.motions.has(item))
			continue;

		Motion& item_motion = registry.motions.get(item);

		float distance = glm::distance(player_motion.position, item_motion.position);

		// Find the textbox linked to this item
		Entity textboxEntity;
		bool foundTextbox = false;

		for (Entity textbox : registry.textboxes.entities)
		{
			if (registry.textboxes.get(textbox).targetItem == item)
			{
				textboxEntity = textbox;
				foundTextbox = true;
				break;
			}
		}

		// Update isVisible based on distance
		if (foundTextbox)
		{
			Textbox& textbox = registry.textboxes.get(textboxEntity);
			textbox.isVisible = (distance < TEXTBOX_VISIBILITY_RADIUS);

			RenderRequest renderRequest = getTextboxRenderRequest(textbox);

			if (textbox.isVisible)
			{
				if (!registry.renderRequests.has(textboxEntity))
				{
					registry.renderRequests.insert(textboxEntity, renderRequest);
				}
			}
			else
			{
				if (registry.renderRequests.has(textboxEntity))
				{
					registry.renderRequests.remove(textboxEntity);
				}
			}
		}
	}
}
