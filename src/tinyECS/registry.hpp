#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<GridLine> gridLines;

	// Enchanted Grotto CCs
	ComponentContainer<Potion> potions;
	ComponentContainer<Item> items;
	ComponentContainer<Ingredient> ingredients;
	ComponentContainer<Inventory> inventories;
	ComponentContainer<Cauldron> cauldrons;
	ComponentContainer<Menu> menus;
	ComponentContainer<Recipe> recipes;
	ComponentContainer<MortarAndPestle> mortarAndPestles;
	ComponentContainer<Moving> moving;
	ComponentContainer<Terrain> terrains;
	ComponentContainer<Textbox> textboxes;

	// constructor that adds all containers for looping over them
	ECSRegistry()
	{
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&gridLines);
		registry_list.push_back(&potions);
		registry_list.push_back(&items);
		registry_list.push_back(&ingredients);
		registry_list.push_back(&inventories);
		registry_list.push_back(&cauldrons);
		registry_list.push_back(&menus);
		registry_list.push_back(&recipes);
		registry_list.push_back(&mortarAndPestles);
		registry_list.push_back(&moving);
		registry_list.push_back(&terrains);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;