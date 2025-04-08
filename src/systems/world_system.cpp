// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "common.hpp"
#include "item_system.hpp"
#include "sound_system.hpp"

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
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
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
	auto scroll_wheel_redirect = [](GLFWwindow* wnd, double _xoffset, double _yoffset)
		{ ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_wheel(_xoffset, _yoffset); };
	auto window_resize_redirect = [](GLFWwindow* wnd, int _width, int _height)
		{ ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_window_resize(_width, _height); };

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);
	glfwSetScrollCallback(window, scroll_wheel_redirect);
	glfwSetWindowSizeCallback(window, window_resize_redirect);

	return window;
}

bool WorldSystem::init(RenderSystem* renderer_arg, BiomeSystem* biome_sys)
{

	this->renderer = renderer_arg;
	this->biome_sys = biome_sys;

	RespawnSystem::getInstance().renderer = renderer_arg;

	// Set all states to default
	restart_game(false);

	return true;
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	// Updating window title with number of fruits to show serialization
	std::stringstream title_ss;

	updateFPS(elapsed_ms_since_last_update);
	// visually update counter every 500 ms
	if (m_fps_update_timer >= 500.0f) {
		m_fps_update_timer = 0.0f;
		m_last_fps = m_current_fps;
		title_ss << "FPS: " << m_current_fps;
	}
	else {
		title_ss << "FPS: " << m_last_fps;
	}

	glfwSetWindowTitle(window, title_ss.str().c_str());

	// autosave every minute
	ScreenState& screen = registry.screenStates.components[0];
	screen.autosave_timer -= elapsed_ms_since_last_update;
	if (screen.autosave_timer <= 0) {
		screen.autosave_timer = AUTOSAVE_TIMER;
		ItemSystem::saveGameState();
	}

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

	// Update respawn system timers for persistent respawns
	RespawnSystem::getInstance().step(elapsed_ms_since_last_update);

	// Remove enemies from killed_enemies list if they're respawned in RespawnSystem
	for (auto& [id, state] : RespawnSystem::getInstance().getRespawnStates()) {
		if (state.isSpawned && !state.enemyName.empty()) {
			// Find and remove from killed_enemies if present
			auto& killed_list = registry.screenStates.components[0].killed_enemies;
			auto it = std::find(killed_list.begin(), killed_list.end(), state.enemyName);
			if (it != killed_list.end()) {
				std::cout << "Removing " << state.enemyName << " from killed_enemies list to allow respawn" << std::endl;
				killed_list.erase(it);
			}
		}
	}

	updateThrownAmmo(elapsed_ms_since_last_update);

	if (m_ui_system->isCauldronOpen() || m_ui_system->isMortarPestleOpen()) return true; // skip the following updates if menu is open
	updatePlayerState(player, player_motion, elapsed_ms_since_last_update);
	update_textbox_visibility();

	for (Entity e : registry.delayedMovements.entities) {
		DelayedMovement& delay = registry.delayedMovements.get(e);
		delay.delay_ms -= elapsed_ms_since_last_update;
		if (delay.delay_ms <= 0.f) {
			if (registry.motions.has(e)) {
				registry.motions.get(e).velocity = delay.velocity;
			}
			registry.delayedMovements.remove(e);
		}
	}

	// apply damage over time on enemies
	for (auto entity : registry.enemies.entities) {
		if (!registry.enemies.has(entity)) continue;
		Enemy& enemy = registry.enemies.get(entity);
		if (enemy.dot_effect == PotionEffect::WATER) continue;

		enemy.dot_timer -= elapsed_ms_since_last_update;
		enemy.dot_duration -= elapsed_ms_since_last_update;
		if (enemy.dot_timer <= 0.f) {
			enemy.dot_timer = enemy.dot_effect == PotionEffect::MOLOTOV ? DOT_MOLOTOV_TIMER : DOT_POISON_TIMER;
			handleEnemyInjured(entity, enemy.dot_damage);
		}
		if (enemy.dot_duration <= 0.f) {
			enemy.dot_duration = 0.f;
			enemy.dot_damage = 0.f;
			enemy.dot_effect = PotionEffect::WATER;
		}
	}

	// if we have saved grotto, should pulse rejuvenation glow effect
	for (Entity entity : registry.texturedEffects.entities) {
		TexturedEffect& effect = registry.texturedEffects.get(entity);
		Motion& motion = registry.motions.get(entity);

		effect.animation_timer += elapsed_ms_since_last_update / 1000.f;

		// initially grow over 3 seconds
		if (!effect.done_growing) {
			float grow_duration = 3.f; // seconds
			float t = effect.animation_timer / grow_duration;
			t = std::min(t, 1.f); // clamp between 0 and 1

			// linearly interpolate from small (20) to large (100)
			float scale = 20.f + t * (100.f - 20.f);
			motion.scale = vec2(scale, scale);

			if (t >= 1.f) {
				effect.done_growing = true;
				effect.animation_timer = 0.f;
			}
		}

		// pulse around larger size
		else {
			float base_scale = 100.f;
			float amplitude = 10.f;
			float frequency = 1.f / 4.f; // one cycle every 4 seconds

			float scale = base_scale + amplitude * std::sin(effect.animation_timer * 2.f * M_PI * frequency);
			motion.scale = vec2(scale, scale);
		}
	}

	// this happens when we save the grotto and complete the game
	if (screen.play_ending) {
		// Fade the fog intensity from 1.5 to 0 over 5 seconds to show corruption fading
		const float fade_duration = 5.f;
		screen.fog_intensity -= elapsed_ms_since_last_update / 1000.f * (1.5f / fade_duration);
		screen.fog_intensity = std::max(0.f, screen.fog_intensity);

		// Only trigger the congrats text when fog intensity has reached 0
		if (screen.fog_intensity == 0.0f && !screen.ending_text_shown) {
			m_ui_system->createScreenText("Congratulations, you've saved the grotto!", 3.0f);
			screen.ending_text_shown = true;  // Mark the text as shown to prevent repeating
		}

		// make enemies flash and die
		for (Entity entity : registry.enemies.entities) {
			Motion& motion = registry.motions.get(entity);
			// make enemies stop moving if they're chasing you so it doesnt look weird
			motion.velocity = vec2(0, 0);
			motion.position = motion.position;
			registry.enemies.get(entity).attack_damage = 0.f;

			// enemy will flash damage once before being deleted
			if (!registry.damageFlashes.has(entity)) {
				DamageFlash& flash = registry.damageFlashes.emplace(entity);
				flash.flash_value = 1.0f;
				flash.kill_after_flash = true;
			}
			m_ui_system->updateEnemyHealth(entity, 0.f);
		}
	}

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game(bool hard_reset)
{
	std::cout << "Restarting..." << std::endl;

	ScreenState& screen = registry.screenStates.components[0];

	// Save the player's inventory before clearing if it exists
	// Entity player_entity;
	// nlohmann::json player_inventory_data;
	// if (!registry.players.entities.empty()) {
	// 	player_entity = registry.players.entities[0];
	// 	if (registry.inventories.has(player_entity)) {
	// 		player_inventory_data = ItemSystem::serializeInventory(player_entity);
	// 	}
	// }

	// Entity cauldron;
	// nlohmann::json cauldron_inventory_data;
	// if (!registry.cauldrons.entities.empty()) { // in the future may have multiple cauldrons
	// 	cauldron = registry.cauldrons.entities[0];
	// 	if (registry.inventories.has(cauldron)) {
	// 		cauldron_inventory_data = ItemSystem::serializeInventory(cauldron);
	// 		std::cout << "serialized cauldron data" << std::endl;
	// 	}
	// }

	// close cauldron if it's open
	if (m_ui_system && m_ui_system->isCauldronOpen()) m_ui_system->closeCauldron(true);

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
		createPlayer(renderer, vec2(GRID_CELL_WIDTH_PX * 17.5, GRID_CELL_HEIGHT_PX * 12.0));
	}

	// re-open tutorial, state is loaded from persistence
	// state.tutorial_step_complete = true;

	if (hard_reset) {
		screen.from_biome = (int)BIOME::FOREST;
		screen.biome = (int)BIOME::GROTTO;
		screen.is_switching_biome = true;
		screen.switching_to_biome = (int)BIOME::GROTTO;
		screen.tutorial_state = 0;
		screen.tutorial_step_complete = true;
		screen.fog_intensity = FOG_INTENSITY;
		createWelcomeScreen(renderer, vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 - 50));
		screen.killed_enemies = {};
		screen.unlocked_biomes = {};
		if (m_ui_system) {
			m_ui_system->updateEffectsBar();
			m_ui_system->updateHealthBar();
			m_ui_system->updateInventoryBar();
		}

		biome_sys->init(renderer);

		screen.is_switching_biome = true;
		screen.fade_status = 1;
		screen.darken_screen_factor = 1.0f;
		biome_sys->switchBiome((int)BIOME::GROTTO, true);
	}
	else {
		nlohmann::json loaded_data = ItemSystem::loadCoreState();

		// Initialize the biome system after core data is loaded
		biome_sys->init(renderer);

		// For deferred inventory loading
		if (!loaded_data.is_null()) {
			biome_sys->setLoadedGameData(loaded_data);
		}
	}
}

