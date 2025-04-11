#pragma once

#include "common.hpp"
#include <unordered_map>
#include <string>
#include "../tinyECS/tiny_ecs.hpp"
#include "../world_init.hpp"
#include <nlohmann/json.hpp>

struct RespawnState {
    std::string persistentID;
    float respawnCooldownRemaining = 0.0f;
    bool isSpawned = true;
    vec2 originalPosition;
    BIOME biome;
    
    ItemType itemType = ItemType::POTION;
    int itemAmount = 1;
    
    std::string enemyName = "";
    int enemyMovable = 0;
};

class RespawnSystem {
public:
    static RespawnSystem& getInstance() {
        static RespawnSystem instance;
        return instance;
    }
    
    // Update all respawn timers
    void step(float elapsed_ms);
    
    // Register or update an entity's respawn state
    void registerEntity(Entity entity, bool isSpawned);
    
    // Set an entity to respawn after a cooldown
    void setRespawning(const std::string& persistentID, float respawnTime);
    
    // Check if an entity should be spawned
    bool shouldEntitySpawn(const std::string& persistentID);
    
    // Generate a persistent ID for a new entity
    std::string generatePersistentID(BIOME biome, const std::string& entityType, const vec2& position);
    
    // Create entity from respawn state
    Entity spawnEntityFromState(RenderSystem* renderer, const std::string& persistentID);
    
    // Getter for respawn states (needed for world system to check enemy respawns)
    const std::unordered_map<std::string, RespawnState>& getRespawnStates() const {
        return respawnStates;
    }
    
    // Serialize and deserialize for save/load
    nlohmann::json serialize();
    void deserialize(const nlohmann::json& data);
    
    void reset();
    
private:
    RespawnSystem() {}
    
    // Map of persistent IDs to respawn states
    std::unordered_map<std::string, RespawnState> respawnStates;
    
    RenderSystem* renderer = nullptr;
    
    friend class WorldSystem;
}; 