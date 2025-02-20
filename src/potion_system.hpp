#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// Everything to do with potion creation
// Note: For filling cauldrons, all that's necessary is to set the
// filled boolean to true, nothing needs to be done here
class PotionSystem {
public:
	PotionSystem() {}

	// Update active cauldrons. Checks for wait actions, and
	// updates the color of active cauldrons.
	void updateCauldrons(float elapsed_ms);

	// Record adding an ingredient to a cauldron. Requires that the
	// cauldron entity has an inventory component, and the ingredient
	// entity has an item AND ingredient component!
	static void addIngredient(Entity cauldron, Entity ingredient);

	// Record a heatknob modification to the cauldron
	// value is an integer from 1-100, where 100 is max heat
	static void changeHeat(Entity cauldron, int value);

	// Record a stir action to the cauldron
	// Call this only when the mouse is released from the ladle
	static void stirCauldron(Entity cauldron, int stirs);

	// Gets the resulting potion in its current state and
	// empties the cauldron, resetting its values. Requires that
	// the cauldron is filled (but no other requirements)
	static Potion bottlePotion(Entity cauldron);

private:
	// Base method for recording an action to a cauldron
	static void recordAction(Entity cauldron, ActionType action, int value);

	// Returns a COPY of the potion currently associated with this entity.
	static Potion getPotion(Entity cauldron);

	// Returns a default water potion, used if no potion component is
	// currently associated with the cauldron
	static Potion getDefaultPotion();

	// Gets a color value that is a percentage distance between the start and end
	// ratio is a number between 0 and 1 representing the percentage
	static vec3 interpolateColor(vec3 init, vec3 end, float ratio);

	// Takes the current state of the cauldron and updates it through a potion component
	// in the registry. This method is called after each action, and updates the color 
	// and result on the backend should the player bottle the potion. Do not call this 
	// function if the cauldron is empty.
	// 
	// The effect of a potion is calculated by checking the ItemTypes added,
	// ignoring their amount and grindlevel. If the ItemTypes match the ones
	// specified for a recipe, the resulting potion type must be the one specified
	// in that recipe. If no recipe is found, then the potion is a failed one. If
	// no ingredients have been added, then the potion is getDefaultPotion()
	//
	// Next, the potency of the potion is computed using Levenshtein distance formula,
	// which computes the minimum number of edits that would be required to change the
	// user actions to match the recipe actions. Actions are considered equal if the
	// ActionType is equal, but a small penalty will be applied depending on how far
	// away the value is. For add ingredient actions, the value difference takes into
	// account the type, amount, and grindlevel of the ingredient.
	//
	// The color of the potion is somewhere between DEFAULT_COLOR and the color specified
	// in the recipe in terms of RGB values, with the delta being computed from the 
	// the difference between the actual potency and its max.
	//
	// Formulas:
	// quality = max(#steps - (ld + penalty) * POTION_DIFFICULTY, 0) / #steps
	//   where:
	//   - quality is float from 0-1 that determines how good the potion is
	//   - #steps is the amount of steps in the recipe
	//   - ld is the levenshtein distance
	//   - penalty is applied for the ld actions with equal type but
	//     different values according to constants defined in common.hpp
	//
	// duration = min_duration + (max_duration - min_duration) * quality
	// potency = min_potency + (max_potency - min_potency) * quality
	// color = default_color +/- abs(max_color - default_color) * quality
	//   color is calculated per-channel and is added or subtracted
	//   based on if the approaching color is higher or lower
	static void updatePotion(Entity cauldron);
};