void WorldSystem::handle_collisions(float elapsed_ms)
{
	// Ensure player exists
	if (registry.players.entities.empty())
		return;
	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity))
		return;

	Motion& player_motion = registry.motions.get(player_entity);
	vec2 original_position = player_motion.position;
	vec2 previous_position = player_motion.previous_position;

	// Handle non-terrain collisions first (ammo, enemies, etc.)
	for (Entity collision_entity : registry.collisions.entities)
	{
		ScreenState& screen = registry.screenStates.components[0];

		if (!registry.collisions.has(collision_entity))
			continue;
		Collision& collision = registry.collisions.get(collision_entity);

		// case ammo hits enemy
		if ((registry.ammo.has(collision_entity) || registry.ammo.has(collision.other)) && (registry.enemies.has(collision_entity) || registry.enemies.has(collision.other))) {
			Entity ammo_entity = registry.ammo.has(collision_entity) ? collision_entity : collision.other;
			Entity enemy_entity = registry.enemies.has(collision_entity) ? collision_entity : collision.other;

			Enemy& enemy = registry.enemies.get(enemy_entity);
			Ammo& ammo = registry.ammo.get(ammo_entity);
			if (!registry.potions.has(ammo_entity)) continue;
			Potion& potion = registry.potions.get(ammo_entity);

			if (registry.potions.get(ammo_entity).effect == PotionEffect::MOLOTOV && registry.motions.has(enemy_entity)) {
				// damage all enemies within 100 px
				vec2 enemy_pos = registry.motions.get(enemy_entity).position;
				for (auto neighbour_enemy : registry.enemies.entities) {
					if (neighbour_enemy == enemy_entity || !registry.motions.has(neighbour_enemy)) continue;
					vec2 pos = registry.motions.get(neighbour_enemy).position;
					float dx = pos.x - enemy_pos.x;
					float dy = pos.y - enemy_pos.y;
					if ((dx * dx + dy * dy) <= MOLOTOV_RADIUS_SQUARED) {
						// apply damage over time effect
						Enemy& neighbour = registry.enemies.get(neighbour_enemy);
						neighbour.dot_damage = potion.effectValue * MOLOTOV_MULTIPLIER;
						neighbour.dot_timer = DOT_MOLOTOV_TIMER;
						neighbour.dot_duration = potion.duration;
						neighbour.dot_effect = PotionEffect::MOLOTOV;
						handleEnemyInjured(neighbour_enemy, ammo.damage);
					}
				}
			}

			if (potion.effect == PotionEffect::POISON) {
				enemy.dot_damage = potion.effectValue;
				enemy.dot_timer = DOT_POISON_TIMER;
				enemy.dot_duration = potion.duration;
				enemy.dot_effect = PotionEffect::POISON;
			}
			else if (potion.effect == PotionEffect::MOLOTOV) {
				enemy.dot_damage = potion.effectValue * MOLOTOV_MULTIPLIER;
				enemy.dot_timer = DOT_MOLOTOV_TIMER;
				enemy.dot_duration = potion.duration;
				enemy.dot_effect = PotionEffect::MOLOTOV;
			}

			handleEnemyInjured(enemy_entity, ammo.damage);
			registry.remove_all_components_of(ammo_entity);
		}
		// case where enemy hits player
		else if ((registry.players.has(collision_entity) || registry.players.has(collision.other)) && (registry.enemies.has(collision_entity) || registry.enemies.has(collision.other))) {
			Entity player_entity = registry.players.has(collision_entity) ? collision_entity : collision.other;
			Entity enemy_entity = registry.enemies.has(collision_entity) ? collision_entity : collision.other;

			Player& player = registry.players.get(player_entity);

			if (player.damage_cooldown > 0)	continue;
			if (registry.enemies.get(enemy_entity).attack_damage == 0.f) continue;

			// player flashes red and takes damage equal to enemy's attack
			if (!registry.damageFlashes.has(player_entity)) registry.damageFlashes.emplace(player_entity);
			player.health -= (registry.enemies.get(enemy_entity).attack_damage * player.defense);
			SoundSystem::playPlayerOuchSound(-1, 0);
			player.damage_cooldown = PLAYER_DAMAGE_COOLDOWN;

			// if player dies, reload from most recent save and respawn in grotto
			if (player.health <= 0) {
				std::cout << "player died!" << std::endl;
				GLuint last_biome = screen.biome; // this is the biome that the player died in

				// remove any ammo from screen
				for (auto thrown_ammo : registry.ammo.entities) {
					if (registry.ammo.get(thrown_ammo).is_fired) registry.remove_all_components_of(thrown_ammo);
				}

				// Apply death penalty: remove a random valid item
				if (registry.inventories.has(player_entity)) {
					Inventory& inventory = registry.inventories.get(player_entity);

					std::vector<Entity> valid_items;
					for (Entity item : inventory.items) {
						if (item != Entity() && registry.items.has(item)) {
							valid_items.push_back(item);
						}
					}

					if (!valid_items.empty()) {
						int rand_index = rand() % valid_items.size();
						Entity item_to_remove = valid_items[rand_index];

						Item& item = registry.items.get(item_to_remove);
						std::cout << "Death penalty: lost item " << item.name << std::endl;

						// Remove 1 amount
						item.amount -= 1;
						if (item.amount <= 0) {
							ItemSystem::removeItemFromInventory(player_entity, item_to_remove);
							ItemSystem::destroyItem(item_to_remove);
						}

						if (m_ui_system) {
							m_ui_system->updateInventoryBar();
						}
					}
				}

				ItemSystem::loadGameState();
				screen.is_switching_biome = true;
				screen.switching_to_biome = (GLuint)BIOME::GROTTO;
				screen.from_biome = (GLuint)BIOME::GROTTO;
				// since we don't save the game state upon dying, load game state does not necessarily have the updated data as to where the player died
				// we set screen.from_biome and screen.biome to the same from_biome on reload so we need to overide the biome with the latest one
				screen.biome = last_biome;
				screen.fade_status = 0;
				registry.collisions.clear();
				registry.damageFlashes.clear();
				player.health = PLAYER_MAX_HEALTH; // reset to max health
				for (auto effect : player.active_effects) {
					if (registry.potions.has(effect)) {
						removePotionEffect(registry.potions.get(effect), player_entity);
					}
				}
				player.active_effects.clear(); // reset active effects
			}
			if (m_ui_system) {
				m_ui_system->updateHealthBar();
				m_ui_system->updateEffectsBar();
				m_ui_system->updateInventoryBar();
			}
			continue;
		}
		else if ((registry.ammo.has(collision_entity) || registry.ammo.has(collision.other)) && (registry.terrains.has(collision_entity) || registry.terrains.has(collision.other))) {
			Entity ammo_entity = registry.ammo.has(collision_entity) ? collision_entity : collision.other;
			Entity terrain_entity = registry.terrains.has(collision_entity) ? collision_entity : collision.other;

			if (registry.potions.has(ammo_entity) && registry.potions.get(ammo_entity).effect == PotionEffect::MOLOTOV && registry.motions.has(terrain_entity)) {
				// damage all enemies within 100 px
				vec2 terrain_pos = registry.motions.get(terrain_entity).position;
				for (auto neighbour_enemy : registry.enemies.entities) {
					if (!registry.motions.has(neighbour_enemy)) continue;
					vec2 pos = registry.motions.get(neighbour_enemy).position;
					float dx = pos.x - terrain_pos.x;
					float dy = pos.y - terrain_pos.y;
					if ((dx * dx + dy * dy) <= MOLOTOV_RADIUS_SQUARED) handleEnemyInjured(neighbour_enemy, registry.ammo.get(ammo_entity).damage);
				}
			}

			registry.remove_all_components_of(ammo_entity);
			continue;
		}
	}

	bool moving_diagonally = (original_position.x != previous_position.x) && (original_position.y != previous_position.y);

	// Helper function to check collisions with all terrain entities for an updated position
	// this takes in a test_position argument that will act as player's "new" position, 
	// returns a boolean if there is a collision or not
	auto checkCollisions = [](const vec2& test_position) {
		for (Entity terrain_entity : registry.terrains.entities) {
			if (!registry.motions.has(terrain_entity))
				continue;

			Motion& terrain_motion = registry.motions.get(terrain_entity);
			Terrain& terrain = registry.terrains.get(terrain_entity);

			vec2 orig_pos = registry.motions.get(registry.players.entities[0]).position;

			// set our player's position to the test position
			registry.motions.get(registry.players.entities[0]).position = test_position;

			bool has_collision = PhysicsSystem::collides(registry.motions.get(registry.players.entities[0]),
				terrain_motion, &terrain, terrain_entity);

			// restore original position
			registry.motions.get(registry.players.entities[0]).position = orig_pos;

			if (has_collision)
				return true;
		}

		return false;
		};

	if (moving_diagonally) {
		// position as if we moved just horizontally
		vec2 horizontal_pos = previous_position;
		horizontal_pos.x = original_position.x;

		// position as if we moved just vertically
		vec2 vertical_pos = previous_position;
		vertical_pos.y = original_position.y;

		bool horizontal_collision = checkCollisions(horizontal_pos);
		bool vertical_collision = checkCollisions(vertical_pos);
		bool diagonal_collision = checkCollisions(original_position);

		if (diagonal_collision) {
			if (!horizontal_collision && !vertical_collision) {
				player_motion.position = previous_position;
			}
			else if (!horizontal_collision) {
				player_motion.position = horizontal_pos;
			}
			else if (!vertical_collision) {
				player_motion.position = vertical_pos;
			}
			else {
				player_motion.position = previous_position;
			}
		}
		else {
			// there's no collision with diagonal movement, allow player to move
			player_motion.position = original_position;
		}
	}
	else {
		// For non-diagonal movement, just check if there's a collision
		if (checkCollisions(original_position)) {
			player_motion.position = previous_position;
		}
	}

	// Clear collisions after processing
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

