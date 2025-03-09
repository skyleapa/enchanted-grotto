#include "common.hpp"
#include "biome_system.hpp"
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

#include <vector>

void BiomeSystem::init(RenderSystem* renderer_arg) {
    this->renderer = renderer_arg;
    ScreenState& screen = registry.screenStates.components[0];
    screen.darken_screen_factor = 0;
    screen.fade_status = 0;
    screen.is_switching_biome = false;
    screen.biome = (GLuint)BIOME::FOREST;
    screen.from_biome = (GLuint)BIOME::FOREST;

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
    Motion& player_motion = registry.motions.get(player);

    if (screen.is_switching_biome)
    {
        if (screen.fade_status == 0)
        {
            screen.darken_screen_factor += elapsed_ms_since_last_update * TIME_UPDATE_FACTOR;
            if (screen.darken_screen_factor >= 1)
                screen.fade_status = 1; // after fade out
        }
        else if (screen.fade_status == 1)
        {
            if (screen.biome != screen.switching_to_biome)
            {
                screen.from_biome = screen.biome;
                screen.biome = screen.switching_to_biome;
                screen.darken_screen_factor = 1;
                switchBiome((int)screen.biome);
                renderPlayerInNewBiome();
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
        to_remove.push_back(entity);
    }

    for (auto entity : to_remove) {
        registry.remove_all_components_of(entity);
    }

    if (biome == (GLuint)BIOME::FOREST)
    {
        createForest();
    }
    else if (biome == (GLuint)BIOME::GROTTO)
    {
        createGrotto();
    }
    else if (biome == (GLuint)BIOME::DESERT) {
        createDesert();
    }
}

void BiomeSystem::renderPlayerInNewBiome() {
    if (registry.players.entities.size() == 0) return;

    Entity player_entity = registry.players.entities[0];
    if (!registry.motions.has(player_entity)) return;

    ScreenState screen = registry.screenStates.components[0];
    Motion& player_motion = registry.motions.get(player_entity);

    player_motion.scale = { PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT };

    if (screen.from_biome == (int)BIOME::FOREST && screen.biome == (int)BIOME::GROTTO) { // through grotto entrance from forest
        player_motion.scale = { PLAYER_BB_WIDTH * PlAYER_BB_GROTTO_SIZE_FACTOR, PLAYER_BB_HEIGHT * PlAYER_BB_GROTTO_SIZE_FACTOR };
        player_motion.position = vec2({ player_motion.position.x, GRID_CELL_HEIGHT_PX * 11 }); // bring player to front of door
    }
    else if (screen.from_biome == (int)BIOME::GROTTO && screen.biome == (int)BIOME::FOREST) { // through grotto exit into forest
        player_motion.position = vec2(GROTTO_ENTRANCE_X, GROTTO_ENTRANCE_Y + 50);
    }
    else if (screen.from_biome == (int)BIOME::FOREST && screen.biome == (int)BIOME::DESERT) {
        player_motion.position = vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 12);
    }
    else if (screen.from_biome == (int)BIOME::DESERT && screen.biome == (int)BIOME::FOREST) {
        player_motion.position = vec2(GRID_CELL_WIDTH_PX * 2, GRID_CELL_HEIGHT_PX * 2);
    }
}

void BiomeSystem::createForest()
{
    // create boundaries
    for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::FOREST))
    {
        create_boundary_line(renderer, position, scale);
    }

    // create forest bridge
    createForestBridge(renderer, vec2(307, 485));
    createForestBridgeTop(renderer, vec2(307, 425));
    createForestBridgeBottom(renderer, vec2(309, 545));

    // NOTE: leaving this in for debugging vertices of meshes for the future
    /*
        Entity terrain_entity = createForestBridgeTop(renderer, vec2(700, 400));
        Mesh* mesh = registry.meshPtrs.get(terrain_entity);
        Motion motion = registry.motions.get(terrain_entity);
        std::vector<vec2> transformed_vertices = PhysicsSystem::get_transformed_vertices(*mesh, motion);

        for (vec2 vertex : transformed_vertices) {
            createCoffeeBean(renderer, vertex, "vertex bean", 1);
        }
    */

    // create forest river
    createForestRiver(renderer, vec2(307, 0));

    // create trees
    createTree(renderer, vec2(GRID_CELL_WIDTH_PX * 11, GRID_CELL_HEIGHT_PX * 3));
    createTree(renderer, vec2(GRID_CELL_WIDTH_PX * 19, GRID_CELL_HEIGHT_PX * 10));
    createTree(renderer, vec2(GRID_CELL_WIDTH_PX * 23, GRID_CELL_HEIGHT_PX * 11));

    createGrottoEntrance(renderer, vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 1), 7, "Grotto Entrance");

    createBush(renderer, vec2(GRID_CELL_WIDTH_PX * 11, GRID_CELL_HEIGHT_PX * 12));

    createFruit(renderer, vec2(GRID_CELL_WIDTH_PX * 10, GRID_CELL_HEIGHT_PX * 3), "Magical Fruit", 1);
    createFruit(renderer, vec2(GRID_CELL_WIDTH_PX * 19, GRID_CELL_HEIGHT_PX * 10), "Magical Fruit", 1);
    createFruit(renderer, vec2(GRID_CELL_WIDTH_PX * 23, GRID_CELL_HEIGHT_PX * 11), "Magical Fruit", 1);

    createCoffeeBean(renderer, vec2(GRID_CELL_WIDTH_PX * 11, GRID_CELL_HEIGHT_PX * 11.5), "Coffee Bean", 1);
    createCoffeeBean(renderer, vec2(GRID_CELL_WIDTH_PX * 9.9, GRID_CELL_HEIGHT_PX * 12.1), "Coffee Bean", 1);
    createCoffeeBean(renderer, vec2(GRID_CELL_WIDTH_PX * 12, GRID_CELL_HEIGHT_PX * 12.7), "Coffee Bean", 1);

    createEnt(renderer, vec2(GRID_CELL_WIDTH_PX * 1.7, GRID_CELL_HEIGHT_PX * 5), 0);

    createDesertEntrance(renderer, vec2(GRID_CELL_WIDTH_PX * 2.1, GRID_CELL_HEIGHT_PX * 1.7), 7, "Desert Entrance");
}

