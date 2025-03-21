#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include "systems/item_system.hpp"
#include "systems/ui_system.hpp"
#include <iostream>


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

Entity createBoundaryLine(RenderSystem* renderer, vec2 position, vec2 scale)
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

Entity createWelcomeScreen(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " welcome screen" << std::endl;
	registry.welcomeScreens.emplace(entity);

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

Entity createPlayer(RenderSystem* renderer, vec2 position)
{
	// create player in grotto

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

	motion.scale = { PLAYER_BB_WIDTH * PlAYER_BB_GROTTO_SIZE_FACTOR, PLAYER_BB_HEIGHT * PlAYER_BB_GROTTO_SIZE_FACTOR };

	auto& inventory = registry.inventories.emplace(entity);
	inventory.capacity = 10;
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

/* ==============================================================================================================
	Collectable items and interaction textbox
============================================================================================================== */

Entity createCollectableIngredient(RenderSystem* renderer, vec2 position, ItemType type, int amount, bool canRespawn)
{
	assert(ITEM_INFO.count(type) && "Tried to create an item that has no info!");
	ItemInfo info = ITEM_INFO.at(type);
	auto entity = ItemSystem::createCollectableIngredient(position, type, amount, canRespawn);

	// Mesh
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = info.size;

	// this dynamically gets the textbox name from ITEM_INFO defined in components.hpp
	std::string text = "[F] " + info.name;

	// assumption for textbox size (max-width is 150)
	const float TEXTBOX_WIDTH = 150.f;

	// default position (left of the entity)
	float textboxX = position.x - 140;
	float textboxY = position.y - 20;

	// adjust if going off-screen horizontally (don't need vertical adjustment for now)
	if (textboxX < 0) {
		textboxX = position.x + 40;
	}
	else if (textboxX + TEXTBOX_WIDTH > WINDOW_WIDTH_PX) {
		textboxX = position.x - TEXTBOX_WIDTH - 20;
	}

	Entity textbox = createTextbox(renderer, vec2(textboxX, textboxY), entity, text);

	registry.renderRequests.insert(
		entity,
		{ info.texture,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::ITEM });

	return entity;
}

Entity createTextbox(RenderSystem* renderer, vec2 position, Entity itemEntity, std::string text)
{
	auto entity = Entity();

	// Create a Textbox component
	Textbox& textbox = registry.textboxes.emplace(entity);
	textbox.targetItem = itemEntity;
	textbox.isVisible = false; // Initially hidden
	textbox.text = text;
	textbox.pos = position;

	// Store mesh reference
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// // Motion component (position it above the item)
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position + vec2(-TEXTBOX_WIDTH / 2, 0);
	motion.scale = vec2(TEXTBOX_WIDTH, -TEXTBOX_HEIGHT);

	return entity;
}

/* ==============================================================================================================
	Forest Creation
============================================================================================================== */

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

	// create coffee beans on bush
	createCollectableIngredient(renderer, vec2(position.x - 30, position.y - 12), ItemType::COFFEE_BEANS, 1, true);
	createCollectableIngredient(renderer, vec2(position.x + 38, position.y - 10), ItemType::COFFEE_BEANS, 1, true);
	createCollectableIngredient(renderer, vec2(position.x + 10, position.y + 25), ItemType::COFFEE_BEANS, 1, true);

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

	// create magical fruit that spawns on tree
	createCollectableIngredient(renderer, vec2(position.x, position.y - 30), ItemType::GALEFRUIT, 1, true);

	return entity;
}

Entity createForestBridge(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
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
	// top half of river texture
	auto entity1 = Entity();
	auto& terrain1 = registry.terrains.emplace(entity1);
	terrain1.collision_setting = 1.0f;

	// bottom half of river texture
	auto entity2 = Entity();
	auto& terrain2 = registry.terrains.emplace(entity2);
	terrain2.collision_setting = 1.0f;

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

/* ==============================================================================================================
	Grotto Creation
============================================================================================================== */

Entity createGrottoStaticEntities(RenderSystem* renderer, vec2 position, vec2 scale, float angle, GLuint texture_asset_id, float can_collide)
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

Entity createGrottoPoolMesh(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 3.0f; // using mesh for collision

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::GROTTO_POOL);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	motion.scale = vec2({ 510, 195 });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT, // texture count, doesn't use mesh
			EFFECT_ASSET_ID::CHICKEN,
			GEOMETRY_BUFFER_ID::GROTTO_POOL,
			RENDER_LAYER::STRUCTURE,
			1
		});

	return entity;
}