// on key callback
void WorldSystem::on_key(int key, int scancode, int action, int mod)
{
	// First pass the event to the UI system if it's initialized
	if (m_ui_system != nullptr) {
		m_ui_system->handleKeyEvent(key, scancode, action, mod);
	}

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
	{
		// Check if any UI menus are open and close them first
		if (m_ui_system != nullptr) {
			if (m_ui_system->isCauldronOpen()) {
				m_ui_system->closeCauldron(true);
				return;
			}
			else if (m_ui_system->isMortarPestleOpen()) {
				m_ui_system->closeMortarPestle(true);
				return;
			}
			else if (m_ui_system->isRecipeBookOpen()) {
				m_ui_system->closeRecipeBook();
				return;
			}
			else if (m_ui_system->isChestMenuOpen()) {
				m_ui_system->closeChestMenu();
				return;
			}
		}

		close_window();
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_L)
	{
		if (registry.screenStates.components[0].play_ending || registry.screenStates.components[0].is_switching_biome) return;
		restart_game(true);
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		if (m_ui_system->isRecipeBookOpen()) {
			m_ui_system->closeRecipeBook();
		}
		else {
			m_ui_system->openRecipeBook(m_ui_system->getOpenedRecipeBook());
		}
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_P)
	{
		ItemSystem::saveGameState();
	}

	Entity player = registry.players.entities[0]; // Assume only one player entity
	if (!registry.motions.has(player))
	{
		return;
	}

	std::unordered_set<int>& pressed_keys = WorldSystem::pressed_keys;

	ScreenState& screen = registry.screenStates.components[0];

	// toggle tutorial
	if (action == GLFW_PRESS && key == GLFW_KEY_T) {
		screen.tutorial_step_complete = true;
		screen.tutorial_state = (screen.tutorial_state == (int)TUTORIAL::COMPLETE) ? (int)TUTORIAL::TOGGLE_TUTORIAL : (int)TUTORIAL::COMPLETE;
	}

	// skip tutorial step
	if (action == GLFW_PRESS && key == GLFW_KEY_N) {
		screen.tutorial_step_complete = true;
		if (screen.tutorial_state != (int)TUTORIAL::COMPLETE) screen.tutorial_state += 1;
	}


	// Handle character movement
	if (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_D || key == GLFW_KEY_A)
		// even if press happened in different biome should still continue after switching
	{
		if (action == GLFW_PRESS)
		{
			pressed_keys.insert(key);
		}
		else if (action == GLFW_RELEASE)
		{
			if (std::find(pressed_keys.begin(), pressed_keys.end(), key) != pressed_keys.end())
				pressed_keys.erase(key);
		}
	}

	if (screen.is_switching_biome || screen.tutorial_state == (int) TUTORIAL::WELCOME_SCREEN) return; // don't handle character movement or interaction if screen is switching
	if (action == GLFW_PRESS && key == GLFW_KEY_F)
	{
		handle_player_interaction();
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_F11) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if (glfwGetWindowMonitor(window)) {
			// We are in fullscreeen, undo it
			glfwSetWindowMonitor(window, nullptr, winPosX, winPosY, WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, GLFW_DONT_CARE);
		}
		else {
			glfwGetWindowPos(window, &winPosX, &winPosY);
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{
	// Actual mouse coords
	double x = mouse_position.x;
	double y = mouse_position.y;

	// Cauldron uses unscaled mouse coords lmao
	renderer->updateCauldronMouseLoc(x, y);

	// Subtract possible black bar heights
	GLint viewport_coords[4];
	glGetIntegerv(GL_VIEWPORT, viewport_coords);
	float scale = renderer->getRetinaScale();
	x -= viewport_coords[0] / scale;
	y -= viewport_coords[1] / scale;

	// Scale down to size
	x *= (float)WINDOW_WIDTH_PX / (viewport_coords[2] / scale);
	y *= (float)WINDOW_HEIGHT_PX / (viewport_coords[3] / scale);

	if (x < 0 || x > WINDOW_WIDTH_PX || y < 0 || y > WINDOW_HEIGHT_PX) {
		return;
	}

	// Pass the event to the UI system if it's initialized
	if (m_ui_system != nullptr) {
		m_ui_system->handleMouseMoveEvent(x, y);
	}

	// record the current mouse position
	mouse_pos_x = x;
	mouse_pos_y = y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods)
{
	std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;

	// Pass the event to the UI system if it's initialized 
	if (m_ui_system != nullptr) {
		bool isOpen = m_ui_system->isClickOnUIElement();
		m_ui_system->handleMouseButtonEvent(button, action, mods);
		if (isOpen) {
			return;
		}
	}

	// on button press
	if (action != GLFW_PRESS) {
		return;
	}

	// std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
	// std::cout << "mouse tile position: " << tile_x << ", " << tile_y << std::endl;

	ScreenState& screen = registry.screenStates.components[0];
	if (!screen.is_switching_biome && button == GLFW_MOUSE_BUTTON_LEFT && throwAmmo(vec2(mouse_pos_x, mouse_pos_y))) {
		SoundSystem::playThrowSound((int)SOUND_CHANNEL::GENERAL, 0);
		if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::THROW_POTION) {
			screen.tutorial_step_complete = true;
			screen.tutorial_state += 1;
		}
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		consumePotion();
	}
}

void WorldSystem::on_mouse_wheel(double xoffset, double yoffset)
{
	// Pass the event to the UI system if it's initialized 
	if (m_ui_system != nullptr) {
		m_ui_system->handleScrollWheelEvent(xoffset, yoffset);
	}
}

void WorldSystem::on_window_resize(int w, int h)
{
	int fbw, fbh;
	glfwGetFramebufferSize(window, &fbw, &fbh);

	// This could happen when alt-tabbing from fullscreen
	if (fbw == 0 || fbh == 0) {
		return;
	}

	float scale = 1.0f;
	int xsize = WINDOW_WIDTH_PX, ysize = WINDOW_HEIGHT_PX;
	if ((float)w / h > WINDOW_RATIO) {
		scale = (float)fbh / ysize;
	}
	else {
		scale = (float)fbw / xsize;
	}

	xsize *= scale;
	ysize *= scale;
	int x = (fbw - xsize) / 2;
	int y = (fbh - ysize) / 2;
	renderer->setViewportCoords(x, y, xsize, ysize);
	renderer->initializeWaterBuffers(false); // Need to redo water sim cause of texture size change
	m_ui_system->updateWindowSize(scale);
}

void WorldSystem::handle_player_interaction()
{
	// Check if player exists
	if (registry.players.entities.empty())
		return;

	Entity player = registry.players.entities[0]; // Assume single-player
	if (!registry.motions.has(player) || !registry.inventories.has(player))
		return;

	// If a cauldron is open just close it
	if (m_ui_system && m_ui_system->isCauldronOpen()) {
		m_ui_system->closeCauldron(true);
		return;
	}

	// If a mortar is open, close it
	if (m_ui_system && m_ui_system->isMortarPestleOpen()) {
		m_ui_system->closeMortarPestle(true);
		return;
	}

	// If a recipe book is open, close it
	if (m_ui_system && m_ui_system->isRecipeBookOpen()) {
		m_ui_system->closeRecipeBook();
		return;
	}

	if (m_ui_system && m_ui_system->isChestMenuOpen()) {
		m_ui_system->closeChestMenu();
		return;
	}

	Motion& player_motion = registry.motions.get(player);
	for (Entity item : registry.items.entities)
	{
		if (!registry.motions.has(item)) {
			continue;
		}

		Motion& item_motion = registry.motions.get(item);
		Item& item_info = registry.items.get(item);

		// Check if item is collectable or is an interactable entrance
		if (!item_info.isCollectable && !registry.entrances.has(item) && !registry.cauldrons.has(item)
			&& !registry.mortarAndPestles.has(item) && !registry.guardians.has(item)) {
			if (item_info.type != ItemType::RECIPE_BOOK && item_info.type != ItemType::CHEST) {
				continue;
			}
		}

		// If item is not in pickup range, continue
		float distance = glm::distance(player_motion.position, item_motion.position);
		// std::cout << "distance: " << distance << "item: " << item_info.name << std::endl;
		if (distance > ITEM_PICKUP_RADIUS) {
			continue;
		}

		// If this is a cauldron/mortar & pestle and it's invisible, ignore it so if items overlap, we don't get stuck opening it
		if (registry.cauldrons.has(item) || registry.mortarAndPestles.has(item) || item_info.type == ItemType::RECIPE_BOOK || registry.chests.has(item)) {
			if (registry.renderRequests.has(item) && !registry.renderRequests.get(item).is_visible) {
				continue;
			}
		}

		// Handle interaction
		bool handle_textbox = false;
		if (registry.entrances.has(item))
		{
			handle_textbox = biome_sys->handleEntranceInteraction(item);
		}
		else if (registry.guardians.has(item))
		{
			handle_textbox = handleGuardianUnlocking(item);
		}
		else if (item_info.isCollectable)
		{
			handle_textbox = handle_item_pickup(player, item);
		}
		else if (registry.cauldrons.has(item)) {
			// don't allow opening if it's currently invisible
			if (registry.renderRequests.has(item) && !registry.renderRequests.get(item).is_visible) return;
			if (m_ui_system != nullptr)
			{
				handle_textbox = m_ui_system->openCauldron(item, true);
				if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::INTERACT_CAULDRON) {
					ScreenState& screen = registry.screenStates.components[0];
					screen.tutorial_step_complete = true;
					screen.tutorial_state += 1;
				}
			}
		}
		else if (registry.mortarAndPestles.has(item)) {
			if (registry.renderRequests.has(item) && !registry.renderRequests.get(item).is_visible) return;

			if (m_ui_system != nullptr) {
				handle_textbox = m_ui_system->openMortarPestle(item, true);
				if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::MORTAR_PESTLE) {
					ScreenState& screen = registry.screenStates.components[0];
					screen.tutorial_step_complete = true;
					screen.tutorial_state += 1;
				}

			}
		}
		else if (item_info.type == ItemType::RECIPE_BOOK) {
			if (registry.renderRequests.has(item) && !registry.renderRequests.get(item).is_visible) return;
			if (m_ui_system != nullptr) {
				handle_textbox = m_ui_system->openRecipeBook(item);
				if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::RECIPE_BOOK) {
					ScreenState& screen = registry.screenStates.components[0];
					screen.tutorial_step_complete = true;
					screen.tutorial_state += 1;
				}
			}
		}
		else if (item_info.type == ItemType::CHEST) {
			if (registry.renderRequests.has(item) && !registry.renderRequests.get(item).is_visible) return;
			if (m_ui_system != nullptr) {
				handle_textbox = m_ui_system->openChestMenu(item);
			}
		}

		if (handle_textbox)
		{
			// Remove visual components of the item
			if (!item_info.isCollectable)
				return;

			if (registry.motions.has(item))
				registry.motions.remove(item);
			if (registry.renderRequests.has(item))
				registry.renderRequests.remove(item);
			return;
		}
	}
}

