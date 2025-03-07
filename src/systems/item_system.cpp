#include "item_system.hpp"
#include <iostream>
#include <fstream>

Entity ItemSystem::createItem(ItemType type, int amount, bool isCollectable) {
    Entity entity = Entity();
    
    Item& item = registry.items.emplace(entity);
    item.type = type;
    item.amount = amount;
    item.isCollectable = isCollectable;
    item.name = ITEM_NAMES.at(type);
    
    return entity;
}

Entity ItemSystem::createIngredient(ItemType type, int amount) {
    Entity entity = createItem(type, amount, false);
    
    // Add ingredient-specific component
    Ingredient& ingredient = registry.ingredients.emplace(entity);
    ingredient.grindLevel = 0.0f;  // Starts ungrounded
    
    return entity;
}

Entity ItemSystem::createPotion(PotionEffect effect, int duration, const vec3& color, float quality, float effectValue) {
    Entity entity = createItem(ItemType::POTION, 1, false);
    
    // Add potion-specific component
    Potion& potion = registry.potions.emplace(entity);
    potion.effect = effect;
    potion.duration = duration;
    potion.color = color;
    potion.quality = quality;
    potion.effectValue = effectValue;
    
    // Get potion name
    registry.items.get(entity).name = EFFECT_NAMES.at(effect) + " Potion";
    return entity;
}

void ItemSystem::init() {
    // Load persistent data
    loadGameState("game_state.json");
}

void ItemSystem::step(float elapsed_ms) {
    // TODO - Update any time-based item effects here, unused for now
    (void)elapsed_ms;
}

Entity ItemSystem::createItemEntity(ItemType type, int amount) {
    return createItem(type, amount, false);
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
        if (registry.items.has(existing)) {
            Item& existing_item = registry.items.get(existing);
            if (existing_item.type == item_comp.type) {
                // Add amounts together
                existing_item.amount += item_comp.amount;
                if (registry.ammo.has(item) && !registry.ammo.has(existing)) registry.ammo.emplace(existing); // update ammo in case it didn't previous save component
                // Don't destroy the original item if it's a collectable (it will respawn)
                if (!item_comp.isCollectable) {
                    destroyItem(item);
                }
                return true;
            }
        }
    }
    
    // If we couldn't stack, check capacity
    if (inv.items.size() >= inv.capacity) {
        inv.isFull = true;
        return false;
    }
    
    // If item is collectable, create a copy for the inventory
    if (item_comp.isCollectable) {
        Entity copy = createItem(item_comp.type, item_comp.amount, false);
        if (registry.ammo.has(item)) registry.ammo.emplace(copy); // ensure ammo component gets copied over
        inv.items.push_back(copy);
    } else {
        inv.items.push_back(item);
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
    } else if (registry.chests.has(inventory)) {
        data["owner_type"] = "chest";
    } else {
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
    } else if (type == "potion") {
        auto& pot_data = data["potion"];
        entity = createPotion(
            pot_data["effect"],
            pot_data["duration"],
            vec3(pot_data["color"][0], pot_data["color"][1], pot_data["color"][2]),
            pot_data["quality"],
            pot_data["effectValue"]
        );
    } else {
        entity = createItem(data["type_id"], data["amount"], false);
    }

    return entity;
}

void ItemSystem::deserializeInventory(Entity inventory, const nlohmann::json& data) {
    if (!registry.inventories.has(inventory)) {
        inventory = Entity();
        registry.inventories.emplace(inventory);
    }
    
    Inventory& inv = registry.inventories.get(inventory);
    inv.capacity = data["capacity"];
    
    // Create appropriate components based on owner type
    std::string owner_type = data["owner_type"];
    if (owner_type == "cauldron" && !registry.cauldrons.has(inventory)) {
        registry.cauldrons.emplace(inventory);
    } else if (owner_type == "chest" && !registry.chests.has(inventory)) {
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

bool ItemSystem::saveGameState(const std::string& filename) {
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
    
    try {
        std::ofstream file(filename);
        // https://json.nlohmann.me/api/basic_json/dump/
        file << data.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save game state: " << e.what() << std::endl;
        return false;
    }
}

bool ItemSystem::loadGameState(const std::string& filename) {
    try {
        std::ifstream file(filename);
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
            } else {
                // For non-player inventories, create new entities
                Entity inv = Entity();
                deserializeInventory(inv, inv_data);
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load game state: " << e.what() << std::endl;
        return false;
    }
} 