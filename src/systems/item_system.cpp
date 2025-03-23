#include "item_system.hpp"
#include "ui_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include <fstream>

// initialize m_ui_system
UISystem* ItemSystem::m_ui_system = nullptr;

Entity ItemSystem::createItem(ItemType type, int amount, bool isCollectable, bool is_ammo, bool canRespawn) {
	Entity entity = Entity();
	// std::cout << "Entity " << entity.id() << " of type " << (int) type << std::endl;

	Item& item = registry.items.emplace(entity);
	item.type = type;
	item.amount = amount;
	item.isCollectable = isCollectable;
	item.name = ITEM_INFO.at(type).name;
	item.is_ammo = is_ammo;
	item.canRespawn = canRespawn;

	if (is_ammo) {
		registry.ammo.emplace(entity);
	}

	return entity;
}

Entity ItemSystem::createIngredient(ItemType type, int amount) {
	Entity entity = createItem(type, amount, false, false);

	// Add ingredient-specific component
	Ingredient& ingredient = registry.ingredients.emplace(entity);
	ingredient.grindLevel = 0.0f;  // Starts ungrounded
	return entity;
}

Entity ItemSystem::createPotion(PotionEffect effect, int duration, const vec3& color, float quality, float effectValue, int amount) {
	bool is_throwable = std::find(throwable_potions.begin(), throwable_potions.end(), effect) != throwable_potions.end();
	Entity entity = createItem(ItemType::POTION, amount, false, is_throwable, false);

	// Add potion-specific component
	Potion& potion = registry.potions.emplace(entity);
	potion.effect = effect;
	potion.duration = duration;
	potion.color = color;
	potion.quality = quality;
	potion.effectValue = effectValue;

	// Get potion name
	return entity;
}

std::string ItemSystem::getItemName(Entity item) {
	Item& it = registry.items.get(item);
	std::string name = ITEM_INFO.at(it.type).name;

	// Add grind stat for ingredient
	if (registry.ingredients.has(item)) {
		int lvl = (int) (registry.ingredients.get(item).grindLevel * 100);
		if (lvl > 0) {
			name += " (" + std::to_string(lvl) + "% Grinded)";
		}
	}

	// Add potion name for potions
	if (registry.potions.has(item)) {
		Potion& potion = registry.potions.get(item);
		if (potion.effect == PotionEffect::WATER) {
			return "Flask of Holy Water";
		} else if (potion.effect == PotionEffect::FAILED) {
			return "Failed Potion";
		}
		
		for (Recipe r : RECIPES) {
			if (potion.effect == r.effect) {
				name = r.name;
				break;
			}
		}

		name = PotionSystem::getNormalizedQuality(potion).name + " " + name;
	}

	return name;
}


Entity ItemSystem::createCollectableIngredient(vec2 position, ItemType type, int amount, bool canRespawn) {
	Entity item = createItem(type, amount, true, false, canRespawn);
	registry.items.get(item).originalPosition = position;
	Ingredient& ing = registry.ingredients.emplace(item);
	ing.grindLevel = ITEM_INFO.at(type).grindable ? 0.f : -1.f;

	return item;
}

void ItemSystem::step(float elapsed_ms) {
	// TODO - Update any time-based item effects here, unused for now
	(void)elapsed_ms;
}

Entity ItemSystem::createItemEntity(ItemType type, int amount) {
	return createItem(type, amount, false, false);
}

void ItemSystem::destroyItem(Entity item) {
	if (registry.items.has(item)) {
		registry.remove_all_components_of(item);
	}
}

