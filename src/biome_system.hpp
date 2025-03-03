#include <vector>
#include "./tinyECS/registry.hpp"
#include "./tinyECS/components.hpp"
#include "common.hpp"
#include "render_system.hpp"
#include "world_init.hpp"

// to handle circular dependency


class BiomeSystem
{
public:

    void init(RenderSystem* renderer_arg); // initialize screen state
    void step(float elapsed_ms);
    void switchBiome(int biome); // handle switching biome
    bool handle_entrance_interaction(Entity entrance_entity);

    // create objects that are rendered onto the forest biome
	void createForest();

	// create objects that are rendered in the grotto
	void createGrotto();

    BiomeSystem()
    {
    }

private:
    RenderSystem* renderer;
};

