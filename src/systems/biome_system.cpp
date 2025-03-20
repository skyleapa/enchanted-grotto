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

	switchBiome(screen.biome);
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
				switchBiome((int)screen.biome);
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

void BiomeSystem::switchBiome(int biome) {
	std::vector<Entity> to_remove;
	for (auto entity : registry.motions.entities) {
		if (registry.players.has(entity) || registry.inventories.has(entity)) continue;

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
			if (registry.cauldrons.components[0].is_boiling) SoundSystem::playBoilSound((int) SOUND_CHANNEL::BOILING, -1);
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

	renderPlayerInNewBiome();
}

void BiomeSystem::renderPlayerInNewBiome() {
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
			}
			// recreate textbox
			if (registry.motions.has(cauldron)) {
				Motion& motion = registry.motions.get(cauldron);
				createTextbox(renderer, vec2(motion.position.x + 60, motion.position.y - 40), cauldron, "[F] Use Cauldron");
			}

		}

		for (Entity mortar : registry.mortarAndPestles.entities) {
			if (registry.renderRequests.has(mortar)) {
				RenderRequest& rr = registry.renderRequests.get(mortar);
				rr.is_visible = true;
				std::cout << "re-rendering mortar" << std::endl;
			}
			// recreate textbox
			if (registry.motions.has(mortar)) {
				createTextbox(renderer, { GRID_CELL_WIDTH_PX * 6.5, GRID_CELL_HEIGHT_PX * 3 }, mortar, "[F] Use Mortar & Pestle");
			}

		}
	}
	else if (screen.from_biome == (int)BIOME::GROTTO && screen.biome == (int)BIOME::FOREST) { // through grotto exit into forest
		player_motion.position = vec2(GROTTO_ENTRANCE_X, GROTTO_ENTRANCE_Y + 50);

		// ensure to make cauldron invisble since it should still exist but only rendered when in grotto
		for (Entity cauldron : registry.cauldrons.entities) {
			if (registry.renderRequests.has(cauldron)) {
				RenderRequest& rr = registry.renderRequests.get(cauldron);
				rr.is_visible = false;
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

	createGrottoPoolMesh(renderer, vec2(GRID_CELL_WIDTH_PX * 4.8, GRID_CELL_HEIGHT_PX * 11.9));

	if (registry.cauldrons.entities.size() == 0) {
		std::cout << "creating cauldron in grotto" << std::endl;
		Entity new_cauldron = createCauldron(renderer, vec2({ GRID_CELL_WIDTH_PX * 13.50, GRID_CELL_HEIGHT_PX * 6.05 }), vec2({ 150, 220 }), "Cauldron", false);
		for (Entity cauldron : registry.cauldrons.entities) {
			if (new_cauldron != cauldron) registry.remove_all_components_of(cauldron);
		}
	}
	// assert(registry.cauldrons.entities.size() == 1); // We should always only have one cauldron for testing purposes

	if (registry.mortarAndPestles.entities.size() == 0) {
		std::cout << "creating mortar in grotto" << std::endl;
		Entity new_mortar = createMortarPestle(renderer, vec2({ GRID_CELL_WIDTH_PX * 7.5, GRID_CELL_HEIGHT_PX * 5.22 }), vec2({ 213, 141 }), "Mortar and Pestle");
		for (Entity mortar : registry.mortarAndPestles.entities) {
			if (new_mortar != mortar) registry.remove_all_components_of(mortar);
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

	createTree(renderer, vec2(714, 465));
	createTree(renderer, vec2(857, 540));
	createTree(renderer, vec2(1080, 500));

	createBush(renderer, vec2(GRID_CELL_WIDTH_PX * 11, GRID_CELL_HEIGHT_PX * 11.5));
	createBush(renderer, vec2(1100, 292));

	ScreenState screen = registry.screenStates.components[0];
	if (std::find(screen.killed_enemies.begin(), screen.killed_enemies.end(), "Ent 1") == screen.killed_enemies.end())
	{
		createEnt(renderer, vec2(GRID_CELL_WIDTH_PX * 2.15, GRID_CELL_HEIGHT_PX * 5), 0, "Ent 1");
	}

	createForestToGrotto(renderer, vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 1), "Grotto Entrance");
	createForestToDesert(renderer, vec2(GRID_CELL_WIDTH_PX * 2.1, GRID_CELL_HEIGHT_PX * 1.2), "Desert Entrance");
	createForestToMushroom(renderer, vec2(GRID_CELL_WIDTH_PX * 2.1, WINDOW_HEIGHT_PX - 40), "Mushroom Entrance");
	createForestToForestEx(renderer, vec2(WINDOW_WIDTH_PX, 470), "Forest Ex Entrance");
}

void BiomeSystem::createForestEx()
{
	for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::FOREST_EX))
	{
		createBoundaryLine(renderer, position, scale);
	}

	createTree(renderer, vec2(130, 130));
	createTree(renderer, vec2(157, 540));
	createTree(renderer, vec2(216, 240));
	createTree(renderer, vec2(403, 180));
	createTree(renderer, vec2(504, 535));
	createTree(renderer, vec2(857, 140));
	createTree(renderer, vec2(1120, 280));
	createTree(renderer, vec2(1080, 535));

	createBush(renderer, vec2(920, 392));

	createForestExToForest(renderer, vec2(50, 470), "Forest Ex to Forest");
	createForestExToCrystal(renderer, vec2(930, 665), "Forest Ex to Crystal");
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

	createMushroomToForest(renderer, vec2(60, 50), "Mushroom To Forest");
	createMushroomToCrystal(renderer, vec2(1220, 200), "Mushroom to Crystal");
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

	createCrystalToMushroom(renderer, vec2(50, 200), "Crystal To Mushroom");
	createCrystalToForestEx(renderer, vec2(930, 30), "Crystal to Forest Ex");
}