bool ItemSystem::addItemToInventory(Entity inventory, Entity item) {
	if (!registry.inventories.has(inventory) || !registry.items.has(item)) {
		return false;
	}

	Inventory& inv = registry.inventories.get(inventory);
	Item& item_comp = registry.items.get(item);

	// Try to stack if possible
	for (Entity existing : inv.items) {
		if (!registry.items.has(existing)) {
			continue;
		}

		// Do not stack if item types are different
		Item& existing_item = registry.items.get(existing);
		if (existing_item.type != item_comp.type) {
			continue;
		}

		// Do not stack if ingredient grindlevels are different
		if (registry.ingredients.has(existing) && registry.ingredients.has(item)) {
			Ingredient& existingIng = registry.ingredients.get(existing);
			Ingredient& newIng = registry.ingredients.get(item);
			if (fabs(existingIng.grindLevel - newIng.grindLevel) > FLT_EPSILON) {
				continue;
			}
		}

		// FAILED potions stack based on COLOR
		// Otherwise, potions stack based on QUALITY
		// Potion quality should be normalized at this point
		if (existing_item.type == ItemType::POTION) {
			Potion& first = registry.potions.get(existing);
			Potion& second = registry.potions.get(item);
			if (first.effect != second.effect) {
				continue;
			}

			if (first.effect == PotionEffect::FAILED && first.color != second.color) {
				continue;
			}

			if (fabs(first.quality - second.quality) > FLT_EPSILON) {
				continue;
			}
		}

		// Add amounts together
		existing_item.amount += item_comp.amount;
		if (registry.ammo.has(item) && !registry.ammo.has(existing)) {
			registry.ammo.emplace(existing); // update ammo in case it didn't previous save component
		}

		// Don't destroy the original item if it's a collectable (it will respawn)
		if (!item_comp.isCollectable) {
			destroyItem(item);
		}

		// update bar if inventory belongs to player
		if (registry.players.has(inventory) && m_ui_system != nullptr) {
			m_ui_system->updateInventoryBar();
		}

		return true;
	}

	// If we couldn't stack, check capacity
	if (inv.isFull) {
		return false;
	}

	// If item is collectable, create a copy for the inventory
	if (item_comp.isCollectable) {
		Entity copy = copyItem(item);
		inv.items.push_back(copy);
	}
	else {
		// std::cout << "Pushed new item: " << item_comp.name << std::endl;
		inv.items.push_back(item);
	}

	// Update capacity
	if (inv.items.size() >= inv.capacity) {
		inv.isFull = true;
	}
	
	std::cout << "Added new item: " << item_comp.name << " to inventory." << std::endl;

	// update bar if inventory belongs to player
	if (registry.players.has(inventory) && m_ui_system != nullptr) {
		m_ui_system->updateInventoryBar();
	}

	return true;
}

bool ItemSystem::removeItemFromInventory(Entity inventory, Entity item) {
	if (!registry.inventories.has(inventory)) {
		return false;
	}

	Inventory& inv = registry.inventories.get(inventory);
	auto it = std::find(inv.items.begin(), inv.items.end(), item);

	if (it != inv.items.end()) {
		inv.items.erase(it);
		inv.isFull = false;
		return true;
	}

	// update bar if inventory belongs to player
	if (registry.players.has(inventory) && m_ui_system != nullptr) {
		m_ui_system->updateInventoryBar();
	}
	return false;
}

bool ItemSystem::transferItem(Entity source_inventory, Entity target_inventory, Entity item) {
	if (removeItemFromInventory(source_inventory, item)) {
		if (addItemToInventory(target_inventory, item)) {
			return true;
		}
		// If adding to target failed, put it back in source
		addItemToInventory(source_inventory, item);
	}
	return false;
}

void ItemSystem::swapItems(Entity inventory, int slot1, int slot2) {
	std::vector<Entity>& items = registry.inventories.get(inventory).items;
	if (items.size() <= slot1 || items.size() <= slot2) {
		return;
	}
	std::iter_swap(items.begin() + slot1, items.begin() + slot2);
}

Entity ItemSystem::copyItem(Entity toCopy) {
	const Item item = registry.items.get(toCopy);
	Entity res = Entity();
	// std::cout << "Entity " << res.id() << " item copy" << std::endl;
	registry.items.emplace(res, item);
	if (registry.ingredients.has(toCopy)) {
		//std::cout << "Entity " << oldIng.grindLevel << " item copy" << std::endl;
		auto& oldIng = registry.ingredients.get(toCopy);
		registry.ingredients.emplace(res, Ingredient(oldIng));
		// std::cout << "Entity " << oldIng.grindLevel << " item copy" << std::endl;
	}
	if (registry.potions.has(toCopy)) {
		auto& oldPot = registry.potions.get(toCopy);
		registry.potions.emplace(res, Potion(oldPot));
	}
	if (registry.ammo.has(toCopy)) {
		auto& oldAmmo = registry.ammo.get(toCopy);
		registry.ammo.emplace(res, Ammo(oldAmmo));
	}
	return res;
}

// Serialization
nlohmann::json ItemSystem::serializeItem(Entity item) {
	nlohmann::json data;

	if (!registry.items.has(item)) {
		return data;
	}

	const Item& item_comp = registry.items.get(item);
	data["saved_id"] = item.id();  // Store the Entity ID for reference during deserialization
	data["type_id"] = item_comp.type;
	data["amount"] = item_comp.amount;
	data["is_ammo"] = item_comp.is_ammo;

	// Serialize ingredient data if present
	if (registry.ingredients.has(item)) {
		const Ingredient& ing = registry.ingredients.get(item);
		data["type"] = "ingredient";
		data["ingredient"] = {
			{"grindLevel", ing.grindLevel}
		};
	}

	// Serialize potion data if present
	if (registry.potions.has(item)) {
		const Potion& pot = registry.potions.get(item);
		data["type"] = "potion";
		data["potion"] = {
			{"effect", pot.effect},
			{"duration", pot.duration},
			{"color", {pot.color.x, pot.color.y, pot.color.z}},
			{"quality", pot.quality},
			{"effectValue", pot.effectValue}
		};
	}

	return data;
}

