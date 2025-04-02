#pragma once

#include "./tinyECS/registry.hpp"
#include "./tinyECS/components.hpp"
#include "common.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_init.hpp"

class BiomeSystem
{
public:

    void init(RenderSystem* renderer_arg); // initialize screen state
    void step(float elapsed_ms);
    void switchBiome(int biome, bool is_first_load); // handle switching biome
    bool handleEntranceInteraction(Entity entrance_entity);
    void renderPlayerInNewBiome(bool is_first_load);

    void createGrotto();
    void createForest();
    void createForestEx();
    void createDesert();
    void createMushroom();
    void createCrystal();

    bool desert_unlocked = false;
    bool mushroom_unlocked = false;
    bool crystal_unlocked = false;

    void setUISystem(UISystem* ui_system) { m_ui_system = ui_system; }

    BiomeSystem()
    {
    }

private:
    RenderSystem* renderer;

    // <position, scale> of boundary lines
    std::map<int, std::vector<std::tuple<vec2, vec2>>> biome_boundaries = {
        {(int)BIOME::GROTTO,
            {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX * 4), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
             std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
             std::make_tuple(vec2(0, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
             std::make_tuple(vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 11, GRID_CELL_HEIGHT_PX * 13.3), vec2(750, BOUNDARY_LINE_THICKNESS)), // entrance left
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 24, GRID_CELL_HEIGHT_PX * 13.3), vec2(150, BOUNDARY_LINE_THICKNESS)), // entrance right
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 13.6, GRID_CELL_HEIGHT_PX * 7), vec2(GRID_CELL_WIDTH_PX * 2.6, GRID_CELL_HEIGHT_PX * 1.5)), // cauldron 
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 4.75, GRID_CELL_HEIGHT_PX * 6.35), vec2(460, BOUNDARY_LINE_THICKNESS)), // tools bottom
             std::make_tuple(vec2(475, 260), vec2(BOUNDARY_LINE_THICKNESS, 110)) // tools right
           }},
        {(int)BIOME::FOREST,
            {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX * 1.3), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
             std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX - 10), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
             std::make_tuple(vec2(5, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
             std::make_tuple(vec2(WINDOW_WIDTH_PX - 25, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 2), vec2(GRID_CELL_WIDTH_PX * 8, BOUNDARY_LINE_THICKNESS)), // grotto bottom
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 16, GRID_CELL_HEIGHT_PX), vec2(BOUNDARY_LINE_THICKNESS, GRID_CELL_HEIGHT_PX * 2)), // grotto left
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 24, GRID_CELL_HEIGHT_PX), vec2(BOUNDARY_LINE_THICKNESS, GRID_CELL_HEIGHT_PX * 2))
            }},
        {(int)BIOME::DESERT,
            {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
            std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // desert bottom
            std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 9, WINDOW_HEIGHT_PX - GRID_CELL_HEIGHT_PX - 5), vec2(GRID_CELL_WIDTH_PX * 17, BOUNDARY_LINE_THICKNESS)), // desert bottom wall
            std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 17.5, WINDOW_HEIGHT_PX - 30), vec2(BOUNDARY_LINE_THICKNESS, GRID_CELL_HEIGHT_PX)), // desert bottom right
            std::make_tuple(vec2(GRID_CELL_WIDTH_PX, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
            std::make_tuple(vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
            }},
        {(int)BIOME::MUSHROOM,
            {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
                std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX - 50), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
                std::make_tuple(vec2(30, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
                std::make_tuple(vec2(WINDOW_WIDTH_PX - 40, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
            }},
        {(int)BIOME::CRYSTAL,
            {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
                std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX - 35), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
                std::make_tuple(vec2(40, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
                std::make_tuple(vec2(WINDOW_WIDTH_PX - 35, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
            }},
        {(int)BIOME::FOREST_EX,
            {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
                std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX - 20), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
                std::make_tuple(vec2(20, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
                std::make_tuple(vec2(WINDOW_WIDTH_PX - 30, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
            }},
    };

    std::vector<std::tuple<vec2, vec2, float, GLuint, int>> grotto_static_entity_pos = {
        {vec2({GRID_CELL_WIDTH_PX * 20.5, GRID_CELL_HEIGHT_PX * 9.6}), vec2({156, 330}), 0, (GLuint)TEXTURE_ASSET_ID::GROTTO_CARPET, 0},
        {vec2({GRID_CELL_WIDTH_PX * 21, GRID_CELL_HEIGHT_PX * 3}), vec2({335, 260}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_TOP_BOOKSHELF, 1},
        {vec2({GRID_CELL_WIDTH_PX * 24.2, GRID_CELL_HEIGHT_PX * 8.8}), vec2({90, 429}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_RIGHT_BOOKSHELF, 1},
        {vec2({GRID_CELL_WIDTH_PX * 4.8, GRID_CELL_HEIGHT_PX * 11}), vec2({510, 215}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_POOL, 2}
    };

    UISystem* m_ui_system = nullptr;

};

