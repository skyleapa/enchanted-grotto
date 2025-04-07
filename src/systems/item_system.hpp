#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "render_system.hpp"
#include <string>
#include <fstream>
#include "nlohmann/json.hpp"
#include "respawn_system.hpp"
#include "ui_system.hpp"

// Main system for managing items and inventories
class ItemSystem {
public:
	ItemSystem() {}
	
	void step(float elapsed_ms);
	
	// Item management
	Entity createItemEntity(ItemType type, int amount = 1);
	static void destroyItem(Entity item);
	
	// Inventory management
	static bool addItemToInventory(Entity inventory, Entity item);
	static bool removeItemFromInventory(Entity inventory, Entity item);
	bool transferItem(Entity source_inventory, Entity target_inventory, Entity item);
	static void swapItems(Entity inventory, int slot1, int slot2);

	// Creates an exact copy of an item thats stored in a new entity
	static Entity copyItem(Entity toCopy);
	
	// Serialization
	static bool saveGameState();
	static bool loadGameState();
	
	// Item factory methods
	static Entity createItem(ItemType type, int amount = 1, bool isCollectable = false, bool is_ammo = false, bool canRespawn = true);
	static Entity createIngredient(ItemType type, int amount = 1);
	static Entity createPotion(PotionEffect effect, int duration, const vec3& color, float quality, float effectValue, int amount);
	static Entity createCollectableIngredient(vec2 position, ItemType type, int amount, bool canRespawn = true);
	static std::string getItemName(Entity item);
	
	// Serialization helpers made public and static
	static nlohmann::json serializeItem(Entity item);
	static nlohmann::json serializeInventory(Entity inventory);
	static nlohmann::json serializeScreenState();
	static nlohmann::json serializePlayerState(Entity player_entity);
	static Entity deserializeItem(const nlohmann::json& data);
	static void deserializeInventory(Entity inventory, const nlohmann::json& data);
	static void deserializeScreenState(const nlohmann::json& data);
	static void deserializePlayerState(Entity player_entity, const nlohmann::json& data);

	// Set reference to UI system
	static void setUISystem(UISystem* ui_system) { m_ui_system = ui_system; }

	// Split loading into two phases to handle biome initialization timing for the CHEST :(
	static nlohmann::json loadCoreState(); // Load screen state & other core data, returns the parsed JSON
	static void loadInventoryState(const nlohmann::json& data); // Load inventory data after biomes initialized

private:
	// Pointer to the UI system for passing input events
	static UISystem* m_ui_system;
}; 