nlohmann::json ItemSystem::serializeInventory(Entity inventory) {
	nlohmann::json data;

	if (!registry.inventories.has(inventory)) {
		return data;
	}

	const Inventory& inv = registry.inventories.get(inventory);
	data["saved_id"] = inventory.id();  // Store the Entity ID for reference
	data["capacity"] = inv.capacity;

	// Store the inventory owner type based on components
	if (registry.cauldrons.has(inventory)) {
		data["owner_type"] = "cauldron";
	}
	else if (registry.chests.has(inventory)) {
		data["owner_type"] = "chest";
	}
	else {
		data["owner_type"] = "player";  // Default to player inventory
	}

	nlohmann::json items_data = nlohmann::json::array();
	for (Entity item : inv.items) {
		if (registry.items.has(item)) {
			items_data.push_back(serializeItem(item));
		}
	}
	data["items"] = items_data;

	return data;
}

nlohmann::json ItemSystem::serializeScreenState() {
	ScreenState& screen = registry.screenStates.components[0];

	nlohmann::json data;

	data["tutorial_state"] = screen.tutorial_state;
	data["biome"] = screen.biome;
	data["from_biome"] = screen.from_biome;
	data["killed_enemies"] = screen.killed_enemies;

	return data;
}

// serializes all player attributes except for inventory
nlohmann::json ItemSystem::serializePlayerState(Entity player_entity) {

	Player& player = registry.players.get(player_entity);
	nlohmann::json data;

	data["name"] = player.name;
	data["cooldown"] = player.cooldown;
	data["health"] = player.health;
	data["damage_cooldown"] = player.damage_cooldown;
	data["consumed_potion"] = player.consumed_potion;
	data["speed_multiplier"] = player.speed_multiplier;
	data["effect_multiplier"] = player.effect_multiplier;
	data["defense"] = player.defense;

	nlohmann::json active_effects = nlohmann::json::array();
	for (Entity effect : player.active_effects) {
		if (registry.items.has(effect) && registry.potions.has(effect)) {
			active_effects.push_back(serializeItem(effect));
		}
	}
	data["active_effects"] = active_effects;

	if (registry.motions.has(player_entity)) {
		data["load_position_x"] = registry.motions.get(player_entity).position.x;
		data["load_position_y"] = registry.motions.get(player_entity).position.y;
	}

	return data;
}

Entity ItemSystem::deserializeItem(const nlohmann::json& data) {
	Entity entity;

	std::string type = data.value("type", "basic");
	if (type == "ingredient") {
		entity = createIngredient(data["type_id"], data["amount"]);
		if (registry.ingredients.has(entity)) {
			Ingredient& ing = registry.ingredients.get(entity);
			auto& ing_data = data["ingredient"];
			ing.grindLevel = ing_data["grindLevel"];
		}
	}
	else if (type == "potion") {
		auto& pot_data = data["potion"];
		entity = createPotion(
			pot_data["effect"],
			pot_data["duration"],
			vec3(pot_data["color"][0], pot_data["color"][1], pot_data["color"][2]),
			pot_data["quality"],
			pot_data["effectValue"],
			data["amount"]
		);
	}
	else {
		entity = createItem(data["type_id"], data["amount"], false, data["is_ammo"]);
	}

	return entity;
}

void ItemSystem::deserializeInventory(Entity inventory, const nlohmann::json& data) {
	if (!registry.inventories.has(inventory)) {
		// std::cout << "Entity " << inventory.id() << " deserialize inventory" << std::endl;
		registry.inventories.emplace(inventory);
	}

	Inventory& inv = registry.inventories.get(inventory);
	inv.capacity = data["capacity"];

	// Create appropriate components based on owner type
	std::string owner_type = data["owner_type"];
	// Don't need this anymore because we call createCauldron
	// if (owner_type == "cauldron" && !registry.cauldrons.has(inventory)) {
	//     registry.cauldrons.emplace(inventory);
	if (owner_type == "chest" && !registry.chests.has(inventory)) {
		registry.chests.emplace(inventory);
	}

	// Clear existing items
	for (Entity item : inv.items) {
		destroyItem(item);
	}
	inv.items.clear();

	// Load new items
	for (const auto& item_data : data["items"]) {
		Entity item = deserializeItem(item_data);
		if (registry.items.has(item)) {
			inv.items.push_back(item);
		}
	}
}

