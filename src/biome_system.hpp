#include "./tinyECS/registry.hpp"
#include "./tinyECS/components.hpp"
#include "common.hpp"
#include "render_system.hpp"
#include "world_init.hpp"

class BiomeSystem
{
public:

    void init(RenderSystem* renderer_arg); // initialize screen state
    void step(float elapsed_ms);
    void switchBiome(int biome); // handle switching biome
    bool handleEntranceInteraction(Entity entrance_entity);

    // create objects that are rendered onto the forest biome
    void createForest();

    // create objects that are rendered in the grotto
    void createGrotto();

    BiomeSystem()
    {
    }

private:
    RenderSystem* renderer;

    std::map<int, std::vector<std::tuple<vec2, vec2>>> biome_boundaries = {
        {(int)BIOME::GROTTO,
            {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX * 4), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
             std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
             std::make_tuple(vec2(0, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
             std::make_tuple(vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 20.5, GRID_CELL_HEIGHT_PX * 13.6), vec2(190, BOUNDARY_LINE_THICKNESS)), // door
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 13.30, GRID_CELL_HEIGHT_PX * 7.5), vec2(GRID_CELL_WIDTH_PX * 3.7, GRID_CELL_HEIGHT_PX * 1.5)), // cauldron 
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 4.75, GRID_CELL_HEIGHT_PX * 6.35), vec2(460, BOUNDARY_LINE_THICKNESS)), // tools bottom
             std::make_tuple(vec2(475, 260), vec2(BOUNDARY_LINE_THICKNESS, 110)) // tools right
           }},
           {(int)BIOME::FOREST,
            {std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, GRID_CELL_HEIGHT_PX * 1.3), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // top
             std::make_tuple(vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX), vec2(WINDOW_WIDTH_PX, BOUNDARY_LINE_THICKNESS)), // bottom
             std::make_tuple(vec2(0, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // left
             std::make_tuple(vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX / 2), vec2(BOUNDARY_LINE_THICKNESS, WINDOW_HEIGHT_PX)), // right
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 20, GRID_CELL_HEIGHT_PX * 2), vec2(GRID_CELL_WIDTH_PX * 5, BOUNDARY_LINE_THICKNESS)), // grotto bottom
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 17.2, GRID_CELL_HEIGHT_PX), vec2(BOUNDARY_LINE_THICKNESS, GRID_CELL_HEIGHT_PX * 2)), // grotto left
             std::make_tuple(vec2(GRID_CELL_WIDTH_PX * 22.8, GRID_CELL_HEIGHT_PX), vec2(BOUNDARY_LINE_THICKNESS, GRID_CELL_HEIGHT_PX * 2))}}
    };

    std::vector<std::tuple<vec2, vec2, float, GLuint, int>> grotto_static_entity_pos = {
        {vec2({GRID_CELL_WIDTH_PX * 20.5, GRID_CELL_HEIGHT_PX * 9}), vec2({180, 310}), 0, (GLuint)TEXTURE_ASSET_ID::GROTTO_CARPET, 0},
        {vec2({GRID_CELL_WIDTH_PX * 21, GRID_CELL_HEIGHT_PX * 3}), vec2({335, 260}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_TOP_BOOKSHELF, 1},
        {vec2({GRID_CELL_WIDTH_PX * 24.2, GRID_CELL_HEIGHT_PX * 9.1}), vec2({90, 429}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_RIGHT_BOOKSHELF, 1},
        {vec2({GRID_CELL_WIDTH_PX * 4.8, GRID_CELL_HEIGHT_PX * 11.9}), vec2({510, 215}), 180, (GLuint)TEXTURE_ASSET_ID::GROTTO_POOL, 1}
    };

};

