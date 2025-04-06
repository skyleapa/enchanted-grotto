#include "common.hpp"
#include "biome_system.hpp"
#include "physics_system.hpp"
#include "item_system.hpp"
#include "world_init.hpp"
#include "sound_system.hpp"
#include <iostream>

#include <vector>

void BiomeSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	ScreenState& screen = registry.screenStates.components[0];
	screen.darken_screen_factor = 1;
	screen.fade_status = 1; // start from black screen
	screen.is_switching_biome = true;
}

// step function to handle biome changes
void BiomeSystem::step(float elapsed_ms_since_last_update) {

	// handle switching biomes
	ScreenState& screen = registry.screenStates.components[0];
	if (registry.players.entities.size() < 1)
		return;
	Entity& player = registry.players.entities[0];
	if (!registry.motions.has(player))
		return;

	if (screen.is_switching_biome)
	{
		if (screen.fade_status == 0)
		{
			screen.darken_screen_factor += elapsed_ms_since_last_update * TIME_UPDATE_FACTOR;
			if (screen.darken_screen_factor >= 1)
				screen.fade_status = 1; // after fade out

			// stop ammo motion, enemy motion is handled in ai_system
			for (Entity ammo : registry.ammo.entities) {
				if (registry.motions.has(ammo)) {
					registry.motions.get(ammo).velocity = { 0,0 };
				}
			}

		}
		else if (screen.fade_status == 1)
		{
			if (screen.biome != screen.switching_to_biome)
			{
				screen.from_biome = screen.biome;
				screen.biome = screen.switching_to_biome;
				screen.darken_screen_factor = 1;
				switchBiome((int)screen.biome, screen.first_game_load);
			}
			screen.darken_screen_factor -= elapsed_ms_since_last_update * TIME_UPDATE_FACTOR;
			if (screen.darken_screen_factor <= 0)
				screen.fade_status = 2; // after fade in
		}
		else
		{
			// complete biome switch
			screen.darken_screen_factor = 0;
			screen.is_switching_biome = false;
			screen.fade_status = 0;
		}
	}
	return;
}

void BiomeSystem::switchBiome(int biome, bool is_first_load) {
	std::vector<Entity> to_remove;
	for (auto entity : registry.motions.entities) {
		if (registry.players.has(entity) || registry.inventories.has(entity) || registry.potions.has(entity)) continue; // don't lose potion effects

		// don't delete any render requests marked as invisible
		if (registry.renderRequests.has(entity)) {
			if (!registry.renderRequests.get(entity).is_visible) continue;
		}

		to_remove.push_back(entity);
	}

	for (auto entity : to_remove) {
		registry.remove_all_components_of(entity);
	}

	// halt any boiling sounds first
	SoundSystem::haltBoilSound();

	if (biome == (GLuint)BIOME::FOREST) {
		createForest();
	}
	else if (biome == (GLuint)BIOME::FOREST_EX) {
		createForestEx();
	}
	else if (biome == (GLuint)BIOME::GROTTO) {
		createGrotto();
		// continue boiling if it was boiling before leaving grotto, assumes we only have 1 cauldron enttity
		if (registry.cauldrons.entities.size() > 0) {
			if (registry.cauldrons.components[0].is_boiling) SoundSystem::playBoilSound((int)SOUND_CHANNEL::BOILING, -1);
		}
	}
	else if (biome == (GLuint)BIOME::DESERT) {
		createDesert();
	}
	else if (biome == (GLuint)BIOME::MUSHROOM) {
		createMushroom();
	}
	else if (biome == (GLuint)BIOME::CRYSTAL) {
		createCrystal();
	}

	renderPlayerInNewBiome(is_first_load);
}

