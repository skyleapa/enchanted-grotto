#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include "systems/item_system.hpp"
#include <iostream>

Entity createWelcomeScreen(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " welcome screen" << std::endl;
	WelcomeScreen& screen = registry.welcomeScreens.emplace(entity);

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.angle = 180.f;
	motion.position = position;

	motion.scale = vec2({ WINDOW_WIDTH_PX - 230, WINDOW_HEIGHT_PX - 170 });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::WELCOME_TO_GROTTO,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::UI });

	return entity;
}

Entity createTree(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " tree" << std::endl;
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
	// std::cout << "Entity " << entity.id() << " gridline" << std::endl;

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
	// std::cout << "Entity " << entity.id() << " player" << std::endl;
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
	inventory.capacity = 8;
	inventory.isFull = false;

	std::vector<TEXTURE_ASSET_ID> walking_down = {
		TEXTURE_ASSET_ID::PLAYER_WALKING_S_1, TEXTURE_ASSET_ID::PLAYER_WALKING_S_2,
		TEXTURE_ASSET_ID::PLAYER_WALKING_S_3, TEXTURE_ASSET_ID::PLAYER_WALKING_S_4
	};

	// Default to idle animation
	auto& animation = registry.animations.emplace(entity);
	animation.frames = walking_down;
	animation.frame_time = 150.f;

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
	// std::cout << "Entity " << entity.id() << " forest bridge" << std::endl;
	auto& terrain = registry.terrains.emplace(entity);
	// we're using mesh collisions here, so AABB is not used, see components.cpp for more
	terrain.collision_setting = 2.0f;
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

Entity createForestBridgeTop(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " forest bridge top" << std::endl;
	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 3.0f;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BRIDGE_TOP);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	motion.scale = vec2({ FOREST_BRIDGE_WIDTH - 10, FOREST_BRIDGE_HEIGHT - 135 });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT, // texture count, doesn't use mesh
			EFFECT_ASSET_ID::CHICKEN,
			GEOMETRY_BUFFER_ID::BRIDGE_TOP,
			RENDER_LAYER::STRUCTURE,
			1
		});

	return entity;
}


Entity createForestBridgeBottom(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " forest bridge bottom" << std::endl;
	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 3.0f;

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BRIDGE_BOTTOM);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	motion.scale = vec2({ FOREST_BRIDGE_WIDTH - 14, FOREST_BRIDGE_HEIGHT - 135 });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT, // texture count, doesn't use mesh
			EFFECT_ASSET_ID::CHICKEN,
			GEOMETRY_BUFFER_ID::BRIDGE_BOTTOM,
			RENDER_LAYER::STRUCTURE,
			1
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
	// std::cout << "Entity " << entity1.id() << " river" << std::endl;
	// std::cout << "Entity " << entity2.id() << " river" << std::endl;

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
			TEXTURE_ASSET_ID::FOREST_RIVER_TOP,
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
			TEXTURE_ASSET_ID::FOREST_RIVER_BOTTOM,
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
	// std::cout << "Entity " << entity.id() << " static grotto entity" << std::endl;
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
	// std::cout << "Entity " << entity.id() << " boundary line" << std::endl;

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
	// std::cout << "Entity " << entity.id() << " grotto entrance" << std::endl;

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
	// std::cout << "Entity " << entity.id() << " bush" << std::endl;
	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.height_ratio = 0.35f;
	terrain.width_ratio = 0.55f;

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

Entity createCollectableIngredient(RenderSystem* renderer, vec2 position, ItemType type, int amount)
{
	assert(ITEM_INFO.count(type) && "Tried to create an item that has no info!");
	ItemInfo info = ITEM_INFO.at(type);
	auto entity = ItemSystem::createCollectableIngredient(position, type, amount);

	// Mesh
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = info.size;

	if (type == ItemType::SAP) {
		vec2 textbox_position = { position.x + 160.0f, position.y };
		Entity textbox = createTextbox(renderer, textbox_position, entity);
	}
	else {
		Entity textbox = createTextbox(renderer, position, entity);
	}

	registry.renderRequests.insert(
		entity,
		{ info.texture,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::ITEM });

	return entity;
}

Entity createCauldron(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name, bool create_textbox = false)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " cauldron" << std::endl;

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

	// Create cauldron
	auto& cauldron = registry.cauldrons.emplace(entity);
	if (create_textbox) createTextbox(renderer, position, entity);

	// Give cauldron an inventory
	auto& inv = registry.inventories.emplace(entity);
	inv.capacity = 0x7FFFFFFF;

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
	// std::cout << "Entity " << entity.id() << " mortar and pestle" << std::endl;

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
	// std::cout << "Entity " << entity.id() << " chest" << std::endl;

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
	// std::cout << "Entity " << entity.id() << " recipe book" << std::endl;

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
	// std::cout << "Entity " << entity.id() << " grotto exit" << std::endl;

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
	// std::cout << "Entity " << entity.id() << " textbox" << std::endl;

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

	// TODO: Use ItemType and a map
	// Find correct texture based on item type
	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::TEXTBOX_FRUIT; // placeholder
	if (item.type == ItemType::MAGICAL_FRUIT)
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_FRUIT;
	}
	else if (item.type == ItemType::COFFEE_BEANS)
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_COFFEE_BEAN;
	}
	else if (item.type == ItemType::GROTTO_ENTRANCE)
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_ENTER_GROTTO;
	}
	else if (item.type == ItemType::CAULDRON)
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_CAULDRON;
	}
	else if (item.type == ItemType::GROTTO_EXIT)
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_GROTTO_EXIT;
	}
	else if (item.type == ItemType::DESERT_ENTRANCE)
	{
		texture = TEXTURE_ASSET_ID::TEXTBOX_ENTER_DESERT;
	}
	else if (item.type == ItemType::FOREST_ENTRANCE) {
		texture = TEXTURE_ASSET_ID::TEXTBOX_ENTER_FOREST;
	}
	else if (item.type == ItemType::SAP) {
		texture = TEXTURE_ASSET_ID::TEXTBOX_SAP;
	}
	else if (item.type == ItemType::MAGICAL_DUST) {
		texture = TEXTURE_ASSET_ID::TEXTBOX_MAGICAL_DUST;
	}

	return {
		texture,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::ITEM };
}

