#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!! TODO A1: scan for invaders and shoot at them
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// invader detection system for towers
	// - for each tower, scan its row:
	//   - if an invader is detected and the tower's shooting timer has expired,
	//     then shoot (create a projectile) and reset the tower's shot timer
	for (const Entity& tower_entity : registry.towers.entities) {
	}
}