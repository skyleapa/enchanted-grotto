#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);
	static std::vector<vec2> get_transformed_vertices(const Mesh& mesh, const Motion& motion);
	static bool collides(const Motion& player_motion, const Motion& terrain_motion, const Terrain* terrain, Entity terrain_entity);

	PhysicsSystem()
	{
	}

	std::vector<int> ammo_stopping_entities = {
		(int)TEXTURE_ASSET_ID::TREE,
		(int)TEXTURE_ASSET_ID::GROTTO_ENTRANCE,
		(int)TEXTURE_ASSET_ID::GROTTO_CAULDRON,
		(int)TEXTURE_ASSET_ID::GROTTO_CHEST,
		(int)TEXTURE_ASSET_ID::GROTTO_MORTAR_PESTLE,
		(int)TEXTURE_ASSET_ID::GROTTO_RECIPE_BOOK,
		(int)TEXTURE_ASSET_ID::GROTTO_RIGHT_BOOKSHELF,
		(int)TEXTURE_ASSET_ID::GROTTO_TOP_BOOKSHELF,
		(int)TEXTURE_ASSET_ID::DESERT_CACTUS,
		(int)TEXTURE_ASSET_ID::DESERT_SKULL,
		(int)TEXTURE_ASSET_ID::DESERT_TREE,
		(int)TEXTURE_ASSET_ID::DESERT_SAND_PILE_PAGE,
		(int)TEXTURE_ASSET_ID::BUSH,
		(int)TEXTURE_ASSET_ID::ENT,
		(int)TEXTURE_ASSET_ID::MUMMY,
		(int)TEXTURE_ASSET_ID::WELCOME_TO_GROTTO,
		(int)TEXTURE_ASSET_ID::MUSHROOM_BLUE,
		(int)TEXTURE_ASSET_ID::MUSHROOM_PINK,
		(int)TEXTURE_ASSET_ID::MUSHROOM_PURPLE,
		(int)TEXTURE_ASSET_ID::MUSHROOM_TALL_BLUE,
		(int)TEXTURE_ASSET_ID::MUSHROOM_TALL_PINK,
		(int)TEXTURE_ASSET_ID::CRYSTAL_1,
		(int)TEXTURE_ASSET_ID::CRYSTAL_2,
		(int)TEXTURE_ASSET_ID::CRYSTAL_3,
		(int)TEXTURE_ASSET_ID::CRYSTAL_4,
		(int)TEXTURE_ASSET_ID::CRYSTAL_MINECART,
		(int)TEXTURE_ASSET_ID::CRYSTAL_ROCK
	};
};