Entity createEnt(RenderSystem* renderer, vec2 position, int movable) {
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " ent" << std::endl;

	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.attack_radius = 5;
	enemy.health = 75;
	enemy.start_pos = position;
	enemy.state = (int)ENEMY_STATE::IDLE;
	enemy.can_move = movable;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ ENT_WIDTH, ENT_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ENT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::TERRAIN,
		});

	return entity;
}

Entity createMummy(RenderSystem* renderer, vec2 position, int movable) {
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " mummy" << std::endl;

	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.attack_radius = 5;
	enemy.health = 100;
	enemy.start_pos = position;
	enemy.state = (int)ENEMY_STATE::IDLE;
	enemy.can_move = movable;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ MUMMY_WIDTH, MUMMY_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::MUMMY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::TERRAIN,
		});

	return entity;
}

bool createFiredAmmo(RenderSystem* renderer, vec2 target, Entity& item_entity, Entity& player_entity) {

	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " fired ammo" << std::endl;

	if (!registry.ammo.has(item_entity)) std::cout << "item isn't in ammo list" << std::endl;
	if (!registry.motions.has(player_entity) || !registry.ammo.has(item_entity)) return false;

	// Ammo& ammo = registry.ammo.get(item_entity);
	Ammo& ammo = registry.ammo.emplace(entity);
	Player& player = registry.players.get(player_entity);
	Motion& player_motion = registry.motions.get(player_entity);
	vec2 player_pos = player_motion.position;

	float delta_x = target.x - player_pos.x;
	float delta_y = target.y - player_pos.y;
	float distance = sqrt(delta_x * delta_x + delta_y * delta_y); // TODO: use glm::distance instead
	float unit_x = delta_x / distance;
	float unit_y = delta_y / distance;

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { unit_x, unit_y };
	motion.position = player_pos;
	motion.scale = vec2({ 50, 50 });

	ammo.is_fired = true;
	ammo.start_pos = player_motion.position;
	if (ammo.damage == 0) {
		ammo.damage = 25;
	}
	ammo.target = { player_pos.x + unit_x * player.throw_distance, player_pos.y + unit_y * player.throw_distance };

	registry.renderRequests.insert(
		entity,
		{
			ITEM_INFO.at(registry.items.get(item_entity).type).texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::ITEM,
		});

	return true;
}

Entity createDesertEntrance(RenderSystem* renderer, vec2 position, int id, std::string name)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " desert entrance" << std::endl;

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::DESERT;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::DESERT_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2(DESERT_FOREST_TRANSITION_WIDTH, DESERT_FOREST_TRANSITION_HEIGHT);

	Entity textbox = createTextbox(renderer, vec2({ position.x + GRID_CELL_WIDTH_PX * 4, position.y }), entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DESERT_TO_FOREST, // drawn into biome so just have the placement set for rendering
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 0 });

	return entity;
}

Entity createDesertExit(RenderSystem* renderer, vec2 position, int id, std::string name)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " desert exit" << std::endl;

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::FOREST;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::FOREST_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2(DESERT_FOREST_TRANSITION_WIDTH, DESERT_FOREST_TRANSITION_HEIGHT);

	Entity textbox = createTextbox(renderer, vec2({ position.x , position.y }), entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FOREST_TO_DESERT, // drawn into biome so just have the placement set for rendering
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 0 });

	return entity;
}

Entity createDesertTree(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " desert tree" << std::endl;
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

	motion.scale = vec2({ DESERT_TREE_WIDTH, DESERT_TREE_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DESERT_TREE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createDesertCactus(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " desert catcus" << std::endl;
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

	motion.scale = vec2({ DESERT_CACTUS_WIDTH, DESERT_CACTUS_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DESERT_CACTUS,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN ,
		 1 });

	return entity;
}

Entity createDesertRiver(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " desert river" << std::endl;
	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 1.0f; // rivers are not walkable

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;
	motion.scale = vec2({ DESERT_RIVER_WIDTH, DESERT_RIVER_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DESERT_RIVER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::STRUCTURE,
			1 // this is the render sub layer (river should be below bridge)
		});

	return entity;
}

Entity createDesertSandPile(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " desert sand pile" << std::endl;
	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ DESERT_FOREST_TRANSITION_WIDTH, DESERT_FOREST_TRANSITION_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DESERT_TO_FOREST,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN ,
		 0 });

	return entity;
}

Entity createDesertPage(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " desert page" << std::endl;
	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.height_ratio = 1.f;
	terrain.width_ratio = 0.2f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ DESERT_PAGE_WIDTH, DESERT_PAGE_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DESERT_SAND_PILE_PAGE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN ,
		 1 });

	return entity;
}

Entity createDesertSkull(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " desert skull" << std::endl;
	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.height_ratio = 0.3f;
	terrain.width_ratio = 0.8f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ DESERT_SKULL_WIDTH, DESERT_SKULL_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DESERT_SKULL,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN ,
		 1 });

	return entity;
}