Entity createCauldron(RenderSystem* renderer, vec2 position, vec2 scale, std::string name, bool create_textbox = false)
{
	// Create simple cauldron water entity
	auto waterEntity = Entity();
	auto& waterMotion = registry.motions.emplace(waterEntity);
	waterMotion.angle = 180.f;
	waterMotion.velocity = { 0, 0 };
	waterMotion.position = {
		CAULDRON_WATER_POS.x * WINDOW_WIDTH_PX - 2,
		WINDOW_HEIGHT_PX - CAULDRON_WATER_POS.y * WINDOW_HEIGHT_PX - 2
	};
	waterMotion.scale = { CAULDRON_D + 10, CAULDRON_D + 10 }; // looks slightly better
	registry.renderRequests.insert(
		waterEntity,
		{
			TEXTURE_ASSET_ID::CAULDRON_WATER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::UI,
			0, false
		});

	// The actual cauldron entity
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
	cauldron.water = waterEntity;
	if (create_textbox) createTextbox(renderer, position, entity, "[F] Use Cauldron");

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

Entity createMortarPestle(RenderSystem* renderer, vec2 position, vec2 scale, std::string name)
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
	
	// Create mortar pestle
	registry.mortarAndPestles.emplace(entity);
	createTextbox(renderer, { GRID_CELL_WIDTH_PX * 6.5, GRID_CELL_HEIGHT_PX * 3 }, entity, "[F] Use Mortar & Pestle");

	// Give mortar an inventory
	auto& inv = registry.inventories.emplace(entity);
	inv.capacity = 1;

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

Entity createChest(RenderSystem* renderer, vec2 position, vec2 scale, std::string name)
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

	// Entity textbox = createTextbox(renderer, position, entity, "[F] Open Chest"); // not needed for Milestone 1

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

Entity createRecipeBook(RenderSystem* renderer, vec2 position, vec2 scale, std::string name)
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

	// Entity textbox = createTextbox(renderer, position, entity, "[F] Recipe Book"); // not needed for Milestone 1

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

/* ==============================================================================================================
	Desert Creation
============================================================================================================== */

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

	createCollectableIngredient(renderer, {position.x, position.y}, ItemType::CACTUS_PULP, 1, true);
	createCollectableIngredient(renderer, {position.x + 40.0f, position.y - 30.0f}, ItemType::CACTUS_PULP, 1, true);

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

	createCollectableIngredient(renderer, { position.x - 100.0f, position.y + 10.0f}, ItemType::PETRIFIED_BONE, 2, true);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DESERT_SKULL,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN ,
		 1 });

	return entity;
}

/* ==============================================================================================================
	Mushroom Biome Creation
============================================================================================================== */

Entity createMushroomAcidLake(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 2.0f; // 2 as we're using a mesh for collisions

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ MUSHROOM_ACID_LAKE_WIDTH, MUSHROOM_ACID_LAKE_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MUSHROOM_ACID_LAKE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		});

	return entity;
}

Entity createMushroomAcidLakeMesh(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();
	auto& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 3.0f; // using mesh for collision

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::MUSHROOM_ACID_LAKE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	motion.scale = vec2({ MUSHROOM_ACID_LAKE_WIDTH - 20, MUSHROOM_ACID_LAKE_HEIGHT - 20 });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT, // texture count, doesn't use mesh
			EFFECT_ASSET_ID::CHICKEN,
			GEOMETRY_BUFFER_ID::MUSHROOM_ACID_LAKE,
			RENDER_LAYER::STRUCTURE,
			1
		});

	return entity;
}

