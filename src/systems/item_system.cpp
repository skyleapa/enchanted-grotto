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
		int lvl = (int)(registry.ingredients.get(item).grindLevel * 100);
		if (lvl > 0) {
			name += " (" + std::to_string(lvl) + "% Grinded)";
		}
	}

	// Add potion name for potions
	if (registry.potions.has(item)) {
		Potion& potion = registry.potions.get(item);
		if (potion.effect == PotionEffect::WATER) {
			return "Flask of Holy Water";
		}
		else if (potion.effect == PotionEffect::FAILED) {
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

	//std::cout << "Added new item: " << item_comp.name << " to inventory." << std::endl;

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
		// update bar if inventory belongs to player
		if (registry.players.has(inventory) && m_ui_system != nullptr) {
			m_ui_system->updateInventoryBar();
		}
		return true;
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
	registry.inventories.get(inventory).selection = slot1;
}

Entity ItemSystem::copyItem(Entity toCopy) {
	Entity res = Entity();
	auto& oldItem = registry.items.get(toCopy);

	Item& newItem = registry.items.emplace(res);
	newItem.type = oldItem.type;
	newItem.name = oldItem.name;
	newItem.isCollectable = oldItem.isCollectable;
	newItem.amount = oldItem.amount;
	newItem.respawnTime = oldItem.respawnTime;
	newItem.originalPosition = oldItem.originalPosition;
	newItem.is_ammo = oldItem.is_ammo;
	newItem.canRespawn = oldItem.canRespawn;
	newItem.lastBiome = oldItem.lastBiome;

	//registry.items.emplace(res, newItem);

	if (registry.ingredients.has(toCopy)) {
		//std::cout << "Entity " << oldIng.grindLevel << " item copy" << std::endl;
		auto& oldIng = registry.ingredients.get(toCopy);
		// registry.ingredients.emplace(res, Ingredient(oldIng));
		Ingredient& newIng = registry.ingredients.emplace(res);
		newIng.grindLevel = oldIng.grindLevel;
		// std::cout << "Entity " << oldIng.grindLevel << " item copy" << std::endl;
	}
	if (registry.potions.has(toCopy)) {
		auto& oldPot = registry.potions.get(toCopy);
		// registry.potions.emplace(res, Potion(oldPot));
		Potion& newPot = registry.potions.emplace(res);
		newPot.effect = oldPot.effect;
		newPot.duration = oldPot.duration;
		newPot.effectValue = oldPot.effectValue;
		newPot.quality = oldPot.quality;
		newPot.color = oldPot.color;
	}
	if (registry.ammo.has(toCopy)) {
		auto& oldAmmo = registry.ammo.get(toCopy);
		// registry.ammo.emplace(res, Ammo(oldAmmo));
		Ammo& newAmmo = registry.ammo.emplace(res);
		newAmmo.start_pos = oldAmmo.start_pos;
		newAmmo.target = oldAmmo.target;
		newAmmo.is_fired = oldAmmo.is_fired;
		newAmmo.damage = oldAmmo.damage;
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
	data["unlocked_biomes"] = screen.unlocked_biomes;

	return data;
}

// serializes all player attributes except for inventory
nlohmann::json ItemSystem::serializePlayerState(Entity player) {
	nlohmann::json data;
	
	if (registry.players.has(player)) {
		Player& player_comp = registry.players.get(player);
		data["name"] = player_comp.name;
		data["health"] = player_comp.health;
		data["cooldown"] = player_comp.cooldown;
		data["damage_cooldown"] = player_comp.damage_cooldown;
		data["consumed_potion"] = player_comp.consumed_potion;
		data["speed_multiplier"] = player_comp.speed_multiplier;
		data["effect_multiplier"] = player_comp.effect_multiplier;
		data["defense"] = player_comp.defense;
		
		nlohmann::json active_effects = nlohmann::json::array();
		for (Entity effect : player_comp.active_effects) {
			active_effects.push_back(serializeItem(effect));
		}
		data["active_effects"] = active_effects;

		if (registry.motions.has(player)) {
			Motion& motion = registry.motions.get(player);
			data["load_position_x"] = motion.position.x;
			data["load_position_y"] = motion.position.y;
		}
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

void ItemSystem::deserializePlayerState(Entity player_entity, const nlohmann::json& data) {
	if (!registry.players.has(player_entity)) return;
	
	Player& player = registry.players.get(player_entity);
	player.name = data.value("name", "Player");
	player.health = data.value("health", PLAYER_MAX_HEALTH);
	player.cooldown = data.value("cooldown", 0.0f);
	player.damage_cooldown = data.value("damage_cooldown", 0.0f);
	player.consumed_potion = data.value("consumed_potion", false);
	player.speed_multiplier = data.value("speed_multiplier", 1.0f);
	player.effect_multiplier = data.value("effect_multiplier", 1.0f);
	player.defense = data.value("defense", 1.0f);
	
	player.active_effects.clear();
	
	for (const auto& effect : data["active_effects"]) {
		player.active_effects.push_back(deserializeItem(effect));
	}

	registry.players.get(player_entity).load_position = vec2(data["load_position_x"], data["load_position_y"]);
}

bool ItemSystem::saveGameState() {
	std::cout << "Saving game state..." << std::endl;
	
	nlohmann::json data;
	
	nlohmann::json inventories = nlohmann::json::array();
	
	if (!registry.cauldrons.entities.empty()) {
		Entity cauldron = registry.cauldrons.entities[0];
		if (registry.inventories.has(cauldron)) {
			inventories.push_back(serializeInventory(cauldron));
		}
	}
	
	if (!registry.players.entities.empty()) {
		Entity player = registry.players.entities[0];
		if (registry.inventories.has(player)) {
			inventories.push_back(serializeInventory(player));
		}
	}
	
	for (Entity chest : registry.chests.entities) {
		if (registry.inventories.has(chest)) {
			inventories.push_back(serializeInventory(chest));
		}
	}
	
	data["inventories"] = inventories;
	data["screen_state"] = serializeScreenState();
	
	if (UISystem::s_instance != nullptr) {
		data["recipe_book_index"] = UISystem::s_instance->current_recipe_index;
	}
	
	if (registry.players.size() > 0) {
		data["player_state"] = serializePlayerState(registry.players.entities[0]);
	}
	
	// Save respawn system state (items, enemies)
	data["respawn_states"] = RespawnSystem::getInstance().serialize();

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
				if (!registry.cauldrons.entities.empty()) {
					deserializeInventory(registry.cauldrons.entities[0], inv_data);
				}
			}
			else if (owner_type == "chest") {
				Entity chest;
				if (inv_data.contains("name")) {
					std::string chest_name = inv_data["name"];
					
					for (Entity existing_chest : registry.chests.entities) {
						// TODO: chest PR
					}
				}
				
				// TODO: chest PR
				if (!registry.chests.entities.empty()) {
					deserializeInventory(registry.chests.entities[0], inv_data);
				}
			}
		}

		deserializeScreenState(data["screen_state"]);
		
		// Load recipe book index
		if (data.contains("recipe_book_index") && UISystem::s_instance != nullptr) {
			UISystem::s_instance->current_recipe_index = data["recipe_book_index"];
		}
		
		// Load player state
		if (data.contains("player_state") && !registry.players.entities.empty()) {
			deserializePlayerState(registry.players.entities[0], data["player_state"]);
		}
		
		// Load respawn system state
		if (data.contains("respawn_states")) {
			RespawnSystem::getInstance().deserialize(data["respawn_states"]);
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
	for (const auto& biome : data["unlocked_biomes"]) {
		registry.screenStates.components[0].unlocked_biomes.push_back(biome);
	}
}