#include "potion_system.hpp"
#include "item_system.hpp"
#include "tinyECS/registry.hpp"
#include <unordered_set>
#include <iostream>
#include <cfloat>

void PotionSystem::updateCauldrons(float elapsed_ms) {
	for (Entity cauldron : registry.cauldrons.entities) {
		Cauldron& cc = registry.cauldrons.get(cauldron);
		if (!cc.filled) {
			continue;
		}

		// Don't need to update cauldrons that have no actions
		if (cc.actions.size() == 0) {
			continue;
		}

		cc.timeSinceLastAction += elapsed_ms;
		cc.timeElapsed += elapsed_ms;

		// Add wait action if threshold exceeded
		if (cc.timeSinceLastAction >= DEFAULT_WAIT) {
			std::cout << "WAIT action recorded!" << std::endl;
			recordAction(cauldron, ActionType::WAIT, cc.timeSinceLastAction / DEFAULT_WAIT);
			cc.timeSinceLastAction = 0;
		}

		// Update cauldron color
		if (cc.colorElapsed >= COLOR_FADE_DURATION) {
			continue;
		}

		// Cauldron color approaches the currently stored potion's color
		// This process actually isn't linear because the ratio is between
		// the cauldron's current color and the stored color.
		cc.colorElapsed += elapsed_ms;
		float ratio = cc.colorElapsed / (float)COLOR_FADE_DURATION;
		cc.color = interpolateColor(cc.color, getPotion(cauldron).color, ratio);
	}
}

void PotionSystem::addIngredient(Entity cauldron, Entity ingredient) {
	Cauldron& cc = registry.cauldrons.get(cauldron);
	Inventory& ci = registry.inventories.get(cauldron);

	// If the last action added the exact same ingredient, then merge the amount
	// of this ingredient with the previous action
	// do-while-false allows breaking
	do {
		if (cc.actions.size() == 0) {
			break;
		}

		Action& lastAction = cc.actions[cc.actions.size() - 1];
		if (lastAction.type != ActionType::ADD_INGREDIENT) {
			break;
		}

		Entity lastIngredient = ci.items[lastAction.value];
		Item& curItem = registry.items.get(ingredient);
		Item& lastItem = registry.items.get(lastIngredient);
		if (lastItem.type != curItem.type) {
			break;
		}

		// Float comparison moment. Return if grindlevels are far enough apart
		Ingredient& curIng = registry.ingredients.get(ingredient);
		Ingredient& lastIng = registry.ingredients.get(lastIngredient);
		if (fabs(lastIng.grindLevel - curIng.grindLevel) >= FLT_EPSILON) {
			break;
		}

		lastItem.amount += curItem.amount;

		// handle tutorial adding 5 coffee beans and 3 magical fruits
		if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::ADD_INGREDIENT) {

			// this is buggy so just add 8 items
			bool added_fruits = false;
			bool added_beans = false;
			for (Entity& entity : ci.items) {
				if (!registry.items.has(entity)) continue;
				Item& item = registry.items.get(entity);
				if (item.type == ItemType::MAGICAL_FRUIT) {
					if (item.amount >= 3) {
						added_fruits = true;
					}
				}
				if (item.type == ItemType::COFFEE_BEANS) {
					if (item.amount >= 1) { // TODO CHANGE
						added_beans = true;
					}
				}
			}
			if (added_beans && added_fruits) {
				ScreenState& screen = registry.screenStates.components[0];
				screen.tutorial_step_complete = true;
				screen.tutorial_state += 1;
			}
		}
		updatePotion(cauldron);
		return;
	} while (false);

	ci.items.push_back(ingredient);
	recordAction(cauldron, ActionType::ADD_INGREDIENT, ci.items.size() - 1);

}

void PotionSystem::changeHeat(Entity cauldron, int value) {
	Cauldron& cc = registry.cauldrons.get(cauldron);
	if (cc.actions.size() == 0 && value == 0) {
		return;
	}

	cc.heatLevel = value;
	recordAction(cauldron, ActionType::MODIFY_HEAT, value);
}

