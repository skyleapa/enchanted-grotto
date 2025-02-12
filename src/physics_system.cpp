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

	float box_x = motion.position.x - box_width / 2;   // center the box on x-axis
	float box_y = motion.position.y + full_height / 2 - box_height; // put the box on the bottom

	return { box_x, box_y, box_width, box_height };
}

bool collides(const Motion& motion1, const Motion& motion2, const Terrain* terrain1, const Terrain* terrain2, bool is_player1, bool is_player2)
{
    float width_ratio1 = 1.0f, height_ratio1 = 1.0f;
    float width_ratio2 = 1.0f, height_ratio2 = 1.0f;

    // if entity is a player, apply a smaller bounding box
    if (is_player1) {
        width_ratio1 = 0.7f; 
        height_ratio1 = 0.3f;
    }
    if (is_player2) {
        width_ratio2 = 0.7f;
        height_ratio2 = 0.3f;
    }

    if (terrain1 && terrain1->collision_setting == 0) {
        width_ratio1 = terrain1->width_ratio;
        height_ratio1 = terrain1->height_ratio;
    }

    if (terrain2 && terrain2->collision_setting == 0) {
        width_ratio2 = terrain2->width_ratio;
        height_ratio2 = terrain2->height_ratio;
    }

    vec4 box1 = get_bounding_box(motion1, width_ratio1, height_ratio1);
    vec4 box2 = get_bounding_box(motion2, width_ratio2, height_ratio2);

    bool overlap_x = (box1.x < box2.x + box2.z) && (box1.x + box1.z > box2.x);
    bool overlap_y = (box1.y < box2.y + box2.w) && (box1.y + box1.w > box2.y);

    return overlap_x && overlap_y;
}


void PhysicsSystem::step(float elapsed_ms)
{	
	ComponentContainer<Motion> &motion_container = registry.motions;
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];

		for (uint j = i + 1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			Entity entity_j = motion_container.entities[j];

			bool is_terrain1 = registry.terrains.has(entity_i);
			bool is_terrain2 = registry.terrains.has(entity_j);
			bool is_player1 = registry.players.has(entity_i);
			bool is_player2 = registry.players.has(entity_j);

			Terrain* terrain1 = is_terrain1 ? &registry.terrains.get(entity_i) : nullptr;
			Terrain* terrain2 = is_terrain2 ? &registry.terrains.get(entity_j) : nullptr;
			
			// only check collisions if one is a player and the other is terrain
			if ((is_player1 && is_terrain2) || (is_player2 && is_terrain1))
			{
				if (collides(motion_i, motion_j, terrain1, terrain2, is_player1, is_player2))
				{
					registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				}
			}
		}
	}
}