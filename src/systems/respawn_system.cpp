#include "respawn_system.hpp"
#include "sound_system.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>

std::string RespawnSystem::generatePersistentID(BIOME biome, const std::string& entityType, const vec2& position) {
    std::stringstream ss;
    ss << static_cast<int>(biome) << "_" << entityType << "_";
    ss << std::fixed << std::setprecision(0) << position.x << "_" << position.y;
    return ss.str();
}

void RespawnSystem::registerEntity(Entity entity, bool isSpawned) {
    if (registry.items.has(entity)) {
        Item& item = registry.items.get(entity);
        if (!item.isCollectable || !item.canRespawn) {
            return; // Skip non-collectable or non-respawnable items
        }
    } else if (!registry.enemies.has(entity)) {
        return; // Only track items and enemies
    }
    
    std::string persistentID;
    RespawnState state;
    
    if (registry.items.has(entity)) {
        Item& item = registry.items.get(entity);
        if (item.persistentID.empty()) {
            std::string entityType;
            auto it = ITEM_INFO.find(item.type);
            if (it != ITEM_INFO.end()) {
                entityType = it->second.name;
            } else {
                entityType = "Unknown_" + std::to_string(static_cast<int>(item.type));
            }
            
            // Get current biome consistently for both ID and state
            BIOME currentBiome = static_cast<BIOME>(registry.screenStates.components[0].biome);
            item.persistentID = generatePersistentID(currentBiome, entityType, item.originalPosition);
            
            std::string prefix = item.persistentID.substr(0, item.persistentID.find('_'));
            int biomeFromID = std::stoi(prefix);
            state.biome = static_cast<BIOME>(biomeFromID);
        } else {
            std::string prefix = item.persistentID.substr(0, item.persistentID.find('_'));
            int biomeFromID = std::stoi(prefix);
            state.biome = static_cast<BIOME>(biomeFromID);
        }
        
        persistentID = item.persistentID;
        
        if (respawnStates.count(persistentID) > 0) {
            return;
        }
        
        state.persistentID = persistentID;
        state.isSpawned = isSpawned;
        state.originalPosition = item.originalPosition;
        state.itemType = item.type;
        state.itemAmount = item.amount;
    }
    else if (registry.enemies.has(entity)) {
        Enemy& enemy = registry.enemies.get(entity);
        if (enemy.persistentID.empty()) {
            BIOME currentBiome = static_cast<BIOME>(registry.screenStates.components[0].biome);
            enemy.persistentID = generatePersistentID(
                currentBiome, 
                enemy.name, 
                enemy.start_pos
            );
            
            std::string prefix = enemy.persistentID.substr(0, enemy.persistentID.find('_'));
            int biomeFromID = std::stoi(prefix);
            state.biome = static_cast<BIOME>(biomeFromID);
        } else {
            std::string prefix = enemy.persistentID.substr(0, enemy.persistentID.find('_'));
            int biomeFromID = std::stoi(prefix);
            state.biome = static_cast<BIOME>(biomeFromID);
        }
        
        persistentID = enemy.persistentID;
        
        if (respawnStates.count(persistentID) > 0) {
            return;
        }
        
        state.persistentID = persistentID;
        state.isSpawned = isSpawned;
        state.originalPosition = enemy.start_pos;
        state.enemyName = enemy.name;
        state.enemyMovable = enemy.can_move;
    }
    
    respawnStates[persistentID] = state;
}

void RespawnSystem::setRespawning(const std::string& persistentID, float respawnTime) {
    if (respawnStates.count(persistentID) > 0) {
        respawnStates[persistentID].isSpawned = false;
        respawnStates[persistentID].respawnCooldownRemaining = respawnTime;
        // std::cout << "Setting respawn timer for entity " << persistentID << " to " << respawnTime << "ms" << std::endl;
    }
}

bool RespawnSystem::shouldEntitySpawn(const std::string& persistentID) {
    if (respawnStates.count(persistentID) > 0) {
        RespawnState& state = respawnStates[persistentID];
        
        return state.isSpawned || state.respawnCooldownRemaining <= 0;
    }
    
    // Default to spawning if not tracked yet
    return true;
}

void RespawnSystem::step(float elapsed_ms) {
    if (!renderer) return;
    
    GLuint currentBiome = registry.screenStates.components[0].biome;
    
    for (auto& [id, state] : respawnStates) {
        // Skip entities that are already spawned
        if (state.isSpawned) continue;
        
        if (state.respawnCooldownRemaining > 0) {
            state.respawnCooldownRemaining -= elapsed_ms;
            
            // Print timer debug every 5 seconds for some entities
            // if (fmod(state.respawnCooldownRemaining, 5000.0f) < elapsed_ms) {
            //     std::cout << "Entity " << id << " respawn time remaining: " 
            //               << state.respawnCooldownRemaining / 1000.0f << " seconds." << std::endl;
            // }
            
            // Check if cooldown is complete
            if (state.respawnCooldownRemaining <= 0) {
                state.isSpawned = true;
                
                std::cout << "Timer finished for entity " << id << " in biome " 
                          << static_cast<int>(state.biome) << std::endl;
                
                // Only spawn entities in the current biome
                if (static_cast<GLuint>(state.biome) == currentBiome) { 
                    std::cout << "Creating entity now because player is in the correct biome" << std::endl;
                    Entity entity = spawnEntityFromState(renderer, id);
                    
                    if (entity) {
                        std::cout << "Successfully created entity with ID " << entity.id() << std::endl;
                        
                        if (registry.motions.has(entity)) {
                            registry.motions.get(entity).angle = 180.f;
                        }
                    } else {
                        std::cout << "Failed to create entity!" << std::endl;
                    }
                } else {
                    std::cout << "Player is in biome " << currentBiome 
                              << ", entity will spawn when player enters biome " 
                              << static_cast<int>(state.biome) << std::endl;
                }
            }
        }
    }
}