bool WorldSystem::handle_item_pickup(Entity player, Entity item)
{
	if (!registry.inventories.has(player) || !registry.items.has(item))
		return false;

	Item& item_info = registry.items.get(item);
	if (!ItemSystem::addItemToInventory(player, item))
		return false;
	SoundSystem::playCollectItemSound((int)SOUND_CHANNEL::GENERAL, 0);

	// handle item collection for the tutorial
	if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::COLLECT_ITEMS) {
		Inventory& inventory = registry.inventories.get(player);
		bool collected_bark = false;
		bool collected_leaves = false;
		for (Entity& entity : inventory.items) {
			if (!registry.items.has(entity)) continue;
			Item& item = registry.items.get(entity);
			if (item.type == ItemType::STORM_BARK) {
				if (item.amount >= 2) {
					collected_bark = true;
				}
			}
			if (item.type == ItemType::BLIGHTLEAF) {
				if (item.amount >= 1) {
					collected_leaves = true;
				}
			}
		}
		if (collected_bark && collected_leaves) {
			ScreenState& screen = registry.screenStates.components[0];
			screen.tutorial_step_complete = true;
			screen.tutorial_state += 1;
		}
	}

	if (item_info.canRespawn && item_info.isCollectable) {
		RespawnSystem::getInstance().registerEntity(item, false);

		// Set a random respawn time (60-90 seconds)
		float respawnTime = (rand() % 30000 + 60000);

		if (!item_info.persistentID.empty()) {
			RespawnSystem::getInstance().setRespawning(item_info.persistentID, respawnTime);
		}
	}

	update_textbox_visibility();

	return true;
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
		for (Entity textboxEntity : registry.textboxes.entities)
		{
			if (registry.textboxes.get(textboxEntity).targetItem == item)
			{
				Textbox& textbox = registry.textboxes.get(textboxEntity);
				bool shouldBeVisible = (distance < TEXTBOX_VISIBILITY_RADIUS);

				bool uiIsOpenForItem = false;
				Item& item_info = registry.items.get(item);
				if (registry.cauldrons.has(item) && m_ui_system->isCauldronOpen()) {
					uiIsOpenForItem = true;
				}
				else if (registry.mortarAndPestles.has(item) && m_ui_system->isMortarPestleOpen()) {
					uiIsOpenForItem = true;
				}
				else if (item_info.type == ItemType::RECIPE_BOOK && m_ui_system->isRecipeBookOpen()) {
					uiIsOpenForItem = true;
				}
				else if (item_info.type == ItemType::CHEST && m_ui_system->isChestMenuOpen()) {
					uiIsOpenForItem = true;
				}

				textbox.isVisible = shouldBeVisible && !uiIsOpenForItem;

				if (textbox.isVisible)
				{
					m_ui_system->textboxes[textboxEntity.id()] = textbox;
				}
				break;
			}
		}
	}
}

