
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "systems/ai_system.hpp"
#include "systems/physics_system.hpp"
#include "systems/render_system.hpp"
#include "systems/world_system.hpp"
#include "systems/item_system.hpp"
#include "systems/potion_system.hpp"
#include "systems/ui_system.hpp"
#include "systems/sound_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem	  ai_system;
	WorldSystem   world_system;
	RenderSystem  renderer_system;
	PhysicsSystem physics_system;
	ItemSystem    item_system;
	PotionSystem  potion_system;
	BiomeSystem   biome_system;
	UISystem      ui_system;
	SoundSystem	  sound_system;

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!sound_system.startAndLoadSounds()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	// initialize the main systems
	renderer_system.init(window);
	world_system.init(&renderer_system, &biome_system);
	biome_system.init(&renderer_system);

	// Initialize UI system last (after all other systems) and set reference in world system 
	bool ui_initialized = ui_system.init(window, &renderer_system);
	if (ui_initialized) {
		world_system.setUISystem(&ui_system);
		item_system.setUISystem(&ui_system);
		biome_system.setUISystem(&ui_system);
		ai_system.setUISystem(&ui_system);
		glfwSetCharCallback(window, UISystem::charCallback);
		std::cout << "UI system initialized successfully" << std::endl;
	}
	else {
		std::cerr << "Failed to initialize UI system, continuing without UI" << std::endl;
	}

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {

		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// CK: be mindful of the order of your systems and rearrange this list only if necessary
		world_system.step(elapsed_ms);
		ai_system.step(elapsed_ms);
		physics_system.step(elapsed_ms);
		item_system.step(elapsed_ms);
		potion_system.updateCauldrons(elapsed_ms);
		world_system.handle_collisions(elapsed_ms);
		biome_system.step(elapsed_ms);
		ui_system.step(elapsed_ms);

		renderer_system.draw(&ui_system, elapsed_ms);
		renderer_system.swap_buffers();
	}

	// Save game state before exit
	item_system.saveGameState();

	return EXIT_SUCCESS;
}
