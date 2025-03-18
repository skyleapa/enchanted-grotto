#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"
#include <cstdlib>
#include <ctime>

void AISystem::step(float elapsed_ms) {
	if (registry.screenStates.components[0].is_switching_biome)
	{
		for (Entity enemy : registry.enemies.entities) {
			if (registry.motions.has(enemy)) {
				registry.motions.get(enemy).velocity = {0,0};
			}
		}
		return;
	}
	for (const Entity& player : registry.players.entities) {
		for (const Entity& enemy : registry.enemies.entities) {
			updateEnemyAI(elapsed_ms, enemy, player);
		}
	}
}

void AISystem::updateEnemyAI(float elapsed_ms, Entity enemy_entity, Entity player_entity) {
	Motion& enemy_motion = registry.motions.get(enemy_entity);
	Enemy& enemy = registry.enemies.get(enemy_entity);
	Motion& player_motion = registry.motions.get(player_entity);

	if (enemy.can_move == 0) {
		return;
	}

	float distance_to_player = glm::length(player_motion.position - enemy_motion.position);
	float distance_to_spawn = glm::length(enemy.start_pos - enemy_motion.position);

	// Decision tree
	std::vector<DecisionTreeNode> decisionTree = {
		// If in DETECTION_RADIUS, go attack
		DecisionTreeNode([&]() { return distance_to_player < DETECTION_RADIUS; }, ENEMY_STATE::ATTACK, static_cast<ENEMY_STATE>(enemy.state)),
		// If out of FOLLOWING_RADIUS, go WANDER 
		DecisionTreeNode([&]() { return (distance_to_player > FOLLOWING_RADIUS) && (enemy.state == static_cast<int>(ENEMY_STATE::ATTACK)); }, ENEMY_STATE::WANDER, static_cast<ENEMY_STATE>(enemy.state)),
		// If timer=0, go RETURN
		DecisionTreeNode([&]() { return (enemy.wander_timer <= 0) && (enemy.state == static_cast<int>(ENEMY_STATE::WANDER)); }, ENEMY_STATE::RETURN, ENEMY_STATE::WANDER),
		// If back at spawn, go IDLE
		DecisionTreeNode([&]() { return (distance_to_spawn < 3.0f) && (enemy.state == static_cast<int>(ENEMY_STATE::WANDER)); }, ENEMY_STATE::IDLE, static_cast<ENEMY_STATE>(enemy.state))
	};

	// Traverse decision tree to decide next state
	for (auto& node : decisionTree) {
		if (node.condition()) {
			enemy.state = static_cast<int>(node.trueState);
			break; // Exit the loop once a condition is met
		}
		else {
			enemy.state = static_cast<int>(node.falseState);
		}
	}

	if (enemy.state == static_cast<int>(ENEMY_STATE::ATTACK)) {
		moveEnemyTowardsPlayer(enemy_motion, player_motion, elapsed_ms);
		//std::cout << "Enemy is attacking the player!\n";
	}
	else if (enemy.state == static_cast<int>(ENEMY_STATE::WANDER)) {
		//std::cout << "Enemy is wandering" << std::endl;
		enemy.wander_timer -= elapsed_ms / 200.0f;
		moveEnemyRandomly(enemy_motion, elapsed_ms);
	}
	else if (enemy.state == static_cast<int>(ENEMY_STATE::RETURN)) {
		moveEnemyTowardsSpawn(enemy_motion, enemy.start_pos, elapsed_ms);
		//std::cout << "Enemy is returning to spawn" << std::endl;
		enemy.wander_timer = 10.0f;
	}
}

void AISystem::moveEnemyTowardsPlayer(Motion& enemy_motion, Motion& player_motion, float elapsed_ms) {
	glm::vec2 direction = glm::normalize(player_motion.position - enemy_motion.position);
	glm::vec2 next_position = enemy_motion.position + direction * ENEMY_SPEED * (elapsed_ms / 1000.0f);

	enemy_motion.position = handleCollision(enemy_motion, next_position, direction, elapsed_ms);
}