void BiomeSystem::renderPlayerInNewBiome(bool is_first_load) {
	if (registry.players.entities.size() == 0) return;

	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity)) return;

	ScreenState& screen = registry.screenStates.components[0];
	Motion& player_motion = registry.motions.get(player_entity);

	player_motion.scale = { PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT };

	if (screen.switching_to_biome == (int)BIOME::GROTTO && screen.biome == (int)BIOME::GROTTO) { // through grotto entrance from forest
		player_motion.scale = { PLAYER_BB_WIDTH * PlAYER_BB_GROTTO_SIZE_FACTOR, PLAYER_BB_HEIGHT * PlAYER_BB_GROTTO_SIZE_FACTOR };
		player_motion.position = vec2({ GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 11 }); // bring player to front of door
		if (screen.tutorial_state == (int)TUTORIAL::ENTER_GROTTO) {
			screen.tutorial_step_complete = true;
			screen.tutorial_state += 1;
		}

		// render all cauldrons and make visible again if they were previously existing
		for (Entity cauldron : registry.cauldrons.entities) {
			if (registry.renderRequests.has(cauldron)) {
				RenderRequest& rr = registry.renderRequests.get(cauldron);
				rr.is_visible = true;
				Terrain& terrain = registry.terrains.get(cauldron);
				terrain.collision_setting = 0.0f; // restore the collision setting when inside of grotto
				terrain.width_ratio = 0.80f;
				terrain.height_ratio = 0.40f;
			}
			// recreate textbox
			if (registry.motions.has(cauldron)) {
				Motion& motion = registry.motions.get(cauldron);
				createTextbox(renderer, vec2(motion.position.x + 70, motion.position.y - 80), cauldron, "[F] Use Cauldron");
			}

		}

		for (Entity mortar : registry.mortarAndPestles.entities) {
			if (registry.renderRequests.has(mortar)) {
				RenderRequest& rr = registry.renderRequests.get(mortar);
				rr.is_visible = true;
			}
		}
	}
	else if (screen.from_biome == (int)BIOME::GROTTO && screen.biome == (int)BIOME::FOREST) { // through grotto exit into forest
		player_motion.position = vec2(GROTTO_ENTRANCE_X, GROTTO_ENTRANCE_Y + 50);

		if (screen.tutorial_state == (int)TUTORIAL::EXIT_GROTTO) {
			screen.tutorial_step_complete = true;
			screen.tutorial_state += 1;
		}

		// ensure to make cauldron invisble since it should still exist but only rendered when in grotto
		for (Entity cauldron : registry.cauldrons.entities) {
			if (registry.renderRequests.has(cauldron)) {
				RenderRequest& rr = registry.renderRequests.get(cauldron);
				rr.is_visible = false;
				Terrain& terrain = registry.terrains.get(cauldron);
				terrain.collision_setting = 2.0f; // make sure our cauldron has no collision outside of grotto
			}
		}
		for (Entity mortar : registry.mortarAndPestles.entities) {
			if (registry.renderRequests.has(mortar)) {
				RenderRequest& rr = registry.renderRequests.get(mortar);
				rr.is_visible = false;
			}
		}
	}
	// between forest and forest expansion
	else if (screen.from_biome == (int)BIOME::FOREST && screen.biome == (int)BIOME::FOREST_EX) {
		player_motion.position = vec2(60, 450);
	}
	else if (screen.from_biome == (int)BIOME::FOREST_EX && screen.biome == (int)BIOME::FOREST) {
		player_motion.position = vec2(1150, 430);
	}
	// between forest and desert
	else if (screen.from_biome == (int)BIOME::FOREST && screen.biome == (int)BIOME::DESERT) {
		player_motion.position = vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 12);
	}
	else if (screen.from_biome == (int)BIOME::DESERT && screen.biome == (int)BIOME::FOREST) {
		player_motion.position = vec2(GRID_CELL_WIDTH_PX * 2, GRID_CELL_HEIGHT_PX * 2);
	}
	// between forest and mushroom
	else if (screen.from_biome == (int)BIOME::FOREST && screen.biome == (int)BIOME::MUSHROOM) {
		player_motion.position = vec2(100, 70);
	}
	else if (screen.from_biome == (int)BIOME::MUSHROOM && screen.biome == (int)BIOME::FOREST) {
		player_motion.position = vec2(100, 620);
	}
	// between mushroom and crystal
	else if (screen.from_biome == (int)BIOME::MUSHROOM && screen.biome == (int)BIOME::CRYSTAL) {
		player_motion.position = vec2(70, 200);
	}
	else if (screen.from_biome == (int)BIOME::CRYSTAL && screen.biome == (int)BIOME::MUSHROOM) {
		player_motion.position = vec2(1130, 200);
	}
	// between crystal and forest ex
	else if (screen.from_biome == (int)BIOME::CRYSTAL && screen.biome == (int)BIOME::FOREST_EX) {
		player_motion.position = vec2(900, 610);
	}
	else if (screen.from_biome == (int)BIOME::FOREST_EX && screen.biome == (int)BIOME::CRYSTAL) {
		player_motion.position = vec2(960, 90);
	}

	// if re-opening the game, set player position to where they were 
	if (is_first_load && registry.players.get(player_entity).load_position != vec2(0, 0)) {
		screen.first_game_load = false;
		player_motion.position = registry.players.get(player_entity).load_position;
	}
}