void WorldSystem::showTemporaryGuardianDialogue(Entity guardianEntity, const std::string& message) {

	// Store & remove existing textbox
	Textbox old_textbox_copy;
	for (Entity tb : registry.textboxes.entities) {
		if (registry.textboxes.get(tb).targetItem == guardianEntity) {
			old_textbox_copy = registry.textboxes.get(tb);
			m_ui_system->removeRmlUITextbox(tb.id());
			registry.remove_all_components_of(tb);
			break;
		}
	}

	Entity temp = createTextbox(renderer, old_textbox_copy.pos, guardianEntity, message);
	if (m_ui_system) {
		m_ui_system->textboxes[temp.id()] = registry.textboxes.get(temp);
	}

	// Recreate original textbox
	Entity restored = Entity();
	Textbox& new_tb = registry.textboxes.emplace(restored);
	new_tb = old_textbox_copy;
	new_tb.isVisible = false;
}

bool WorldSystem::handleGuardianUnlocking(Entity guardianEntity) {
	Entity player = registry.players.entities[0];
	Inventory& player_inventory = registry.inventories.get(player);
	Guardian& guardian = registry.guardians.get(guardianEntity);
	if (guardian.received_potion) return true;

	// Check if the correct potion is in the player's inventory
	for (auto it = player_inventory.items.begin(); it != player_inventory.items.end(); ++it)
	{
		Entity itemEntity = *it;

		if (registry.potions.has(itemEntity)) {
			Potion& potion = registry.potions.get(itemEntity);

			if (potion.effect == guardian.unlock_potion)
			{
				// Remove potion from inventory
				registry.items.get(itemEntity).amount -= 1;
				if (registry.items.get(itemEntity).amount <= 0) {
					ItemSystem::removeItemFromInventory(player, itemEntity);
				}

				ScreenState& screen = registry.screenStates.components[0];
				// Unlock corresponding biome
				if (registry.items.get(guardianEntity).type == ItemType::DESERT_GUARDIAN)
				{
					if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "desert") == screen.unlocked_biomes.end()) {
						screen.unlocked_biomes.push_back("desert");
					}
				}
				else if (registry.items.get(guardianEntity).type == ItemType::MUSHROOM_GUARDIAN)
				{
					if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "mushroom") == screen.unlocked_biomes.end()) {
						screen.unlocked_biomes.push_back("mushroom");
					}
					createForestToMushroom(renderer, vec2(GRID_CELL_WIDTH_PX * 2.1, WINDOW_HEIGHT_PX - 40), "Mushroom Entrance");
				}
				else if (registry.items.get(guardianEntity).type == ItemType::CRYSTAL_GUARDIAN)
				{
					// unlocks both crystal and mushroom since crystal is technically end game
					if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "mushroom") == screen.unlocked_biomes.end()) {
						screen.unlocked_biomes.push_back("mushroom");
					}
					if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "crystal") == screen.unlocked_biomes.end()) {
						screen.unlocked_biomes.push_back("crystal");
					}
					createForestExToCrystal(renderer, vec2(930, 665), "Forest Ex to Crystal");
					createMushroomToCrystal(renderer, vec2(1220, 160), "Mushroom to Crystal");

				}
				else if (registry.items.get(guardianEntity).type == ItemType::MASTER_POTION_PEDESTAL)
				{
					// persist that we have finished the game
					if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "saved-grotto") == screen.unlocked_biomes.end()) {
						screen.unlocked_biomes.push_back("saved-grotto");
					}
					createRejuvenationPotion(renderer);
					screen.play_ending = true; // initially play_ending (set to false later)
					screen.saved_grotto = true; // this flag gets set so enemies don't spawn
					createGlowEffect(renderer, false); // do initial grow at start
				}

				// remove textbox
				// for (auto textbox : registry.textboxes.entities) {
				// 	if (registry.textboxes.get(textbox).targetItem == guardianEntity) {
				// 		std::cout << "removing textbox" << std::endl;
				// 		m_ui_system->removeRmlUITextbox(textbox.id());
				// 		registry.remove_all_components_of(textbox);
				// 	}
				// }

				// Set guardian movement to the direction of the exit
				showTemporaryGuardianDialogue(guardianEntity, guardian.success_dialogue);
				if (registry.motions.has(guardianEntity) && guardian.exit_direction != vec2(0, 0)) {
					if (!registry.delayedMovements.has(guardianEntity)) {
						DelayedMovement& delay = registry.delayedMovements.emplace(guardianEntity);
						delay.velocity = guardian.exit_direction * GUARDIAN_SPEED;
						delay.delay_ms = 2000.f; // 2 seconds to let player read dialogue
					}
				}

				if (registry.items.has(guardianEntity)) {
					Item& item = registry.items.get(guardianEntity);
					if (item.type == ItemType::MASTER_POTION_PEDESTAL) {
						createTextbox(renderer, vec2(558, 40), guardianEntity, "Congratulations, you've saved the grotto!");
					}
				}

				std::cout << "You got da potion" << std::endl;
				guardian.received_potion = true;
				m_ui_system->updateInventoryBar();

				return true;
			}
		}
	}
	std::cout << "You don't have the potion!!" << std::endl;

	showTemporaryGuardianDialogue(guardianEntity, guardian.wrong_potion_dialogue);

	return false;
}