void PotionSystem::stirCauldron(Entity cauldron) {
	Inventory& ci = registry.inventories.get(cauldron);
	if (ci.items.size() == 0) {
		return;
	}
	recordAction(cauldron, ActionType::STIR, 1);
}

Potion PotionSystem::bottlePotion(Entity cauldron) {
	Potion potion = getPotion(cauldron);

	// Clear cauldron
	Cauldron& cc = registry.cauldrons.get(cauldron);
	cc.color = DEFAULT_COLOR;
	// cc.filled = false; FOR NOW CAULDRON IS ALWAYS FILLED
	cc.colorElapsed = 0;
	cc.heatLevel = 0;
	cc.timeElapsed = 0;
	cc.timeSinceLastAction = 0;
	cc.actions.clear();

	// Clear cauldron items
	Inventory& cinv = registry.inventories.get(cauldron);
	for (Entity item : cinv.items) {
		ItemSystem::destroyItem(item);
	}
	cinv.items.clear();

	// handle bottling tutorial
	if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::BOTTLE) {
		ScreenState& screen = registry.screenStates.components[0];
		screen.tutorial_step_complete = true;
		screen.tutorial_state += 1;
	}
	return potion;
}

////////////////////////////
// Private Helpers below  //
////////////////////////////

void PotionSystem::recordAction(Entity cauldron, ActionType action, int value = 0) {
	Cauldron& cc = registry.cauldrons.get(cauldron);

	// For stir, heat, and wait actions, if the last action was the same action,
	// then the value gets added or replaced with the current one. Otherwise,
	// the action is appended to the end of the list
	do {
		if (cc.actions.size() == 0) {
			break;
		}

		// only 4 total action types exist
		if (action == ActionType::ADD_INGREDIENT) {
			break;
		}

		Action& lastAction = cc.actions[cc.actions.size() - 1];
		if (action != lastAction.type) {
			break;
		}

		if (action == ActionType::MODIFY_HEAT) {
			lastAction.value = value;
		}
		else {
			lastAction.value += value;
		}

		updatePotion(cauldron);
		return;
	} while (false);

	// Otherwise just add the action to the list
	cc.actions.push_back({ action, value });
	updatePotion(cauldron);
}

Potion PotionSystem::getPotion(Entity cauldron) {
	if (registry.potions.has(cauldron)) {
		return registry.potions.get(cauldron);
	}
	return getDefaultPotion();
}

Potion PotionSystem::getDefaultPotion() {
	Potion potion;
	potion.color = DEFAULT_COLOR;
	potion.duration = 0;
	potion.effect = PotionEffect::WATER;
	potion.effectValue = 0;
	potion.quality = 1.0f;
	return potion;
}

vec3 PotionSystem::interpolateColor(vec3 init, vec3 end, float ratio) {
	vec3 res(0, 0, 0);
	for (int i = 0; i < 3; i++) {
		float diff = abs(init[i] - end[i]) * ratio;
		if (end[i] > init[i]) {
			res[i] = init[i] + (int)diff;
		}
		else {
			res[i] = init[i] - (int)diff;
		}
	}
	return res;
}

///////////// Potion calculation functions ///////////