void BiomeSystem::createGrotto()
{
	// create tutorial screen
	if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::WELCOME_SCREEN) {
		createWelcomeScreen(renderer, vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2 - 50));
	}

	// positions are according to sample grotto interior
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::GROTTO))
	{
		createBoundaryLine(renderer, position, scale);
	}

	for (const auto& [position, size, rotation, texture, layer] : grotto_static_entity_pos) {
		createGrottoStaticEntities(renderer, position, size, rotation, texture, layer);
	}

	createGrottoPoolMesh(renderer, vec2(GRID_CELL_WIDTH_PX * 4.8, GRID_CELL_HEIGHT_PX * 11));

	if (registry.cauldrons.entities.size() == 0) {
		std::cout << "creating cauldron in grotto" << std::endl;
		Entity new_cauldron = createCauldron(renderer, vec2({ GRID_CELL_WIDTH_PX * 13.45, GRID_CELL_HEIGHT_PX * 6.05 }), vec2({ 140, 210 }), "Cauldron", false);
		for (Entity cauldron : registry.cauldrons.entities) {
			if (new_cauldron != cauldron) registry.remove_all_components_of(cauldron);
		}
		// initialize cauldron menu
		if (m_ui_system) {
			m_ui_system->setOpenedCauldron(new_cauldron);
			m_ui_system->openCauldron(new_cauldron, false);
			m_ui_system->closeCauldron(false);
		}
	}
	// assert(registry.cauldrons.entities.size() == 1); // We should always only have one cauldron for testing purposes

	if (registry.mortarAndPestles.entities.size() == 0) {
		Entity new_mortar = createMortarPestle(renderer, vec2({ GRID_CELL_WIDTH_PX * 7.5, GRID_CELL_HEIGHT_PX * 5.22 }), vec2({ 213, 141 }), "Mortar and Pestle");
		for (Entity mortar : registry.mortarAndPestles.entities) {
			if (new_mortar != mortar) registry.remove_all_components_of(mortar);
		}
		// initialize mortar and pestle menu
		if (m_ui_system) {
			m_ui_system->setOpenedMortarPestle(new_mortar);
			m_ui_system->openMortarPestle(new_mortar, false);
			m_ui_system->closeMortarPestle(false);
		}
	}

	// createMortarPestle(renderer, vec2({ GRID_CELL_WIDTH_PX * 7.5, GRID_CELL_HEIGHT_PX * 5.22 }), vec2({ 213, 141 }), "Mortar and Pestle");
	createRecipeBook(renderer, vec2({ GRID_CELL_WIDTH_PX * 4.15, GRID_CELL_HEIGHT_PX * 5.05 }), vec2({ 108, 160 }), "Recipe Book");
	createChest(renderer, vec2({ GRID_CELL_WIDTH_PX * 1.35, GRID_CELL_HEIGHT_PX * 5.2 }), vec2({ 100, 150 }), "Chest");
	createGrottoToForest(renderer, vec2(GRID_CELL_WIDTH_PX * 20.5, GRID_CELL_HEIGHT_PX * 13), "Grotto Exit");
}

