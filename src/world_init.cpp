#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

Entity createTree(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();
	Terrain &terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.height_ratio = 0.1f;
	terrain.width_ratio = 0.2f;

	// store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = {0, 0};
	motion.position = position;

	motion.scale = vec2({TREE_WIDTH, TREE_HEIGHT});

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::TREE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN});

	return entity;
}

Entity createGridLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();

	GridLine &gridLine = registry.gridLines.emplace(entity);
	gridLine.start_pos = start_pos;
	gridLine.end_pos = end_pos;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE,
			RENDER_LAYER::GRIDLINES,
		});

	registry.colors.insert(
		entity,
		glm::vec3(0.1f, 0.1f, 0.1f));

	return entity;
}

Entity createPlayer(RenderSystem *renderer, vec2 position)
{
	// reserve an entity
	auto entity = Entity();
	Player &player = registry.players.emplace(entity);
	player.name = "Madoka";

	// store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {PLAYER_SPEED, PLAYER_SPEED};
	motion.position = position;
	motion.moving_direction = (int)DIRECTION::DOWN;

	motion.scale = vec2({PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT});

	registry.eatables.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::PLAYER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::PLAYER});

	return entity;
}

Entity createForestBridge(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();
	auto &terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.9f;
	terrain.height_ratio = 0.35f;

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.0f, 0.0f};
	motion.position = position;

	motion.scale = vec2({FOREST_BRIDGE_WIDTH, FOREST_BRIDGE_HEIGHT});

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::FOREST_BRIDGE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			0 // this is the render sub layer (bridge should be above river)
		});

	return entity;
}

Entity createForestRiver(RenderSystem *renderer, vec2 position)
{
	auto entity1 = Entity();
	auto &terrain1 = registry.terrains.emplace(entity1);
	terrain1.collision_setting = 1.0f; // rivers are not walkable

	auto entity2 = Entity();
	auto &terrain2 = registry.terrains.emplace(entity2);
	terrain2.collision_setting = 1.0f; // rivers are not walkable

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity1, &mesh);
	registry.meshPtrs.emplace(entity2, &mesh);

	auto &motion1 = registry.motions.emplace(entity1);
	motion1.angle = 0.f;
	motion1.velocity = {0.0f, 0.0f};
	motion1.position = vec2(position.x, 200);
	motion1.scale = vec2({FOREST_RIVER_ABOVE_WIDTH, FOREST_RIVER_ABOVE_HEIGHT});

	registry.renderRequests.insert(
		entity1,
		{
			TEXTURE_ASSET_ID::FOREST_RIVER_ABOVE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			1 // this is the render sub layer (river should be below bridge)
		});

	auto &motion2 = registry.motions.emplace(entity2);
	motion2.angle = 0.f;
	motion2.velocity = {0.0f, 0.0f};
	motion2.position = vec2(position.x, 625);
	motion2.scale = vec2({FOREST_RIVER_BELOW_WIDTH, FOREST_RIVER_BELOW_HEIGHT});

	registry.renderRequests.insert(
		entity2,
		{
			TEXTURE_ASSET_ID::FOREST_RIVER_BELOW,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			1 // this is the render sub layer (river should be below bridge)
		});

	return entity1;
}

Entity createGrottoEntrance(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();
	registry.entrances.emplace(entity); // add entrance component

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = {0.0f, 0.0f};
	motion.position = position;

	motion.scale = vec2({GROTTO_ENTRANCE_WIDTH, GROTTO_ENTRANCE_HEIGHT});

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GROTTO_ENTRANCE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			0 // this is the render sub layer (bridge should be above river)
		});

	return entity;
}

Entity create_grotto_non_interactive_entities(RenderSystem *renderer, vec2 position, vec2 scale, float angle, GLuint texture_asset_id,
											  float width_ratio, float height_ratio, float can_collide)
{
	auto entity = Entity();
	if (can_collide == 1) {
		auto &terrain = registry.terrains.emplace(entity);
		terrain.collision_setting = can_collide;
		terrain.width_ratio = width_ratio;
		terrain.height_ratio = height_ratio;
	}

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.velocity = {0.0f, 0.0f};
	motion.position = position;
	motion.scale = scale;

	registry.renderRequests.insert(
		entity,
		{
			static_cast<TEXTURE_ASSET_ID>(texture_asset_id),
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			0 // this is the render sub layer (bridge should be above river)
		});

	return entity;
}

Entity create_boundary_line(RenderSystem *renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();
	auto &terrain1 = registry.terrains.emplace(entity);
	terrain1.collision_setting = 1.0f; // cannot walk past boundaries

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0;
	motion.velocity = {0.0f, 0.0f};
	motion.position = position;
	motion.scale = scale;

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::BOUNDARY_LINE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 1});

	return entity;
}