// Gets levenshtein distance and a penalty
std::pair<int, float> levDist(Entity cauldron, Recipe& recipe,
	std::vector<Action> playerActions, std::vector<Action> recipeActions) {
	// Return other length if respective length is 0
	if (playerActions.size() == 0) {
		return std::make_pair(recipeActions.size(), 0);
	}

	if (recipeActions.size() == 0) {
		return std::make_pair(playerActions.size(), 0);
	}

	// Setup tail vectors
	Action pa = playerActions[0];
	Action ra = recipeActions[0];
	std::vector<Action> paTail = playerActions;
	std::vector<Action> raTail = recipeActions;
	paTail.erase(paTail.begin());
	raTail.erase(raTail.begin());

	// Check minimum of next possible iterations if action is different
	if (pa.type != ra.type) {
		std::pair next = levDist(cauldron, recipe, paTail, raTail);
		std::pair alt1 = levDist(cauldron, recipe, paTail, recipeActions);
		std::pair alt2 = levDist(cauldron, recipe, playerActions, raTail);
		if (alt1.first < next.first) {
			next = alt1;
		}
		if (alt2.first < next.first) {
			next = alt2;
		}

		next.first += 1;
		return next;
	}

	// Check for penalties if actions is same
	float penalty = 0;
	int diff = abs(pa.value - ra.value);
	switch (pa.type) {
	case ActionType::ADD_INGREDIENT: {
		RecipeIngredient recipeIng = recipe.ingredients[ra.value];
		Inventory& ci = registry.inventories.get(cauldron);
		Entity& itemEntity = ci.items[pa.value];
		Item& item = registry.items.get(itemEntity);

		// Check equality of item type. If item type is not equal
		// do not apply any other penalties
		if (recipeIng.type != item.type) {
			penalty += INGREDIENT_TYPE_PENALTY;
			break;
		}

		// Check grind level and amount equality
		penalty += abs(item.amount - recipeIng.amount) * INGREDIENT_AMOUNT_PENALTY;
		Ingredient& ing = registry.ingredients.get(itemEntity);
		if (ing.grindLevel != -1) {
			penalty += abs(ing.grindLevel - recipeIng.grindAmount) * INGREDIENT_GRIND_PENALTY;
		}
		break;
	}
	case ActionType::WAIT:
		penalty += diff * WAIT_PENALTY;
		break;
	case ActionType::MODIFY_HEAT:
		penalty += diff * HEAT_PENALTY;
		break;
	case ActionType::STIR:
		penalty += diff * STIR_PENALTY;
		break;
	}

	std::pair<int, float> next = levDist(cauldron, recipe, paTail, raTail);
	next.second += penalty;
	return next;

}

void PotionSystem::updatePotion(Entity cauldron) {
	Cauldron& cc = registry.cauldrons.get(cauldron);
	Inventory& ci = registry.inventories.get(cauldron);
	Potion potion = getDefaultPotion();

	// Reset last action time, since every action triggers an update
	cc.timeSinceLastAction = 0;

	// Step 1: Get recipe
	Recipe recipe;
	bool foundRecipe = false;
	std::unordered_set<ItemType> cauldronTypes;
	for (Entity e : ci.items) {
		cauldronTypes.insert(registry.items.get(e).type);
	}

	for (Recipe r : RECIPES) {
		std::unordered_set<ItemType> recipeTypes;
		for (RecipeIngredient ri : r.ingredients) {
			recipeTypes.insert(ri.type);
		}

		if (cauldronTypes == recipeTypes) {
			recipe = r;
			foundRecipe = true;
			break;
		}
	}

	// Step 2: Get levenshtein distance and assign values for color, potency, and duration
	if (foundRecipe) {
		// Effect is already known
		potion.effect = recipe.effect;

		// Get important preliminary numbers
		std::pair<int, float> dist = levDist(cauldron, recipe, cc.actions, recipe.steps);
		int steps = recipe.steps.size();
		float min_potency = recipe.highestQualityEffect * MIN_POTENCY_PERCENTAGE;
		int min_duration = recipe.highestQualityDuration * MIN_DURATION_PERCENTAGE;

		// Assign based on formulas
		potion.quality = max(steps - (dist.first + dist.second) * POTION_DIFFICULTY, 0.f) / steps;
		potion.effectValue = min_potency + (recipe.highestQualityEffect - min_potency) * potion.quality;
		potion.duration = min_duration + (recipe.highestQualityDuration - min_duration) * potion.quality;
		potion.color = interpolateColor(DEFAULT_COLOR, recipe.finalPotionColor, potion.quality);
	}
	else if (ci.items.size() > 0) {
		// Otherwise, if there are ingredients, then its failed
		potion.effect = PotionEffect::FAILED;
	}

	// Update values in registry
	// Note: An extremely rare bug may happen here where a player
	// takes out a default potion because getPotion() executed
	// right in the middle of the two lines below. Just saying.
	registry.potions.remove(cauldron);
	registry.potions.insert(cauldron, potion);

	// Allow color update to happen
	cc.colorElapsed = 0;
}
