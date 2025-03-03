#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

Entity createTree(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.height_ratio = 0.1f;
	terrain.width_ratio = 0.2f;

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
		{ TEXTURE_ASSET_ID::TREE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

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
			GEOMETRY_BUFFER_ID::DEBUG_LINE,
			RENDER_LAYER::BACKGROUND,
		});

	registry.colors.insert(
		entity,
		glm::vec3(0.1f, 0.1f, 0.1f));

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
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });

	auto& inventory = registry.inventories.emplace(entity);
	inventory.capacity = 10;
	inventory.isFull = false;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::PLAYER });

	return entity;
}

Entity createForestBridge(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.9f;
	terrain.height_ratio = 0.35f;

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
		});

	return entity;
}

Entity createForestRiver(RenderSystem* renderer, vec2 position)
{
	auto entity1 = Entity();
	auto& terrain1 = registry.terrains.emplace(entity1);
	terrain1.collision_setting = 1.0f; // rivers are not walkable

	auto entity2 = Entity();
	auto& terrain2 = registry.terrains.emplace(entity2);
	terrain2.collision_setting = 1.0f; // rivers are not walkable

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity1, &mesh);
	registry.meshPtrs.emplace(entity2, &mesh);

	auto& motion1 = registry.motions.emplace(entity1);
	motion1.angle = 0.f;
	motion1.velocity = { 0.0f, 0.0f };
	motion1.position = vec2(position.x, 200);
	motion1.scale = vec2({ FOREST_RIVER_ABOVE_WIDTH, FOREST_RIVER_ABOVE_HEIGHT });

	registry.renderRequests.insert(
		entity1,
		{
			TEXTURE_ASSET_ID::FOREST_RIVER_ABOVE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			1 // this is the render sub layer (river should be below bridge)
		});

	auto& motion2 = registry.motions.emplace(entity2);
	motion2.angle = 0.f;
	motion2.velocity = { 0.0f, 0.0f };
	motion2.position = vec2(position.x, 625);
	motion2.scale = vec2({ FOREST_RIVER_BELOW_WIDTH, FOREST_RIVER_BELOW_HEIGHT });

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

Entity create_grotto_static_entities(RenderSystem* renderer, vec2 position, vec2 scale, float angle, GLuint texture_asset_id, float can_collide)
{
	auto entity = Entity();
	if (can_collide == 1)
	{
		auto& terrain = registry.terrains.emplace(entity);
		terrain.collision_setting = can_collide;
	}

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = angle;
	motion.velocity = { 0.0f, 0.0f };
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

Entity create_boundary_line(RenderSystem* renderer, vec2 position, vec2 scale)
{
	auto entity = Entity();

	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 1.0f; // cannot walk past boundaries

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;
	motion.scale = scale;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BOUNDARY_LINE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 1 });

	return entity;
}

Entity createGrottoEntrance(RenderSystem* renderer, vec2 position, int id, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::GROTTO;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::GROTTO_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ GROTTO_ENTRANCE_WIDTH, GROTTO_ENTRANCE_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x, position.y + 20 }), entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROTTO_ENTRANCE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 0 });

	return entity;
}

Entity createBush(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.height_ratio = 0.4f;
	terrain.width_ratio = 0.75f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ BUSH_WIDTH, BUSH_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BUSH,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createFruit(RenderSystem* renderer, vec2 position, int id, std::string name, int amount)
{
	auto entity = Entity();

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::MAGICAL_FRUIT;
	item.name = name;
	item.isCollectable = true; // Make sure the item can be collected
	item.amount = amount;
	item.originalPosition = position;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ FRUIT_WIDTH, FRUIT_HEIGHT });

	Entity textbox = createTextbox(renderer, position, entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FRUIT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::ITEM });

	return entity;
}

Entity createCoffeeBean(RenderSystem* renderer, vec2 position, int id, std::string name, int amount)
{
	auto entity = Entity();

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::COFFEE_BEANS;
	item.name = name;
	item.isCollectable = true; // Make sure the item can be collected
	item.amount = amount;
	item.originalPosition = position;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ COFFEE_BEAN_WIDTH, COFFEE_BEAN_HEIGHT });

	Entity textbox = createTextbox(renderer, position, entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::COFFEE_BEAN,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::ITEM });

	return entity;
}

Entity createCauldron(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name)
{
	auto entity = Entity();

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::CAULDRON;
	item.name = name;
	item.isCollectable = false;
	item.amount = 0;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	Entity textbox = createTextbox(renderer, position, entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GROTTO_CAULDRON,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::TERRAIN,
		});

	return entity;
}

Entity createMortarPestle(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name)
{
	auto entity = Entity();

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::MORTAR_PESTLE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 0;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	// Entity textbox = createTextbox(renderer, position, entity); // not needed for Milestone 1

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GROTTO_MORTAR_PESTLE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::TERRAIN,
		});

	return entity;
}

Entity createChest(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name)
{
	auto entity = Entity();

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::CHEST;
	item.name = name;
	item.isCollectable = false;
	item.amount = 0;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	// Entity textbox = createTextbox(renderer, position, entity); // not needed for Milestone 1

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GROTTO_CHEST,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::TERRAIN,
		});

	return entity;
}

Entity createRecipeBook(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name)
{
	auto entity = Entity();

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::RECIPE_BOOK;
	item.name = name;
	item.isCollectable = false;
	item.amount = 0;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	// Entity textbox = createTextbox(renderer, position, entity); // not needed for Milestone 1

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::GROTTO_RECIPE_BOOK,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::TERRAIN,
		});

	return entity;
}


Entity createGrottoExit(RenderSystem* renderer, vec2 position, int id, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::FOREST;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::GROTTO_EXIT;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2(190, BOUNDARY_LINE_THICKNESS);

	Entity textbox = createTextbox(renderer, vec2({ position.x, position.y - 50 }), entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BOUNDARY_LINE, // because door is drawn into biome so just have the placement set for rendering
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 0 });

	return entity;
}

Entity createTextbox(RenderSystem* renderer, vec2 position, Entity itemEntity)
{
	auto entity = Entity();

	// Create a Textbox component
	Textbox& textbox = registry.textboxes.emplace(entity);
	textbox.targetItem = itemEntity;
	textbox.isVisible = false; // Initially hidden

	// Store mesh reference
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Motion component (position it above the item)
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position + vec2(-TEXTBOX_WIDTH / 2, 0);
	motion.scale = vec2(TEXTBOX_WIDTH, -TEXTBOX_HEIGHT);

	return entity;
}

RenderRequest getTextboxRenderRequest(Textbox& textbox)
{
	if (!registry.items.has(textbox.targetItem))
	{
		// std::cerr << "ERROR: Target item not found for textbox!" << std::endl;
		return {};
	}

	Item& item = registry.items.get(textbox.targetItem);

	// Find correct texture based on item type
	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::TEXTBOX_FRUIT; // placeholder
	if (item.name == "Magical Fruit")
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_FRUIT;
	}
	else if (item.name == "Coffee Bean")
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_COFFEE_BEAN;
	}
	else if (item.name == "Grotto Entrance")
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_ENTER_GROTTO;
	}
	else if (item.name == "Cauldron")
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_CAULDRON;
	}
	else if (item.name == "Grotto Exit")
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_GROTTO_EXIT;
	}

	return {
		texture,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::ITEM };
}