void WorldSystem::updatePlayerState(Entity& player, Motion& player_motion, float elapsed_ms_since_last_update) {

	Animation& player_animation = registry.animations.get(player);

	// Update velocity based on active keys
	player_motion.velocity = { 0, 0 }; // Reset velocity before recalculating

	// no movement while menu is open, switching biome, or on welcome screen
	ScreenState& screen = registry.screenStates.components[0];
	if ((m_ui_system != nullptr && (m_ui_system->isCauldronOpen() || m_ui_system->isMortarPestleOpen() || m_ui_system->isRecipeBookOpen())) || screen.is_switching_biome || screen.tutorial_state == (int)TUTORIAL::WELCOME_SCREEN) return;

	Player& player_comp = registry.players.get(player);

	if (pressed_keys.size() > 0) {
		player_comp.walking_timer -= elapsed_ms_since_last_update;
		if (player_comp.walking_timer <= 0) {
			SoundSystem::playWalkSound((int)SOUND_CHANNEL::WALK, 0);
			player_comp.walking_timer = PLAYER_WALKING_SOUND_TIMER;
		}
	}

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

	// add any active speed boost
	player_motion.velocity *= player_comp.speed_multiplier;

	// move the character
	player_motion.previous_position = player_motion.position;
	float delta = elapsed_ms_since_last_update * TIME_UPDATE_FACTOR;
	player_motion.position += delta * player_motion.velocity;

	// update player's throwing cooldown
	if (player_comp.cooldown > 0) {
		player_comp.cooldown -= elapsed_ms_since_last_update;
		if (player_comp.cooldown <= 0) {
			player_comp.cooldown = 0;
		}
	}

	// update player's damage cooldown
	if (player_comp.damage_cooldown > 0) player_comp.damage_cooldown -= elapsed_ms_since_last_update;
	else {
		player_comp.damage_cooldown = 0.f;
	}

	// update player's active effects
	updateConsumedPotions(elapsed_ms_since_last_update);

	// update regen timer
	if (registry.regen.has(player)) {
		Regeneration& regen = registry.regen.get(player);
		regen.timer -= elapsed_ms_since_last_update;
		if (regen.timer <= 0) {
			player_comp.health = std::min(player_comp.health + regen.heal_amount, PLAYER_MAX_HEALTH);
			regen.timer = REGEN_TIMER;
			m_ui_system->updateHealthBar();
		}
	}

}