void AISystem::moveEnemyRandomly(Motion& enemy_motion, float elapsed_ms) {
	static bool initialized = false;
	static glm::vec2 current_direction = glm::vec2(1.0f, 0.0f);
	static float direction_timer = 0.0f; 

	if (!initialized) {
		srand(time(nullptr));  // Seed random generator once
		initialized = true;
	}

	// Update direction timer
	direction_timer -= elapsed_ms / 1000.0f;

	// If timer expires, choose a new random direction
	if (direction_timer <= 0.0f) {
		float angle = (rand() % 360) * (3.14159f / 180.0f);
		current_direction = glm::vec2(cos(angle), sin(angle));
		direction_timer = 3.0f;
	}

	glm::vec2 next_position = enemy_motion.position + current_direction * 0.5f * ENEMY_SPEED * (elapsed_ms / 500.0f);

	enemy_motion.position = handleCollision(enemy_motion, next_position, current_direction, elapsed_ms);
}

void AISystem::moveEnemyTowardsSpawn(Motion& enemy_motion, glm::vec2 spawn_position, float elapsed_ms) {
	glm::vec2 direction = glm::normalize(spawn_position - enemy_motion.position);
	glm::vec2 next_position = enemy_motion.position + direction * ENEMY_SPEED * (elapsed_ms / 1000.0f);

	enemy_motion.position = handleCollision(enemy_motion, next_position, direction, elapsed_ms);
}

glm::vec2 AISystem::handleCollision(const Motion& entity_motion, glm::vec2 next_position, glm::vec2 direction, float elapsed_ms) {
	Motion next_motion = entity_motion;
	next_motion.position = next_position;

	if (!isCollision(next_motion)) {
		return next_position;
	}

	// Try alternative movement (sideways)
	glm::vec2 alternative_direction = glm::vec2(direction.y, -direction.x);
	glm::vec2 alternative_position = entity_motion.position + alternative_direction * ENEMY_SPEED * (elapsed_ms / 1000.0f);

	next_motion.position = alternative_position;
	if (!isCollision(next_motion)) {
		return alternative_position;
	}

	// If still colliding, move backward
	glm::vec2 backward_position = entity_motion.position - direction * ENEMY_SPEED * (elapsed_ms / 1000.0f);
	next_motion.position = backward_position;
	if (!isCollision(next_motion)) {
		return backward_position;
	}

	// stay in place
	return entity_motion.position;
}

bool AISystem::isCollision(const Motion& entity_motion) {
	for (const Entity& terrain_entity : registry.terrains.entities) {
		Terrain& terrain = registry.terrains.get(terrain_entity);
		Motion& terrain_motion = registry.motions.get(terrain_entity);

		// Using collides() from physics_system
		if (collides(entity_motion, terrain_motion, &terrain)) {
			return true;  // Collision detected
		}
	}
	return false;  // No collision detected
}

// collision logic from physics_system

vec4 AISystem::get_bounding_box(const Motion& motion, float width_ratio, float height_ratio)
{
	// gets the full bounding box
	float full_width = abs(motion.scale.x);
	float full_height = abs(motion.scale.y);

	// compute a bottom centered bounding box
	float box_width = full_width * width_ratio;
	float box_height = full_height * height_ratio;

	float box_x = motion.position.x - box_width / 2;				// center the box on x-axis
	float box_y = motion.position.y + full_height / 2 - box_height; // put the box on the bottom

	return { box_x, box_y, box_width, box_height };
}

bool AISystem::collides(const Motion& player_motion, const Motion& terrain_motion, const Terrain* terrain)
{
	// default full bounding box size
	float player_width_ratio = 1.0f, player_height_ratio = 1.0f;
	float terrain_width_ratio = 1.0f, terrain_height_ratio = 1.0f;

	// apply a smaller bounding box for the player
	player_width_ratio = 0.7f;
	player_height_ratio = 0.3f;

	// apply bottom collision box for terrain with collision setting = 0
	if (terrain && terrain->collision_setting == 0)
	{
		terrain_width_ratio = terrain->width_ratio;
		terrain_height_ratio = terrain->height_ratio;
	}
	
	// gets our bounding boxes for the player and terrain
	vec4 player_box = get_bounding_box(player_motion, player_width_ratio, player_height_ratio);
	vec4 terrain_box = get_bounding_box(terrain_motion, terrain_width_ratio, terrain_height_ratio);

	// calculate our AABB overlapping bounding boxes
	bool overlap_x = (player_box.x < terrain_box.x + terrain_box.z) && (player_box.x + player_box.z > terrain_box.x);
	bool overlap_y = (player_box.y < terrain_box.y + terrain_box.w) && (player_box.y + player_box.w > terrain_box.y);

	return overlap_x && overlap_y;
}