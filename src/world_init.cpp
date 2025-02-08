#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createGridLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();

	GridLine& gridLine = registry.gridLines.emplace(entity);
	gridLine.start_pos = start_pos;
	gridLine.end_pos = end_pos;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);

	registry.colors.insert(
        entity,
        glm::vec3(0.1f, 0.1f, 0.1f)
    );

	return entity;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createInvader(RenderSystem* renderer, vec2 position)
{
	// reserve an entity
	auto entity = Entity();

	// invader
	Invader& invader = registry.invaders.emplace(entity);
	invader.health = INVADER_HEALTH;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// TODO A1: initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 50.f, 50.f };
	motion.position = position;
	motion.moving_direction = (int) DIRECTION::DOWN;

	// resize, set scale to negative if you want to make it face the opposite way
	// motion.scale = vec2({ -INVADER_BB_WIDTH, INVADER_BB_WIDTH });
	motion.scale = vec2({ INVADER_BB_WIDTH, INVADER_BB_HEIGHT });

	// create an (empty) Bug component to be able to refer to all bug
	registry.eatables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::INVADER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createTower(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// new tower
	auto& t = registry.towers.emplace(entity);
	t.range = (float)WINDOW_WIDTH_PX / (float)GRID_CELL_WIDTH_PX;
	t.timer_ms = TOWER_TIMER_MS;	// arbitrary for now

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;	// A1-TD: CK: rotate to the left 180 degrees to fix orientation
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	std::cout << "INFO: tower position: " << position.x << ", " << position.y << std::endl;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ -TOWER_BB_WIDTH, TOWER_BB_HEIGHT });

	// create an (empty) Tower component to be able to refer to all towers
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TOWER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

void removeTower(vec2 position) {
	// remove any towers at this position
	for (Entity& tower_entity : registry.towers.entities) {
		// get each tower's position to determine it's row
		const Motion& tower_motion = registry.motions.get(tower_entity);
		
		if (tower_motion.position.y == position.y) {
			// remove this tower
			registry.remove_all_components_of(tower_entity);
			std::cout << "tower removed" << std::endl;
		}
	}
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! TODO A1: create a new projectile w/ pos, size, & velocity
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createProjectile(vec2 pos, vec2 size, vec2 velocity)
{
	auto entity = Entity();

	// TODO: projectile
	// TODO: motion
	// TODO: renderRequests

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{
			// usage TEXTURE_COUNT when no texture is needed, i.e., an .obj or other vertices are used instead
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

// LEGACY
Entity createChicken(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::CHICKEN);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 300.f;
	motion.scale.y *= -1; // point front to the right

	// create an (empty) Chicken component to be able to refer to all towers
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			// usage TEXTURE_COUNT when no texture is needed, i.e., an .obj or other vertices are used instead
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::CHICKEN,
			GEOMETRY_BUFFER_ID::CHICKEN
		}
	);

	return entity;
}