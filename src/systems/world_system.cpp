// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "common.hpp"
#include "item_system.hpp"

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

void WorldSystem::init(RenderSystem* renderer_arg, BiomeSystem* biome_sys)
{

	this->renderer = renderer_arg;
	this->biome_sys = biome_sys;

	// start playing background music indefinitely
	// std::cout << "Starting music..." << std::endl;
	// Mix_PlayMusic(background_music, -1);

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	// Updating window title with number of fruits to show serialization
	std::stringstream title_ss;
	int total_items = 0;
	int total_fruits = 0;
	int total_beans = 0;

	// Only count items in the player's inventory
	if (!registry.players.entities.empty()) {
		Entity player = registry.players.entities[0];
		if (registry.inventories.has(player)) {
			Inventory& player_inv = registry.inventories.get(player);

			// Count specific item types and their amounts
			for (Entity item : player_inv.items) {
				if (registry.items.has(item)) {
					Item& item_comp = registry.items.get(item);
					total_items += item_comp.amount;
					if (item_comp.type == ItemType::MAGICAL_FRUIT) total_fruits += item_comp.amount;
					else if (item_comp.type == ItemType::COFFEE_BEANS) total_beans += item_comp.amount;
				}
			}
		}
	}

	title_ss << total_items << " items in player inventory: " << total_fruits << " fruits " << total_beans << " beans";
	glfwSetWindowTitle(window, title_ss.str().c_str());

	if (registry.players.entities.size() < 1)
		return true;
	Entity& player = registry.players.entities[0];
	if (!registry.motions.has(player))
		return true;
	Motion& player_motion = registry.motions.get(player);

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

	updatePlayerWalkAndAnimation(player, player_motion, elapsed_ms_since_last_update);
	updateThrownAmmo(elapsed_ms_since_last_update);
	update_textbox_visibility();
	handle_item_respawn(elapsed_ms_since_last_update);

	if (registry.screenStates.components[0].game_over) restart_game();

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	std::cout << "Restarting..." << std::endl;

	ScreenState& state = registry.screenStates.components[0];
	state.game_over = false;

	// Save the player's inventory before clearing if it exists
	Entity player_entity;
	nlohmann::json player_inventory_data;
	if (!registry.players.entities.empty()) {
		player_entity = registry.players.entities[0];
		if (registry.inventories.has(player_entity)) {
			player_inventory_data = ItemSystem::serializeInventory(player_entity);
		}
	}

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

	if (registry.players.components.size() == 0)
	{
		createPlayer(renderer, vec2(GROTTO_ENTRANCE_X, GROTTO_ENTRANCE_Y + 50));
	}

	// Restore player's inventory if we had one
	if (!player_inventory_data.empty() && !registry.players.entities.empty()) {
		Entity new_player = registry.players.entities[0];
		ItemSystem::deserializeInventory(new_player, player_inventory_data);
	}

	biome_sys->init(renderer);

}