Entity createMushroomBlue(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.2f;
	terrain.height_ratio = 0.1f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ MUSHROOM_WIDTH, MUSHROOM_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MUSHROOM_BLUE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createMushroomPink(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.2f;
	terrain.height_ratio = 0.1f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ MUSHROOM_WIDTH, MUSHROOM_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MUSHROOM_PINK,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createMushroomPurple(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.2f;
	terrain.height_ratio = 0.1f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ MUSHROOM_WIDTH, MUSHROOM_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MUSHROOM_PURPLE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createMushroomTallBlue(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.1f;
	terrain.height_ratio = 0.1f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ MUSHROOM_TALL_WIDTH, MUSHROOM_TALL_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MUSHROOM_TALL_BLUE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createMushRoomTallPink(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.1f;
	terrain.height_ratio = 0.1f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ MUSHROOM_TALL_WIDTH, MUSHROOM_TALL_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::MUSHROOM_TALL_PINK,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

/* ==============================================================================================================
	Crystal Biome Creation
============================================================================================================== */

Entity createCrystal1(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.4f;
	terrain.height_ratio = 0.2f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ CRYSTAL_1_WIDTH, CRYSTAL_1_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CRYSTAL_1,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createCrystal2(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.65f;
	terrain.height_ratio = 0.2f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ CRYSTAL_2_WIDTH, CRYSTAL_2_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CRYSTAL_2,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createCrystal3(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.4f;
	terrain.height_ratio = 0.2f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ CRYSTAL_3_WIDTH, CRYSTAL_3_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CRYSTAL_3,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createCrystal4(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.4f;
	terrain.height_ratio = 0.2f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ CRYSTAL_4_WIDTH, CRYSTAL_4_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CRYSTAL_4,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createCrystalMinecart(RenderSystem* renderer, vec2 position) {

	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.9f;
	terrain.height_ratio = 0.5f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = vec2({ CRYSTAL_MINECART_WIDTH, CRYSTAL_MINECART_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CRYSTAL_MINECART,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

Entity createCrystalPage(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 2.0f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ CRYSTAL_PAGE_WIDTH, CRYSTAL_PAGE_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CRYSTAL_PAGE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::ITEM,
		 0 });

	return entity;
}

Entity createCrystalRock(RenderSystem* renderer, vec2 position) {
	auto entity = Entity();

	Terrain& terrain = registry.terrains.emplace(entity);
	terrain.collision_setting = 0.0f;
	terrain.width_ratio = 0.8f;
	terrain.height_ratio = 0.2f;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 180.f;
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ CRYSTAL_ROCK_WIDTH, CRYSTAL_ROCK_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CRYSTAL_ROCK,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::TERRAIN });

	return entity;
}

/* ==============================================================================================================
	Entering and transition between biomes
============================================================================================================== */

Entity createForestToGrotto(RenderSystem* renderer, vec2 position, std::string name)
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

	Entity textbox = createTextbox(renderer, vec2({ position.x + 40, position.y + 30 }), entity, "[F] Enter Grotto");

	// m_ui_system->createRmlUITextbox(1, "[F] Enter Grotto", vec2(position.x, position.y + 20));

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GROTTO_ENTRANCE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 0 });

	return entity;
}

Entity createGrottoToForest(RenderSystem* renderer, vec2 position, std::string name)
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

	Entity textbox = createTextbox(renderer, vec2({ position.x + 60, position.y - 40 }), entity, "[F] Exit Grotto");

	// registry.renderRequests.insert(
	// 	entity,
	// 	{ TEXTURE_ASSET_ID::BOUNDARY_LINE, // because door is drawn into biome so just have the placement set for rendering
	// 	 EFFECT_ASSET_ID::TEXTURED,
	// 	 GEOMETRY_BUFFER_ID::SPRITE,
	// 	 RENDER_LAYER::STRUCTURE,
	// 	 0 });

	return entity;
}

Entity createForestToDesert(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::DESERT;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::FOREST_TO_DESERT_ENTRANCE;
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

	Entity textbox = createTextbox(renderer, vec2({ position.x + 60, position.y - 20 }), entity, "[F] Enter Desert");

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DESERT_TO_FOREST, // drawn into biome so just have the placement set for rendering
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 0 });

	return entity;
}

Entity createDesertToForest(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::FOREST;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::DESERT_TO_FOREST_ENTRANCE;
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

	Entity textbox = createTextbox(renderer, vec2({ position.x + 40, position.y - 10 }), entity, "[F] Enter Forest");

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FOREST_TO_DESERT, // drawn into biome so just have the placement set for rendering
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 0 });

	return entity;
}

Entity createForestToForestEx(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::FOREST_EX;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::FOREST_TO_FOREST_EX_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ GENERIC_ENTRANCE_WIDTH, GENERIC_ENTRANCE_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x - 210, position.y - 80 }), entity, "[F] Enter Deep Forest");

	return entity;
}

Entity createForestExToForest(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::FOREST;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::FOREST_EX_TO_FOREST_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ GENERIC_ENTRANCE_WIDTH, GENERIC_ENTRANCE_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x, position.y - 100 }), entity, "[F] Enter Forest");

	return entity;
}