bool ItemSystem::saveGameState() {
	nlohmann::json data;
	nlohmann::json inventories = nlohmann::json::array();

	// First, find and save the player inventory if it exists
	Entity player_inventory;
	bool found_player = false;
	for (Entity inventory : registry.inventories.entities) {
		// Only save player inventory if it belongs to a player entity
		if (registry.players.has(inventory) ||
			(!registry.cauldrons.has(inventory) && !registry.chests.has(inventory))) {
			if (!found_player) {
				inventories.push_back(serializeInventory(inventory));
				found_player = true;
			}
			// Skip any other player inventories
			continue;
		}
		inventories.push_back(serializeInventory(inventory));
	}
	data["inventories"] = inventories;
	data["screen_state"] = serializeScreenState();
	
	// Save the current recipe book index
	if (UISystem::s_instance != nullptr) {
	  data["recipe_book_index"] = UISystem::s_instance->current_recipe_index;
	}

  // Save player state
	if (registry.players.size() > 0) {
		data["player_state"] = serializePlayerState(registry.players.entities[0]);
	}

	try {
		std::string save_path = game_state_path(GAME_STATE_FILE);
		std::ofstream file(save_path);
		// https://json.nlohmann.me/api/basic_json/dump/
		file << data.dump(4);
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to save game state: " << e.what() << std::endl;
		return false;
	}
}

bool ItemSystem::loadGameState() {
	try {
		std::string save_path = game_state_path(GAME_STATE_FILE);
		std::ifstream file(save_path);
		if (!file.is_open()) {
			return false;
		}

		nlohmann::json data;
		file >> data;

		Entity player;
		if (!registry.players.entities.empty()) {
			player = registry.players.entities[0];
		}

		// Load inventories
		for (const auto& inv_data : data["inventories"]) {
			std::string owner_type = inv_data["owner_type"];

			if (owner_type == "player") {
				if (player) {
					deserializeInventory(player, inv_data);
				}
			}
			else if (owner_type == "cauldron") {
				// Leave out cauldron serialization for now
				// if (registry.cauldrons.entities.size() > 1) continue;
				// Entity cauldron_inv = createCauldron(renderer, vec2({ GRID_CELL_WIDTH_PX * 13.35, GRID_CELL_HEIGHT_PX * 5.85 }), vec2({ 175, 280 }), 8, "Cauldron", false);
				// std::cout << "Entity " << cauldron_inv.id() << " cauldron" << std::endl;
				// deserializeInventory(cauldron_inv, inv_data);
			}
			else {
				// For non-player inventories, create new entities
				Entity inv = Entity();
				// std::cout << "Entity " << inv.id() << " item copy" << std::endl;
				deserializeInventory(inv, inv_data);
			}
		}

		if (!data["screen_state"].empty()) {
			deserializeScreenState(data["screen_state"]);
		}

		if (!data["player_state"].empty()) {
			deserializePlayerState(data["player_state"]);
		}
		
		if (data.contains("recipe_book_index") && UISystem::s_instance != nullptr) {
			UISystem::s_instance->current_recipe_index = data["recipe_book_index"];
		}

		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to load game state: " << e.what() << std::endl;
		return false;
	}
}

void ItemSystem::deserializeScreenState(const nlohmann::json& data) {
	ScreenState& screen = registry.screenStates.components[0];

	screen.tutorial_state = data["tutorial_state"];
	screen.switching_to_biome = data["biome"]; // biome switching happens only if switching to biome != biome
	screen.biome = data["from_biome"];
	screen.from_biome = data["from_biome"];
	for (const auto& enemy : data["killed_enemies"]) {
		registry.screenStates.components[0].killed_enemies.push_back(enemy);
	}
}

void ItemSystem::deserializePlayerState(const nlohmann::json& data) {
	if (registry.players.entities.size() == 0) return;
	Entity player_entity = registry.players.entities[0];
	Player& player = registry.players.get(player_entity);

	player.name = data["name"];
	player.cooldown = data["cooldown"];
	player.health = data["health"];
	player.damage_cooldown = data["damage_cooldown"];
	player.consumed_potion = data["consumed_potion"];
	player.speed_multiplier = data["speed_multiplier"];
	player.effect_multiplier = data["effect_multiplier"];
	player.defense = data["defense"];

	for (const auto& effect : data["active_effects"]) {
		player.active_effects.push_back(deserializeItem(effect));
	}

	registry.players.get(player_entity).load_position = vec2(data["load_position_x"], data["load_position_y"]);
}