void WorldSystem::handle_collisions()
{
	// Ensure player exists
	if (registry.players.entities.empty())
		return;
	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity))
		return;

	Motion& player_motion = registry.motions.get(player_entity);
	vec2 movement = player_motion.position - player_motion.previous_position;

	bool x_blocked = false;
	bool y_blocked = false;

	for (Entity collision_entity : registry.collisions.entities)
	{
		if (!registry.collisions.has(collision_entity))
			continue;
		Collision& collision = registry.collisions.get(collision_entity);

		// case ammo hits enemy
		if ((registry.ammo.has(collision_entity) || registry.ammo.has(collision.other)) && (registry.enemies.has(collision_entity) || registry.enemies.has(collision.other))) {
			Entity ammo_entity = registry.ammo.has(collision_entity) ? collision_entity : collision.other;
			Entity enemy_entity = registry.enemies.has(collision_entity) ? collision_entity : collision.other;

			Ammo& ammo = registry.ammo.get(ammo_entity);
			Enemy& enemy = registry.enemies.get(enemy_entity);
			enemy.health -= ammo.damage;
			registry.remove_all_components_of(ammo_entity);
			if (enemy.health <= 0) registry.remove_all_components_of(enemy_entity);
			continue;
		}
		// case where enemy hits player - automatically die and restart game
		else if ((registry.players.has(collision_entity) || registry.players.has(collision.other)) && (registry.enemies.has(collision_entity) || registry.enemies.has(collision.other))) {
			ScreenState& state = registry.screenStates.components[0];
			state.game_over = true;
			continue;
		}

		Entity terrain_entity = (collision_entity == player_entity) ? collision.other : collision_entity;
		if (!registry.terrains.has(terrain_entity) || !registry.motions.has(terrain_entity))
			continue;

		Terrain& terrain = registry.terrains.get(terrain_entity);

		// Full blockage (e.g., rivers)
		if (terrain.collision_setting == 1.0f)
		{
			player_motion.position = player_motion.previous_position;
			x_blocked = true;
			y_blocked = true;
			break; // No need to check further
		}

		// Determine blocking direction
		if (std::abs(movement.x) > 0.0f)
			x_blocked = true;
		if (std::abs(movement.y) > 0.0f)
			y_blocked = true;
	}

	// Resolve movement based on blockages
	if (x_blocked)
		player_motion.position.x = player_motion.previous_position.x;
	if (y_blocked)
		player_motion.position.y = player_motion.previous_position.y;

	// Clear collisions after processing
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

	if (action == GLFW_PRESS && key == GLFW_KEY_R)
	{
		restart_game();
	}

	Entity player = registry.players.entities[0]; // Assume only one player entity
	if (!registry.motions.has(player))
	{
		return;
	}

	std::unordered_set<int>& pressed_keys = WorldSystem::pressed_keys;

	// // Handle character movement
	if (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_D || key == GLFW_KEY_A || key == GLFW_KEY_F)
		// even if press happened in different biome should still continue after switching
	{
		if (action == GLFW_PRESS && key != GLFW_KEY_F)
		{
			pressed_keys.insert(key);
		}
		else if (action == GLFW_RELEASE)
		{
			if (std::find(pressed_keys.begin(), pressed_keys.end(), key) != pressed_keys.end()) pressed_keys.erase(key);
		}
	}

	ScreenState& screen = registry.screenStates.components[0];
	if (screen.is_switching_biome) return; // don't handle character movement or interaction if screen is switching
	if (action == GLFW_PRESS && key == GLFW_KEY_F)
	{
		handle_player_interaction();
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

		throwAmmo(vec2(mouse_pos_x, mouse_pos_y));
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
			handle_textbox = biome_sys->handleEntranceInteraction(item);
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

bool WorldSystem::handle_item_pickup(Entity player, Entity item)
{
	if (!registry.inventories.has(player) || !registry.items.has(item))
		return false;

	Item& item_info = registry.items.get(item);

	item_info.originalPosition = registry.motions.get(item).position;

	if (!ItemSystem::addItemToInventory(player, item))
		return false;

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

		if (registry.screenStates.components[0].biome != (GLuint)BIOME::FOREST) return;
		// only respawn if in forest biome

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

void WorldSystem::updatePlayerWalkAndAnimation(Entity& player, Motion& player_motion, float elapsed_ms_since_last_update) {

	Animation& player_animation = registry.animations.get(player);

	// Update velocity based on active keys
	player_motion.velocity = { 0, 0 }; // Reset velocity before recalculating

	if (registry.screenStates.components[0].is_switching_biome) return; // don't move while switching biomes

	if (pressed_keys.count(GLFW_KEY_W)) {
		player_motion.velocity[1] -= PLAYER_SPEED;
		player_animation.frames = { TEXTURE_ASSET_ID::PLAYER_WALKING_W_1, TEXTURE_ASSET_ID::PLAYER_WALKING_W_2,
									TEXTURE_ASSET_ID::PLAYER_WALKING_W_3, TEXTURE_ASSET_ID::PLAYER_WALKING_W_4 };
	}
	if (pressed_keys.count(GLFW_KEY_S))
	{
		player_motion.velocity[1] += PLAYER_SPEED;
		player_animation.frames = { TEXTURE_ASSET_ID::PLAYER_WALKING_S_1, TEXTURE_ASSET_ID::PLAYER_WALKING_S_2,
									TEXTURE_ASSET_ID::PLAYER_WALKING_S_3, TEXTURE_ASSET_ID::PLAYER_WALKING_S_4 };
	}
	if (pressed_keys.count(GLFW_KEY_D)) {
		player_motion.velocity[0] += PLAYER_SPEED;
		player_animation.frames = { TEXTURE_ASSET_ID::PLAYER_WALKING_D_1, TEXTURE_ASSET_ID::PLAYER_WALKING_D_2,
									TEXTURE_ASSET_ID::PLAYER_WALKING_D_3, TEXTURE_ASSET_ID::PLAYER_WALKING_D_4 };
	}
	if (pressed_keys.count(GLFW_KEY_A)) {
		player_motion.velocity[0] -= PLAYER_SPEED;
		player_animation.frames = { TEXTURE_ASSET_ID::PLAYER_WALKING_A_1, TEXTURE_ASSET_ID::PLAYER_WALKING_A_2,
									TEXTURE_ASSET_ID::PLAYER_WALKING_A_3, TEXTURE_ASSET_ID::PLAYER_WALKING_A_4 };
	}

	// Normalize velocity to avoid increased speed when moving diagonally
	if (player_motion.velocity[0] != 0.0f || player_motion.velocity[1] != 0.0f) {
		float magnitude = std::sqrt(player_motion.velocity[0] * player_motion.velocity[0] +
			player_motion.velocity[1] * player_motion.velocity[1]);
		player_motion.velocity[0] = (player_motion.velocity[0] / magnitude) * PLAYER_SPEED;
		player_motion.velocity[1] = (player_motion.velocity[1] / magnitude) * PLAYER_SPEED;
	}

	if (player_motion.velocity[0] == 0.0f && player_motion.velocity[1] == 0.0f) {
		// keep the first frame of the last direction player was moving when they're idle
		player_animation.current_frame = 1;
	}

	player_animation.elapsed_time += elapsed_ms_since_last_update;

	RenderRequest& render_request = registry.renderRequests.get(player);
	// change our frames depending on time passed
	if (player_animation.elapsed_time >= player_animation.frame_time) {
		player_animation.elapsed_time = 0;
		player_animation.current_frame = (player_animation.current_frame + 1) % player_animation.frames.size();
		render_request.used_texture = player_animation.frames[player_animation.current_frame];
	}

	// move the character
	player_motion.previous_position = player_motion.position;
	float delta = elapsed_ms_since_last_update * TIME_UPDATE_FACTOR;
	player_motion.position += delta * player_motion.velocity;

	// update player's throwing cooldown
	Player& player_comp = registry.players.get(player);
	if (player_comp.cooldown > 0) {
		player_comp.cooldown -= elapsed_ms_since_last_update;
		if (player_comp.cooldown <= 0) {
			player_comp.cooldown = 0;
		}
	}
}

void WorldSystem::throwAmmo(vec2 target) {
	if (registry.players.entities.empty()) return;
	Entity player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);

	if (player.cooldown > 0.f) std::cout << "on cooldown" << std::endl;
	if (!registry.inventories.has(player_entity) || player.cooldown > 0.f || !registry.motions.has(player_entity)) return;
	Inventory& inventory = registry.inventories.get(player_entity);
	if (inventory.items.size() < inventory.selection + 1) {
		std::cout << "cannot throw selected item" << std::endl;
		return;
	}

	Entity& item_entity = inventory.items[inventory.selection];

	if (createFiredAmmo(renderer, target, item_entity, player_entity)) {
		if (registry.items.has(item_entity)) {
			Item& item = registry.items.get(item_entity);
			item.amount -= 1;
			if (item.amount == 0) {
				ItemSystem::destroyItem(item_entity);
				ItemSystem::removeItemFromInventory(player_entity, item_entity);
			}
		}
		player.cooldown = 1000.f;
	}

}

void WorldSystem::updateThrownAmmo(float elapsed_ms_since_last_update) {
	for (auto& entity : registry.ammo.entities) {
		if (!registry.motions.has(entity)) continue;

		Ammo& ammo = registry.ammo.get(entity);
		if (!ammo.is_fired) continue;

		Motion& ammo_motion = registry.motions.get(entity);
		ammo_motion.position += ammo_motion.velocity * elapsed_ms_since_last_update * THROW_UPDATE_FACTOR;
		if (abs(ammo_motion.position.x - ammo.start_pos.x) > abs(ammo.target.x - ammo.start_pos.x)
			|| abs(ammo_motion.position.y - ammo.start_pos.y) > abs(ammo.target.y - ammo.start_pos.y))
			registry.remove_all_components_of(entity);

	}
}