// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

vec4 get_bounding_box(const Motion& motion, float width_ratio, float height_ratio)
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

bool collides(const Motion& player_motion, const Motion& terrain_motion, const Terrain* terrain)
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

bool enemyCollides(const Motion& other_motion, const Motion& enemy_motion)
{
	// gets our bounding boxes for the player and terrain
	vec4 other_box = get_bounding_box(other_motion, 1.f, 1.f);
	vec4 enemy_box = get_bounding_box(enemy_motion, 1.f, 1.f);

	// calculate our AABB overlapping bounding boxes
	bool overlap_x = (other_box.x < enemy_box.x + enemy_box.z) && (other_box.x + enemy_box.z > enemy_box.x);
	bool overlap_y = (other_box.y < enemy_box.y + enemy_box.w) && (other_box.y + enemy_box.w > enemy_box.y);

	return overlap_x && overlap_y;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// get our one player
	if (registry.players.entities.empty())
		return;

	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity))
		return;

	Motion& player_motion = registry.motions.get(player_entity);

	for (Entity terrain_entity : registry.terrains.entities)
	{
		if (!registry.motions.has(terrain_entity))
			continue;

		Motion& terrain_motion = registry.motions.get(terrain_entity);
		Terrain& terrain = registry.terrains.get(terrain_entity);
		
		// Collision Detection: only check collisions if one is a player and the other is terrain
		if (collides(player_motion, terrain_motion, &terrain))
		{
			registry.collisions.emplace_with_duplicates(player_entity, terrain_entity);
		}
	}

	// Check enemy collisions
	for (Entity enemy : registry.enemies.entities) {
		if (!registry.motions.has(enemy)) continue;
		Motion& enemy_motion = registry.motions.get(enemy);

		// with enemy
		for (Entity ammo_entity : registry.ammo.entities) {
			if (!registry.motions.has(ammo_entity)) continue;
			Motion& ammo_motion = registry.motions.get(ammo_entity);
			Ammo &ammo = registry.ammo.get(ammo_entity);
			if (!ammo.is_fired) continue;

			if (enemyCollides(ammo_motion, enemy_motion)) {
				registry.collisions.emplace_with_duplicates(ammo_entity, enemy);
			}
		}

		// with player
		if (enemyCollides(player_motion, enemy_motion)) {
			registry.collisions.emplace_with_duplicates(player_entity, enemy);
		}
	}

}