void BiomeSystem::createGrotto()
{
    // positions are according to sample grotto interior
    for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::GROTTO))
    {
        create_boundary_line(renderer, position, scale);
    }

    for (const auto& [position, size, rotation, texture, layer] : grotto_static_entity_pos) {
        create_grotto_static_entities(renderer, position, size, rotation, texture, layer);
    }

    createCauldron(renderer, vec2({ GRID_CELL_WIDTH_PX * 13.35, GRID_CELL_HEIGHT_PX * 5.85 }), vec2({ 175, 280 }), 8, "Cauldron");
    createMortarPestle(renderer, vec2({ GRID_CELL_WIDTH_PX * 7.5, GRID_CELL_HEIGHT_PX * 5.22 }), vec2({ 213, 141 }), 9, "Mortar and Pestle");
    createRecipeBook(renderer, vec2({ GRID_CELL_WIDTH_PX * 4.15, GRID_CELL_HEIGHT_PX * 5.05 }), vec2({ 108, 160 }), 10, "Recipe Book");
    createChest(renderer, vec2({ GRID_CELL_WIDTH_PX * 1.35, GRID_CELL_HEIGHT_PX * 5.2 }), vec2({ 100, 150 }), 11, "Chest");
    createGrottoExit(renderer, vec2(GRID_CELL_WIDTH_PX * 20.5, GRID_CELL_HEIGHT_PX * 13), 12, "Grotto Exit");
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
    else if (entrance.target_biome == (GLuint)BIOME::DESERT) {
        state.is_switching_biome = true;
        state.switching_to_biome = (GLuint)BIOME::DESERT;
    }
    return true;
}

void BiomeSystem::createDesert()
{
    // positions are according to sample desert
    for (const auto& [position, scale] : biome_boundaries.at((int)BIOME::DESERT)) // desert
    {
        create_boundary_line(renderer, position, scale);
    }

    createDesertExit(renderer, vec2(GRID_CELL_WIDTH_PX * 20.3, GRID_CELL_HEIGHT_PX * 12.9), 8, "Desert Exit");
    createDesertTree(renderer, vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 3.9));
    createDesertCactus(renderer, vec2(GRID_CELL_WIDTH_PX * 4.1, GRID_CELL_HEIGHT_PX * 6.2));
    createDesertRiver(renderer, vec2(GRID_CELL_WIDTH_PX * 24, WINDOW_HEIGHT_PX / 2));
    createDesertPage(renderer, vec2(GRID_CELL_WIDTH_PX * 13.5, GRID_CELL_HEIGHT_PX * 3.2));
    createDesertSkull(renderer, vec2(GRID_CELL_WIDTH_PX * 13.7, GRID_CELL_HEIGHT_PX * 10.9));

    createMummy(renderer, vec2(GRID_CELL_WIDTH_PX * 15, GRID_CELL_HEIGHT_PX * 5), 1);
    createMummy(renderer, vec2(GRID_CELL_WIDTH_PX * 4, GRID_CELL_HEIGHT_PX * 8), 1);
}