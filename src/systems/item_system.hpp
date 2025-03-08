#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include <string>
#include <fstream>
#include "nlohmann/json.hpp"

// Main system for managing items and inventories
class ItemSystem {
public:
    ItemSystem() {}
    
    void init();
    void step(float elapsed_ms);
    
    // Item management
    Entity createItemEntity(ItemType type, int amount = 1);
    static void destroyItem(Entity item);
    
    // Inventory management
    static bool addItemToInventory(Entity inventory, Entity item);
    bool removeItemFromInventory(Entity inventory, Entity item);
    bool transferItem(Entity source_inventory, Entity target_inventory, Entity item);
    
    // Serialization
    bool saveGameState(const std::string& filename);
    bool loadGameState(const std::string& filename);
    
    // Item factory methods
    static Entity createItem(ItemType type, int amount = 1, bool isCollectable = false);
    static Entity createIngredient(ItemType type, int amount = 1);
    static Entity createPotion(PotionEffect effect, int duration, const vec3& color, float quality, float effectValue);
    
    // Serialization helpers made public and static
    static nlohmann::json serializeItem(Entity item);
    static nlohmann::json serializeInventory(Entity inventory);
    static Entity deserializeItem(const nlohmann::json& data);
    static void deserializeInventory(Entity inventory, const nlohmann::json& data);
    
private:
    // Helper methods for serialization have been moved to public
}; 