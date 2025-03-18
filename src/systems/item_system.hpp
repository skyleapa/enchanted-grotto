#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include "render_system.hpp"
#include <string>
#include <fstream>
#include "nlohmann/json.hpp"

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
    static Entity createPotion(PotionEffect effect, int duration, const vec3& color, float quality, float effectValue);
    static Entity createCollectableIngredient(vec2 position, ItemType type, int amount, bool canRespawn);
    
    // Serialization helpers made public and static
    static nlohmann::json serializeItem(Entity item);
    static nlohmann::json serializeInventory(Entity inventory);
    static nlohmann::json serializeScreenState();
    static Entity deserializeItem(const nlohmann::json& data);
    static void deserializeInventory(Entity inventory, const nlohmann::json& data);
    static void deserializeScreenState(const nlohmann::json& data);
}; 