Entity createForestToMushroom(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::MUSHROOM;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::FOREST_TO_MUSHROOM_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.angle = 180.f;
	motion.position = position;

	motion.scale = vec2({ FOREST_TO_MUSHROOM_WIDTH, FOREST_TO_MUSHROOM_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x + 70, position.y - 20 }), entity, "[F] Enter Shroomlands");

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FOREST_TO_MUSHROOM,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		 RENDER_LAYER::STRUCTURE,
		 0 });

	return entity;
}

Entity createMushroomToForest(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::FOREST;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::MUSHROOM_TO_FOREST_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ GENERIC_ENTRANCE_WIDTH, GENERIC_ENTRANCE_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x + 100, position.y - 20 }), entity, "[F] Enter Forest");

	return entity;
}

Entity createMushroomToCrystal(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::CRYSTAL;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::MUSHROOM_TO_CRYSTAL_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ GENERIC_ENTRANCE_WIDTH, GENERIC_ENTRANCE_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x - 180, position.y - 100 }), entity, "[F] Enter Crystal Caves");

	return entity;
}

Entity createCrystalToMushroom(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::MUSHROOM;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::CRYSTAL_TO_MUSHROOM_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ GENERIC_ENTRANCE_WIDTH, GENERIC_ENTRANCE_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x - 10, position.y - 100 }), entity, "[F] Enter Shroomlands");

	return entity;
}

Entity createCrystalToForestEx(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::FOREST_EX;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::CRYSTAL_TO_FOREST_EX_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ GENERIC_ENTRANCE_WIDTH, GENERIC_ENTRANCE_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x + 100, position.y }), entity, "[F] Enter Deep Forest");

	return entity;
}

Entity createForestExToCrystal(RenderSystem* renderer, vec2 position, std::string name)
{
	auto entity = Entity();

	Entrance& entrance = registry.entrances.emplace(entity);
	entrance.target_biome = (GLuint)BIOME::CRYSTAL;

	Item& item = registry.items.emplace(entity);
	item.type = ItemType::FOREST_EX_TO_CRYSTAL_ENTRANCE;
	item.name = name;
	item.isCollectable = false;
	item.amount = 1;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;

	motion.scale = vec2({ GENERIC_ENTRANCE_WIDTH, GENERIC_ENTRANCE_HEIGHT });

	Entity textbox = createTextbox(renderer, vec2({ position.x + 20, position.y - 40 }), entity, "[F] Enter Crystal Caves");

	return entity;
}

/* ==============================================================================================================
	Combat Creation
============================================================================================================== */

Entity createEnt(RenderSystem* renderer, vec2 position, int movable, std::string name) {

	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " ent" << std::endl;

	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.attack_radius = 5;
	enemy.health = 75;
	enemy.start_pos = position;
	enemy.state = (int)ENEMY_STATE::IDLE;
	enemy.can_move = movable;
	enemy.name = name;
	enemy.attack_damage = 1;

	// auto& terrain = registry.terrains.emplace(entity);
	// terrain.collision_setting = 1.0f; // cannot walk past guardian

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

Entity createMummy(RenderSystem* renderer, vec2 position, int movable, std::string name) {
	auto entity = Entity();
	// std::cout << "Entity " << entity.id() << " mummy" << std::endl;

	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.attack_radius = 5;
	enemy.health = 100;
	enemy.start_pos = position;
	enemy.state = (int)ENEMY_STATE::IDLE;
	enemy.can_move = movable;
	enemy.name = name;
	enemy.attack_damage = 30;

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

	if (!registry.ammo.has(item_entity)) std::cout << "Cannot throw this item" << std::endl;
	if (!registry.motions.has(player_entity) || !registry.ammo.has(item_entity)) return false;

	Ammo& ammo = registry.ammo.emplace(entity);

	// If it's a potion add colour to it
	if (registry.potions.has(item_entity)) {
		registry.colors.insert(entity, registry.potions.get(item_entity).color / 255.f);
	}

	Player& player = registry.players.get(player_entity);
	Motion& player_motion = registry.motions.get(player_entity);
	vec2 player_pos = player_motion.position;

	float delta_x = target.x - player_pos.x;
	float delta_y = target.y - player_pos.y;
	float angle = atan2f(delta_y, delta_x);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { cosf(angle), sinf(angle) };
	motion.position = player_pos;
	motion.scale = vec2({ 50, 50 });

	ammo.is_fired = true;
	ammo.start_pos = player_motion.position;
	if (ammo.damage == 0) {
		ammo.damage = 25;
	}
	ammo.target = { player_pos.x + THROW_DISTANCE * cosf(angle), player_pos.y + THROW_DISTANCE * sinf(angle) };

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