bool WorldSystem::throwAmmo(vec2 target) {
	if (registry.players.entities.empty()) return false;
	Entity player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);

	if (player.cooldown > 0.f) {
		std::cout << "throwing on cooldown" << std::endl;
		return false;
	}

	if (!registry.inventories.has(player_entity) || player.cooldown > 0.f || !registry.motions.has(player_entity)) return false;
	Inventory& inventory = registry.inventories.get(player_entity);
	if (inventory.items.size() < inventory.selection + 1) {
		std::cout << "cannot throw selected item" << std::endl;
		return false;
	}

	Entity& item_entity = inventory.items[inventory.selection];

	if (createFiredAmmo(renderer, target, item_entity, player_entity)) {
		if (registry.items.has(item_entity)) {
			Item& item = registry.items.get(item_entity);
			item.amount -= 1;
			if (item.amount == 0) {
				ItemSystem::removeItemFromInventory(player_entity, item_entity);
				if (inventory.selection + 1 > inventory.items.size()) inventory.selection = std::max(inventory.selection - 1, 0);
			}
		}
		player.cooldown = PLAYER_THROW_COOLDOWN;
		if (registry.players.has(player_entity) && m_ui_system != nullptr) {
			m_ui_system->updateInventoryBar();
		}
		return true;
	}

	return false;

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

		// make ammo rotate
		ammo_motion.angle += 5.f;

	}
}

void WorldSystem::updateFPS(float elapsed_ms)
{
	m_frame_time_sum -= m_frame_times[m_frame_time_index];
	m_frame_times[m_frame_time_index] = elapsed_ms;
	m_frame_time_sum += elapsed_ms;

	m_frame_time_index = (m_frame_time_index + 1) % 60;

	// Calculate average FPS over the last 60 frames
	float avg_frame_time = m_frame_time_sum / 60.0f;
	if (avg_frame_time > 0) {
		m_current_fps = 1000.0f / avg_frame_time;
		renderer->setFPS(m_current_fps);
	}

	// Update timer for display refresh
	m_fps_update_timer += elapsed_ms;
}

void WorldSystem::updateConsumedPotions(float elapsed_ms_since_last_update) {
	if (registry.players.entities.size() == 0) return;
	Entity player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);
	std::vector<Entity> to_remove = {};
	for (Entity effect : player.active_effects) {
		if (!registry.potions.has(effect)) continue; // only potions should be added

		Potion& potion = registry.potions.get(effect);
		potion.duration -= elapsed_ms_since_last_update;
		if (potion.duration <= 0) {
			std::cout << "potion of effect id " << (int)potion.effect << " has expired" << std::endl;
			removePotionEffect(registry.potions.get(effect), player_entity);
			to_remove.push_back(effect);
		}
	}

	for (Entity entity : to_remove) {
		player.active_effects.erase(std::remove(player.active_effects.begin(), player.active_effects.end(), entity), player.active_effects.end());
		registry.remove_all_components_of(entity);
	}

	// if a potion expired, then update the effect bar, needs to update after active_effects has been modified
	if (to_remove.size() > 0) {
		if (m_ui_system) m_ui_system->updateEffectsBar();
	}
}

