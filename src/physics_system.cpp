// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

vec4 get_bounding_box(const Motion &motion, float width_ratio, float height_ratio)
{
	// gets the full bounding box
	float full_width = abs(motion.scale.x);
	float full_height = abs(motion.scale.y);

	// compute a bottom centered bounding box
	float box_width = full_width * width_ratio;
	float box_height = full_height * height_ratio;

	float box_x = motion.position.x - box_width / 2;				// center the box on x-axis
	float box_y = motion.position.y + full_height / 2 - box_height; // put the box on the bottom

	return {box_x, box_y, box_width, box_height};
}

bool collides(const Motion &player_motion, const Motion &terrain_motion, const Terrain *terrain)
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

	vec4 player_box = get_bounding_box(player_motion, player_width_ratio, player_height_ratio);
	vec4 terrain_box = get_bounding_box(terrain_motion, terrain_width_ratio, terrain_height_ratio);

	bool overlap_x = (player_box.x < terrain_box.x + terrain_box.z) && (player_box.x + player_box.z > terrain_box.x);
	bool overlap_y = (player_box.y < terrain_box.y + terrain_box.w) && (player_box.y + player_box.w > terrain_box.y);

	return overlap_x && overlap_y;
}

bool enterEntrance(const Motion &player_motion, const Motion &entrance_motion)
{
	float player_width_ratio = 1.0f, player_height_ratio = 1.0f;
	float entrance_width_ratio = 0.7f, entrance_height_ratio = 0.9f;

	vec4 player_box = get_bounding_box(player_motion, player_width_ratio, player_height_ratio);
	vec4 entrance_box = get_bounding_box(entrance_motion, entrance_width_ratio, entrance_height_ratio);

	// add padding to bottom so that character is fully within entrance
	float padding = 5;
	entrance_box.w -= padding;
	entrance_box.y -= padding;

	bool fully_contained_x = (player_box.x >= entrance_box.x) &&
							 (player_box.x + player_box.z <= entrance_box.x + entrance_box.z);
	bool fully_contained_y = (player_box.y >= entrance_box.y) &&
							 (player_box.y + player_box.w <= entrance_box.y + entrance_box.w);

	return fully_contained_x && fully_contained_y;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// get our one player
	if (registry.players.entities.empty())
		return;
		
	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity))
		return;

	Motion &player_motion = registry.motions.get(player_entity);	

	for (Entity terrain_entity : registry.terrains.entities)
	{
		if (!registry.motions.has(terrain_entity))
			continue;

		Motion &terrain_motion = registry.motions.get(terrain_entity);
		Terrain &terrain = registry.terrains.get(terrain_entity);

		// only check collisions if one is a player and the other is terrain
		if (collides(player_motion, terrain_motion, &terrain))
		{
			registry.collisions.emplace_with_duplicates(player_entity, terrain_entity);
		}
	}
	for (Entity entrance : registry.entrances.entities)
	{
		if (!registry.motions.has(entrance))
			continue;

		Motion &entrance_motion = registry.motions.get(entrance);
		if (enterEntrance(player_motion, entrance_motion))
		{
			ScreenState &state = registry.screenStates.components[0];
			state.is_switching_biome = true;
			state.switching_to_biome = (GLuint)BIOME::GROTTO;
		}
	}
}