bool BiomeSystem::handleEntranceInteraction(Entity entrance_entity)
{
	Entrance& entrance = registry.entrances.get(entrance_entity);
	ScreenState& state = registry.screenStates.components[0];
	state.darken_screen_factor = 0; // reset screen factor to 0
	if (entrance.target_biome == (GLuint)BIOME::GROTTO)
	{
		state.is_switching_biome = true;
		state.switching_to_biome = (GLuint)BIOME::GROTTO;
	}
	else if (entrance.target_biome == (GLuint)BIOME::FOREST) {
		state.is_switching_biome = true;
		state.switching_to_biome = (GLuint)BIOME::FOREST;
	}
	else if (entrance.target_biome == (GLuint)BIOME::FOREST_EX) {
		state.is_switching_biome = true;
		state.switching_to_biome = (GLuint)BIOME::FOREST_EX;
	}
	else if (entrance.target_biome == (GLuint)BIOME::DESERT) {
		state.is_switching_biome = true;
		state.switching_to_biome = (GLuint)BIOME::DESERT;
	}
	else if (entrance.target_biome == (GLuint)BIOME::MUSHROOM) {
		state.is_switching_biome = true;
		state.switching_to_biome = (GLuint)BIOME::MUSHROOM;
	}
	else if (entrance.target_biome == (GLuint)BIOME::CRYSTAL) {
		state.is_switching_biome = true;
		state.switching_to_biome = (GLuint)BIOME::CRYSTAL;
	}
	return true;
}

void BiomeSystem::createForest()
{
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::FOREST))
	{
		createBoundaryLine(renderer, position, scale);
	}

	createForestBridge(renderer, vec2(307, 485));
	createForestBridgeTop(renderer, vec2(307, 425));
	createForestBridgeBottom(renderer, vec2(309, 545));

	// NOTE: leaving this in for debugging vertices of meshes for the future
	/*
		Entity terrain_entity = createMushroomAcidLakeMesh(renderer, vec2(670, 117));
		Mesh* mesh = registry.meshPtrs.get(terrain_entity);
		Motion motion = registry.motions.get(terrain_entity);
		std::vector<vec2> transformed_vertices = PhysicsSystem::get_transformed_vertices(*mesh, motion);

		for (vec2 vertex : transformed_vertices) {
			createCollectableIngredient(renderer, vertex, ItemType::COFFEE_BEANS, 1, true);
		}
	*/

	createForestRiver(renderer, vec2(307, 0));

	createTree(renderer, vec2(530, 330));
	createTree(renderer, vec2(703, 165));

	createTreeNoFruit(renderer, vec2(714, 465));
	createTree(renderer, vec2(857, 540));
	createTreeNoFruit(renderer, vec2(1080, 500));

	createBush(renderer, vec2(1078, 620));

	createCollectableIngredient(renderer, vec2(1085, 282), ItemType::STORM_BARK, 1, true);
	createCollectableIngredient(renderer, vec2(560, 160), ItemType::STORM_BARK, 1, true);

	// admin flag used so we can test the game and disable guardian spawns
	if (!ADMIN_FLAG) {
		ScreenState screen = registry.screenStates.components[0];
		if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "desert") == screen.unlocked_biomes.end())
		{
			Entity desertGuardian = createGuardianDesert(renderer, vec2(GRID_CELL_WIDTH_PX * 2, GRID_CELL_HEIGHT_PX * 2.5), 0, "Desert Guardian");
		}

		if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "mushroom") == screen.unlocked_biomes.end())
		{
			Entity mushroomGuardian = createGuardianMushroom(renderer, vec2(GRID_CELL_WIDTH_PX * 2.1, WINDOW_HEIGHT_PX - 80), 0, "Mushroom Guardian");
		}
		else {
			createForestToMushroom(renderer, vec2(GRID_CELL_WIDTH_PX * 2.1, WINDOW_HEIGHT_PX - 40), "Mushroom Entrance");
		}
	}

	createForestToGrotto(renderer, vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 1), "Grotto Entrance");
	createForestToForestEx(renderer, vec2(WINDOW_WIDTH_PX, 470), "Forest Ex Entrance");
	createForestToDesert(renderer, vec2(GRID_CELL_WIDTH_PX * 2.1, GRID_CELL_HEIGHT_PX * 1.2), "Desert Entrance");
}