Entity RespawnSystem::spawnEntityFromState(RenderSystem* renderer, const std::string& persistentID) {
    if (respawnStates.count(persistentID) == 0) {
        return Entity(); // Invalid entity
    }
    
    RespawnState& state = respawnStates[persistentID];
    Entity entity;
    
    // Spawn item
    if (state.itemType != ItemType::POTION) {
        entity = createCollectableIngredient(renderer, state.originalPosition, state.itemType, state.itemAmount, true);
        
        if (registry.items.has(entity) && registry.motions.has(entity)) {
            registry.items.get(entity).persistentID = persistentID;
            registry.motions.get(entity).angle = 180.f;
        }
    }
    else if (!state.enemyName.empty()) {
        if (state.enemyName.find("Ent") != std::string::npos) {
            entity = createEnt(renderer, state.originalPosition, state.enemyMovable, state.enemyName);
        } 
        else if (state.enemyName.find("Mummy") != std::string::npos) {
            entity = createMummy(renderer, state.originalPosition, state.enemyMovable, state.enemyName);
        }
        else if (state.enemyName.find("Bug") != std::string::npos) {
            entity = createCrystalBug(renderer, state.originalPosition, state.enemyMovable, state.enemyName);
        }
        else if (state.enemyName.find("Evil Mushroom") != std::string::npos) {
            entity = createEvilMushroom(renderer, state.originalPosition, state.enemyMovable, state.enemyName);
        }
        
        // Set the persistent ID
        if (registry.enemies.has(entity)) {
            registry.enemies.get(entity).persistentID = persistentID;
        }
    }
    
    return entity;
}

nlohmann::json RespawnSystem::serialize() {
    nlohmann::json data = nlohmann::json::array();
    
    for (const auto& [id, state] : respawnStates) {
        nlohmann::json stateData;
        stateData["persistentID"] = state.persistentID;
        stateData["respawnCooldownRemaining"] = state.respawnCooldownRemaining;
        stateData["isSpawned"] = state.isSpawned;
        stateData["position_x"] = state.originalPosition.x;
        stateData["position_y"] = state.originalPosition.y;
        stateData["biome"] = static_cast<int>(state.biome);
        
        // Item-specific data
        if (state.itemType != ItemType::POTION) {
            stateData["entityType"] = "item";
            stateData["itemType"] = static_cast<int>(state.itemType);
            stateData["itemAmount"] = state.itemAmount;
        }
        // Enemy-specific data
        else if (!state.enemyName.empty()) {
            stateData["entityType"] = "enemy";
            stateData["enemyName"] = state.enemyName;
            stateData["enemyMovable"] = state.enemyMovable;
        }
        
        data.push_back(stateData);
    }
    
    return data;
}

void RespawnSystem::deserialize(const nlohmann::json& data) {
    if (!data.is_array()) {
        return;
    }
    
    respawnStates.clear();
    
    for (const auto& stateData : data) {
        RespawnState state;
        state.persistentID = stateData["persistentID"];
        state.respawnCooldownRemaining = stateData["respawnCooldownRemaining"];
        state.isSpawned = stateData["isSpawned"];
        state.originalPosition = vec2(stateData["position_x"], stateData["position_y"]);

        std::string persistentID = state.persistentID;
        if (!persistentID.empty() && persistentID.find('_') != std::string::npos) {
            std::string prefix = persistentID.substr(0, persistentID.find('_'));
            try {
                int biomeFromID = std::stoi(prefix);
                state.biome = static_cast<BIOME>(biomeFromID);
                std::cout << "Fixed biome for " << persistentID << ": using biome " << biomeFromID 
                          << " from ID instead of saved biome " << static_cast<int>(stateData["biome"]) << std::endl;
            } catch (const std::exception& e) {
                state.biome = static_cast<BIOME>(stateData["biome"]);
                std::cout << "Failed to parse biome from ID " << persistentID << ", using saved biome" << std::endl;
            }
        } else {
            state.biome = static_cast<BIOME>(stateData["biome"]);
        }
        
        std::string entityType = stateData["entityType"];
        if (entityType == "item") {
            state.itemType = static_cast<ItemType>(stateData["itemType"]);
            state.itemAmount = stateData["itemAmount"];
        }
        else if (entityType == "enemy") {
            state.enemyName = stateData["enemyName"];
            state.enemyMovable = stateData["enemyMovable"];
        }
        
        respawnStates[state.persistentID] = state;
    }
}

void RespawnSystem::reset() {
    respawnStates.clear();
} 