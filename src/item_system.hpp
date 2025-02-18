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
    Entity createItemEntity(const std::string& name, int type_id, int amount = 1);
    void destroyItem(Entity item);
    
    // Inventory management
    bool addItemToInventory(Entity inventory, Entity item);
    bool removeItemFromInventory(Entity inventory, Entity item);
    bool transferItem(Entity source_inventory, Entity target_inventory, Entity item);
    
    // Serialization
    bool saveGameState(const std::string& filename);
    bool loadGameState(const std::string& filename);
    
    // Item factory methods
    static Entity createItem(const std::string& name, int type_id, int amount = 1, bool isCollectable = false);
    static Entity createIngredient(const std::string& name, int type_id, int amount = 1);
    static Entity createPotion(const std::string& name, int type_id, int effect, int duration, const vec3& color, float quality);
    
private:
    // Helper methods for serialization
    nlohmann::json serializeItem(Entity item) const;
    nlohmann::json serializeInventory(Entity inventory) const;
    Entity deserializeItem(const nlohmann::json& data);
    void deserializeInventory(Entity inventory, const nlohmann::json& data);
}; 