void BiomeSystem::createForestEx()
{
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::FOREST_EX))
	{
		createBoundaryLine(renderer, position, scale);
	}

	createTreeNoFruit(renderer, vec2(130, 130));
	createTree(renderer, vec2(157, 540));
	createTreeNoFruit(renderer, vec2(216, 240));
	createTree(renderer, vec2(403, 180));
	createTree(renderer, vec2(504, 535));
	createTreeNoFruit(renderer, vec2(857, 140));
	createTreeNoFruit(renderer, vec2(1120, 280));
	createTreeNoFruit(renderer, vec2(1080, 535));

	createBush(renderer, vec2(225, 600));

	createCollectableIngredient(renderer, vec2(288, 101), ItemType::EVERFERN, 1, true);
	createCollectableIngredient(renderer, vec2(708, 580), ItemType::EVERFERN, 1, true);
	createCollectableIngredient(renderer, vec2(1153, 109), ItemType::BLIGHTLEAF, 1, true);
	createCollectableIngredient(renderer, vec2(72, 619), ItemType::BLIGHTLEAF, 1, true);
	createEnt(renderer, vec2(606, 390), 1, "Ent");
	createEnt(renderer, vec2(1011, 158), 1, "Ent");

	createMasterPotionPedestal(renderer, vec2(638, 150));

	if (!ADMIN_FLAG) {
		ScreenState screen = registry.screenStates.components[0];
		if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "crystal") == screen.unlocked_biomes.end())
		{
			createGuardianCrystal(renderer, vec2(900, 620), 0, "Crystal Guardian");
		}
		else {
			createForestExToCrystal(renderer, vec2(930, 665), "Forest Ex to Crystal");
		}

		// render potion of rejuvenation on pedestal if we've saved the grotto
		if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "saved-grotto") != screen.unlocked_biomes.end())
		{
			createRejuvenationPotion(renderer);
		}
	}

	createForestExToForest(renderer, vec2(50, 470), "Forest Ex to Forest");
	if (ADMIN_FLAG) createForestExToCrystal(renderer, vec2(930, 665), "Forest Ex to Crystal");
}

