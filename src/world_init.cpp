#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

Entity createTree(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.height_ratio = 0.3f;
	terrain.width_ratio = 0.25f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ TREE_WIDTH, TREE_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TREE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::TERRAIN
		}
	);

	return entity;
}

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

Entity createPlayer(RenderSystem* renderer, vec2 position)
{
	// reserve an entity
	auto entity = Entity();
	Player& player = registry.players.emplace(entity);
	player.name = "Madoka";

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 80.f, 80.f };
	motion.position = position;
	motion.moving_direction = (int) DIRECTION::DOWN;

	motion.scale = vec2({ PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });

	registry.eatables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::PLAYER
		}
	);

	return entity;
}

Entity createForestBridge(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 1.0f;
	terrain.height_ratio = 0.2f;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	motion.scale = vec2({ FOREST_BRIDGE_WIDTH, FOREST_BRIDGE_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::FOREST_BRIDGE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			0 // this is the render sub layer (bridge should be above river)
		}
	);

	return entity;
}


Entity createForestRiver(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 1.0f; // rivers are not walkable

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	motion.scale = vec2({ FOREST_RIVER_WIDTH, FOREST_RIVER_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::FOREST_RIVER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			1 // this is the render sub layer (river should be below bridge)
		}
	);

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