bool WorldSystem::consumePotion() {
	// get the item in the selected inventory slot
	Entity player_entity = registry.players.entities[0];
	Inventory& inv = registry.inventories.get(player_entity);
	if (inv.selection >= inv.items.size()) {
		return false;
	}

	Entity selected_item = inv.items[inv.selection];
	if (!registry.items.has(selected_item)) {
		return false;
	}

	if (!registry.potions.has(selected_item)) {
		return false;
	}

	// Check that the potion is consumable
	if (std::find(consumable_potions.begin(), consumable_potions.end(), registry.potions.get(selected_item).effect) == consumable_potions.end()) {
		return false;
	}

	// replace any potion of the same type currently active with the new one
	Player& player = registry.players.get(player_entity);
	for (int i = 0; i < player.active_effects.size(); i++) {
		Entity entity = player.active_effects[i];
		if (!registry.potions.has(entity)) continue;

		Potion& potion = registry.potions.get(entity);
		if (potion.effect == registry.potions.get(selected_item).effect) {
			removePotionEffect(potion, player_entity);
			player.active_effects.erase(player.active_effects.begin() + i);
			registry.remove_all_components_of(entity);
			break;
		}
	}

	Entity item_copy = ItemSystem::copyItem(selected_item);

	// decrement count in inventory
	Item& item = registry.items.get(selected_item);
	item.amount -= 1;
	if (item.amount <= 0) {
		ItemSystem::removeItemFromInventory(player_entity, selected_item);
		ItemSystem::destroyItem(selected_item);
	}

	// add copy of item to player's active effects - health is instant so don't add to active_effects
	assert(registry.potions.has(item_copy) && "consumed item should be a potion");
	assert(registry.items.has(item_copy) && "consumed item should be an item");

	addPotionEffect(registry.potions.get(item_copy), player_entity);

	if (registry.potions.get(item_copy).effect != PotionEffect::HEALTH) {
		player.active_effects.push_back(item_copy);
	}

	else {
		if (m_ui_system) m_ui_system->updateHealthBar();
	}
	if (m_ui_system) {
		m_ui_system->updateInventoryBar();
		m_ui_system->updateEffectsBar();
	}

	SoundSystem::playGulpSound((int)SOUND_CHANNEL::GENERAL, 0);
	std::cout << "player has consumed a potion of " << (int)registry.potions.get(item_copy).effect << std::endl;

	return true;
}

void WorldSystem::addPotionEffect(Potion& potion, Entity player) {
	Player& player_comp = registry.players.get(player);

	switch (potion.effect) {
	case PotionEffect::SPEED: {
		player_comp.speed_multiplier = player_comp.effect_multiplier * potion.effectValue; // needs to be set separately because we reset velocity every time in updatePlayerState
		break;
	}
	case PotionEffect::HEALTH: {
		player_comp.health = std::min(player_comp.health + player_comp.effect_multiplier * potion.effectValue, PLAYER_MAX_HEALTH);
		break;
	}
	case PotionEffect::REGEN: {
		Regeneration& regen = registry.regen.emplace(player);
		regen.heal_amount = potion.effectValue * player_comp.effect_multiplier;
		regen.timer = 1000.f; // regen every second
		break;
	}
	case PotionEffect::RESISTANCE: {
		player_comp.defense = 1 - (player_comp.effect_multiplier * potion.effectValue); // percent of damage reduced
		break;
	}
	case PotionEffect::SATURATION: {
		player_comp.effect_multiplier = potion.effectValue;
		break;
	}
	default: {
		break;
	}
	}

	if (m_ui_system) m_ui_system->updateEffectsBar();
}

void WorldSystem::removePotionEffect(Potion& potion, Entity player) {
	Player& player_comp = registry.players.get(player);

	switch (potion.effect) {
	case PotionEffect::SPEED:
		player_comp.speed_multiplier = 1.f;
		break;
	case PotionEffect::REGEN:
		if (registry.regen.has(player)) {
			registry.regen.remove(player);
		}
		break;
	case PotionEffect::RESISTANCE:
		player_comp.defense = 1.f;
		break;
	case PotionEffect::SATURATION:
		player_comp.effect_multiplier = 1.f;
		break;
	default:
		break;
	}
}

void WorldSystem::handleEnemyInjured(Entity enemy_entity, float damage = 0.f) {
	if (registry.players.entities.size() == 0 || !enemy_entity) return;
	Player& player = registry.players.components[0];
	ScreenState& screen = registry.screenStates.components[0];
	Enemy& enemy = registry.enemies.get(enemy_entity);

	enemy.health -= damage * player.effect_multiplier;
	m_ui_system->updateEnemyHealth(enemy_entity, enemy.health / enemy.max_health);
	registry.damageFlashes.remove(enemy_entity);
	registry.damageFlashes.emplace(enemy_entity);
	if (enemy.health <= 0) {
		if (!enemy.persistentID.empty()) {
			// Set a respawn timer between 120-180 seconds (longer than items)
			float respawnTime = (rand() % 60000 + 120000);
			RespawnSystem::getInstance().registerEntity(enemy_entity, false);
			RespawnSystem::getInstance().setRespawning(enemy.persistentID, respawnTime);
			std::cout << "Enemy " << enemy.name << " killed, will respawn in "
				<< respawnTime / 1000.0f << " seconds" << std::endl;
		}

		if (enemy.name == "Ent") {
			createCollectableIngredient(renderer, registry.motions.get(enemy_entity).position, ItemType::STORM_BARK, 1, false);
		}
		else if (enemy.name == "Mummy 1" || enemy.name == "Mummy 2") {
			createCollectableIngredient(renderer, registry.motions.get(enemy_entity).position, ItemType::MUMMY_BANDAGES, 1, false);
		}

		// add enemy name to killed_enemies for persistence
		if (std::find(screen.killed_enemies.begin(), screen.killed_enemies.end(), enemy.name) == screen.killed_enemies.end()) {
			screen.killed_enemies.push_back(enemy.name);
		}

		registry.remove_all_components_of(enemy_entity);
	}
	SoundSystem::playEnemyOuchSound((int)SOUND_CHANNEL::GENERAL, 0); // play enemy ouch sound
}