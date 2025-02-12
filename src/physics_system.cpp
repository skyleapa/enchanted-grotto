// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

vec4 get_bounding_box(const Motion& motion, float width_ratio = 1.0f, float height_ratio = 0.3f)
{
	// gets the full bounding box
	float full_width = abs(motion.scale.x);
	float full_height = abs(motion.scale.y);

	// compute a bottom centered bounding box
	float box_width = full_width * width_ratio;
	float box_height = full_height * height_ratio;

	float box_x = motion.position.x - box_width / 2;   // center the box on x-axis
	float box_y = motion.position.y + full_height / 2 - box_height; // put the box on the bottom

	return { box_x, box_y, box_width, box_height };
}

// use_bottom_box1 = to only check for the bottom region collides
bool collides(const Motion& motion1, const Motion& motion2, bool use_bottom_box1 = false, bool use_bottom_box2 = false)
{
	// get bounding boxes (can specify the size)
	vec4 box1 = use_bottom_box1 ? get_bounding_box(motion1) : get_bounding_box(motion1, 0.2f, 0.1f);
	vec4 box2 = use_bottom_box2 ? get_bounding_box(motion2) : get_bounding_box(motion2, 0.2f, 0.1f);

	// using aabb collision
	bool overlap_x = (box1.x < box2.x + box2.z) && (box1.x + box1.z > box2.x);
	bool overlap_y = (box1.y < box2.y + box2.w) && (box1.y + box1.w > box2.y);

	return overlap_x && overlap_y;
}

void PhysicsSystem::step(float elapsed_ms)
{	
	// check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			Entity entity_j = motion_container.entities[j];

			bool is_terrain1 = registry.terrains.has(entity_i);
			bool is_terrain2 = registry.terrains.has(entity_j);
			bool is_player1 = registry.invaders.has(entity_i);
			bool is_player2 = registry.invaders.has(entity_j);

			// If one entity is terrain and the other is a player, use the bottom bounding box
			bool use_bottom_box1 = is_player1 && is_terrain2;
			bool use_bottom_box2 = is_player2 && is_terrain1;

			if (collides(motion_i, motion_j, use_bottom_box1, use_bottom_box2))
			{
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
			}
		}
	}
}