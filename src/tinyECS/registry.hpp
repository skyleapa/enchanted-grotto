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
	ComponentContainer<MortarAndPestle> mortarAndPestles;
	ComponentContainer<Terrain> terrains;
	ComponentContainer<Entrance> entrances;
	ComponentContainer<Textbox> textboxes;
	ComponentContainer<Animation> animations;
	ComponentContainer<Chest> chests;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Guardian> guardians;
	ComponentContainer<Ammo> ammo;
	ComponentContainer<WelcomeScreen> welcomeScreens;
	ComponentContainer<DamageFlash> damageFlashes;
	ComponentContainer<Regeneration> regen;
	ComponentContainer<TexturedEffect> texturedEffects;
	ComponentContainer<DelayedMovement> delayedMovements;

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
		registry_list.push_back(&mortarAndPestles);
		registry_list.push_back(&terrains);
		registry_list.push_back(&entrances);
		registry_list.push_back(&textboxes);
		registry_list.push_back(&animations);
		registry_list.push_back(&chests);
		registry_list.push_back(&enemies);
		registry_list.push_back(&guardians);
		registry_list.push_back(&ammo);
		registry_list.push_back(&welcomeScreens);
		registry_list.push_back(&damageFlashes);
		registry_list.push_back(&regen);
		registry_list.push_back(&texturedEffects);
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