void BiomeSystem::createDesert()
{
	// positions are according to sample desert
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::DESERT))
	{
		createBoundaryLine(renderer, position, scale);
	}

	createDesertToForest(renderer, vec2(GRID_CELL_WIDTH_PX * 20.3, GRID_CELL_HEIGHT_PX * 12.9), "Desert Exit");
	createDesertTree(renderer, vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 3.9));
	createDesertCactus(renderer, vec2(GRID_CELL_WIDTH_PX * 4.1, GRID_CELL_HEIGHT_PX * 6.2));
	createDesertRiver(renderer, vec2(1190, WINDOW_HEIGHT_PX / 2));
	createDesertPage(renderer, vec2(GRID_CELL_WIDTH_PX * 13.5, GRID_CELL_HEIGHT_PX * 3.2));
	createDesertSkull(renderer, vec2(GRID_CELL_WIDTH_PX * 13.7, GRID_CELL_HEIGHT_PX * 10.9));
	createCollectableIngredient(renderer, vec2(1096, 373), ItemType::HEALING_LILY, 1, true);
	createCollectableIngredient(renderer, vec2(400, 194), ItemType::HEALING_LILY, 1, true);

	ScreenState screen = registry.screenStates.components[0];
	if (std::find(screen.killed_enemies.begin(), screen.killed_enemies.end(), "Mummy 1") == screen.killed_enemies.end())
	{
		createMummy(renderer, vec2(GRID_CELL_WIDTH_PX * 15, GRID_CELL_HEIGHT_PX * 5), 1, "Mummy 1");
	}
	if (std::find(screen.killed_enemies.begin(), screen.killed_enemies.end(), "Mummy 2") == screen.killed_enemies.end()) {
		createMummy(renderer, vec2(GRID_CELL_WIDTH_PX * 4, GRID_CELL_HEIGHT_PX * 8), 1, "Mummy 2");
	}
}

void BiomeSystem::createMushroom()
{
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::MUSHROOM))
	{
		createBoundaryLine(renderer, position, scale);
	}

	createMushroomAcidLake(renderer, vec2(670, 117));
	createMushroomAcidLakeMesh(renderer, vec2(670, 117));

	createMushRoomTallPink(renderer, vec2(320, 160));
	createMushroomBlue(renderer, vec2(170, 440));
	createMushroomPurple(renderer, vec2(380, 485));
	createMushroomPink(renderer, vec2(560, 440));
	createMushroomBlue(renderer, vec2(750, 515));
	createMushroomTallBlue(renderer, vec2(1055, 435));

	createCollectableIngredient(renderer, vec2(260, 584), ItemType::GLOWSHROOM, 1, true);
	createCollectableIngredient(renderer, vec2(904, 454), ItemType::GLOWSHROOM, 1, true);
	createCollectableIngredient(renderer, vec2(1090, 114), ItemType::DOOMCAP, 1, true);

	if (!ADMIN_FLAG) {
		ScreenState screen = registry.screenStates.components[0];
		if (std::find(screen.unlocked_biomes.begin(), screen.unlocked_biomes.end(), "crystal") == screen.unlocked_biomes.end())
		{
			Entity crystalGuardian = createGuardianCrystal(renderer, vec2(1150, 200), 0, "Crystal Guardian");
		}
		else {
			createMushroomToCrystal(renderer, vec2(1220, 160), "Mushroom to Crystal");
		}
	}
	createMushroomToForest(renderer, vec2(60, 50), "Mushroom To Forest");
	if (ADMIN_FLAG) createMushroomToCrystal(renderer, vec2(1220, 160), "Mushroom to Crystal");
}

void BiomeSystem::createCrystal()
{
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::CRYSTAL))
	{
		createBoundaryLine(renderer, position, scale);
	}

	createCrystal1(renderer, vec2(1100, 240));
	createCrystal2(renderer, vec2(175, 490));
	createCrystal3(renderer, vec2(340, 170));
	createCrystal4(renderer, vec2(100, 92));

	createCrystalMinecart(renderer, vec2(986, 530));
	createCrystalRock(renderer, vec2(639, 262));
	createCrystalPage(renderer, vec2(966, 510));

	createCollectableIngredient(renderer, vec2(491, 90), ItemType::CRYSTABLOOM, 1, true);
	createCollectableIngredient(renderer, vec2(458, 355), ItemType::CRYSTAL_SHARD, 1, true);
	createCollectableIngredient(renderer, vec2(302, 617), ItemType::CRYSTAL_SHARD, 1, true);
	createCollectableIngredient(renderer, vec2(1141, 624), ItemType::QUARTZMELON, 1, true);

	createCrystalToMushroom(renderer, vec2(50, 200), "Crystal To Mushroom");
	createCrystalToForestEx(renderer, vec2(930, 30), "Crystal to Forest Ex");
}