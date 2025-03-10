#include <gtest/gtest.h>
#include "../src/common.hpp"
#include "../src/systems/item_system.hpp"
#include "../src/tinyECS/registry.hpp"

// Based on googletest docs: http://google.github.io/googletest/reference/testing.html
class ItemSystemTest : public ::testing::Test {
protected:
    ItemSystem item_system;

    void SetUp() override {
        // Reset the registry before each test
        registry.clear_all_components();
    }
};

// Test item creation and initialization
TEST_F(ItemSystemTest, ItemCreation) {
    // Test basic item
    Entity basic = ItemSystem::createItem(ItemType::COFFEE_BEANS, 5);
    ASSERT_TRUE(registry.items.has(basic));
    Item& item = registry.items.get(basic);
    EXPECT_EQ(item.type, ItemType::COFFEE_BEANS);
    EXPECT_EQ(item.amount, 5);
    EXPECT_FALSE(item.isCollectable);
    
    // Test basic item with explicit isCollectable = true
    Entity collectible = ItemSystem::createItem(ItemType::COFFEE_BEANS, 1, true);
    ASSERT_TRUE(registry.items.has(collectible));
    Item& coll_item = registry.items.get(collectible);
    EXPECT_TRUE(coll_item.isCollectable);
    
    // Test ingredient
    Entity ingredient = ItemSystem::createIngredient(ItemType::MAGICAL_FRUIT, 3);
    ASSERT_TRUE(registry.items.has(ingredient));
    ASSERT_TRUE(registry.ingredients.has(ingredient));
    Ingredient& ing = registry.ingredients.get(ingredient);
    EXPECT_FLOAT_EQ(ing.grindLevel, 0.0f);
    EXPECT_FALSE(registry.items.get(ingredient).isCollectable);
    
    // Test potion
    Entity potion = ItemSystem::createPotion(PotionEffect::SPEED, 30, vec3(1,0,0), 0.8f, 3.0f);
    ASSERT_TRUE(registry.items.has(potion));
    ASSERT_TRUE(registry.potions.has(potion));
    Potion& pot = registry.potions.get(potion);
    EXPECT_EQ(pot.effect, PotionEffect::SPEED);
    EXPECT_EQ(pot.duration, 30);
    EXPECT_FLOAT_EQ(pot.quality, 0.8f);
    EXPECT_FLOAT_EQ(pot.effectValue, 3.0f);
    EXPECT_FALSE(registry.items.get(potion).isCollectable);
}

// Test inventory operations
TEST_F(ItemSystemTest, InventoryOperations) {
    // Create test inventory
    Entity inv = Entity();
    Inventory& inventory = registry.inventories.emplace(inv);
    inventory.capacity = 5;
    
    // Test adding items
    Entity item1 = ItemSystem::createItem(ItemType::COFFEE_BEANS, 1);
    Entity item2 = ItemSystem::createItem(ItemType::MAGICAL_FRUIT, 1);
    
    EXPECT_TRUE(item_system.addItemToInventory(inv, item1));
    EXPECT_EQ(inventory.items.size(), 1);
    EXPECT_TRUE(item_system.addItemToInventory(inv, item2));
    EXPECT_EQ(inventory.items.size(), 2);
    
    // Test removing items
    EXPECT_TRUE(item_system.removeItemFromInventory(inv, item1));
    EXPECT_EQ(inventory.items.size(), 1);
    EXPECT_FALSE(item_system.removeItemFromInventory(inv, item1)); // Already removed
    
    // Test stacking
    Entity stackable1 = ItemSystem::createItem(ItemType::COFFEE_BEANS, 5);
    Entity stackable2 = ItemSystem::createItem(ItemType::COFFEE_BEANS, 3);
    
    EXPECT_TRUE(item_system.addItemToInventory(inv, stackable1));
    EXPECT_TRUE(item_system.addItemToInventory(inv, stackable2));
    
    // Should have merged into one stack of 8
    bool found_stack = false;
    for (Entity e : inventory.items) {
        if (registry.items.has(e)) {
            Item& item = registry.items.get(e);
            if (item.type == ItemType::COFFEE_BEANS && item.amount == 8) {
                found_stack = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found_stack);
}

// Test serialization and persistence
TEST_F(ItemSystemTest, Serialization) {
    // Create test data
    Entity inv = Entity();
    Inventory& inventory = registry.inventories.emplace(inv);
    inventory.capacity = 10;
    
    Entity item = ItemSystem::createItem(ItemType::COFFEE_BEANS, 5);
    Entity potion = ItemSystem::createPotion(PotionEffect::SPEED, 30, vec3(1,0,0), 0.9f, 3.0f);
    Entity ingredient = ItemSystem::createIngredient(ItemType::MAGICAL_FRUIT, 2);
    
    EXPECT_TRUE(item_system.addItemToInventory(inv, item));
    EXPECT_TRUE(item_system.addItemToInventory(inv, potion));
    EXPECT_TRUE(item_system.addItemToInventory(inv, ingredient));
    
    // Save state
    EXPECT_TRUE(item_system.saveGameState("test_save.json"));
    
    // Clear everything
    registry.clear_all_components();
    
    // Load state
    EXPECT_TRUE(item_system.loadGameState("test_save.json"));
    
    // Verify loaded data
    bool found_inventory = false;
    for (Entity e : registry.inventories.entities) {
        Inventory& loaded_inv = registry.inventories.get(e);
        EXPECT_EQ(loaded_inv.capacity, 10);
        EXPECT_EQ(loaded_inv.items.size(), 3);
        
        // Verify items were restored correctly
        bool found_basic = false, found_potion = false, found_ingredient = false;
        for (Entity item_entity : loaded_inv.items) {
            if (registry.items.has(item_entity)) {
                Item& loaded_item = registry.items.get(item_entity);
                if (loaded_item.type == ItemType::COFFEE_BEANS) {
                    found_basic = true;
                    EXPECT_EQ(loaded_item.amount, 5);
                }
                if (registry.potions.has(item_entity)) {
                    found_potion = true;
                    Potion& loaded_potion = registry.potions.get(item_entity);
                    EXPECT_EQ(loaded_potion.effect, PotionEffect::SPEED);
                    EXPECT_FLOAT_EQ(loaded_potion.quality, 0.9f);
                    EXPECT_FLOAT_EQ(loaded_potion.effectValue, 3.0f);
                }
                if (registry.ingredients.has(item_entity)) {
                    found_ingredient = true;
                    EXPECT_EQ(loaded_item.type, ItemType::MAGICAL_FRUIT);
                }
            }
        }
        EXPECT_TRUE(found_basic && found_potion && found_ingredient);
        found_inventory = true;
        break;
    }
    EXPECT_TRUE(found_inventory);
}

// Test error handling
TEST_F(ItemSystemTest, ErrorHandling) {
    // Test invalid inventory operations
    Entity invalid_inv = Entity();
    Entity invalid_item = Entity();
    EXPECT_FALSE(item_system.addItemToInventory(invalid_inv, invalid_item));
    EXPECT_FALSE(item_system.removeItemFromInventory(invalid_inv, invalid_item));
    
    // Test inventory capacity limits
    Entity inv = Entity();
    Inventory& inventory = registry.inventories.emplace(inv);
    inventory.capacity = 1;
    
    Entity item1 = ItemSystem::createItem(ItemType::COFFEE_BEANS, 1);
    Entity item2 = ItemSystem::createItem(ItemType::MAGICAL_FRUIT, 1);
    
    EXPECT_TRUE(item_system.addItemToInventory(inv, item1));
    EXPECT_FALSE(item_system.addItemToInventory(inv, item2)); // Should fail, inventory full
    
    // Test invalid serialization operations
    EXPECT_FALSE(item_system.loadGameState("nonexistent_file.json"));
}

// Test entity ID continuity
TEST_F(ItemSystemTest, EntityIDContinuity) {
    // Create and save an item
    Entity item1 = ItemSystem::createItem(ItemType::COFFEE_BEANS, 1);
    unsigned int first_id = item1.id();
    
    // Save state
    EXPECT_TRUE(item_system.saveGameState("test_save.json"));
    
    // Clear everything
    registry.clear_all_components();
    
    // Load state and create a new item
    EXPECT_TRUE(item_system.loadGameState("test_save.json"));
    Entity item2 = ItemSystem::createItem(ItemType::MAGICAL_FRUIT, 1);
    
    // The new item should have a new unique ID
    EXPECT_NE(item2.id(), first_id);
}

// Test different inventory types serialization
TEST_F(ItemSystemTest, DifferentInventoryTypes) {
    // Create player inventory
    Entity player_inv = Entity();
    Inventory& player_inventory = registry.inventories.emplace(player_inv);
    player_inventory.capacity = 10;
    Entity player_item = ItemSystem::createItem(ItemType::COFFEE_BEANS, 5);
    EXPECT_TRUE(item_system.addItemToInventory(player_inv, player_item));
    
    // Create cauldron inventory
    Entity cauldron_inv = Entity();
    Inventory& cauldron_inventory = registry.inventories.emplace(cauldron_inv);
    registry.cauldrons.emplace(cauldron_inv);  // Add cauldron component
    cauldron_inventory.capacity = 5;
    Entity cauldron_item = ItemSystem::createIngredient(ItemType::MAGICAL_FRUIT, 2);
    EXPECT_TRUE(item_system.addItemToInventory(cauldron_inv, cauldron_item));
    
    // Create chest inventory
    Entity chest_inv = Entity();
    Inventory& chest_inventory = registry.inventories.emplace(chest_inv);
    registry.chests.emplace(chest_inv);  // Add chest component
    chest_inventory.capacity = 15;
    Entity chest_item = ItemSystem::createPotion(PotionEffect::SPEED, 30, vec3(1,0,0), 0.8f, 3.0f);
    EXPECT_TRUE(item_system.addItemToInventory(chest_inv, chest_item));
    
    // Save state
    EXPECT_TRUE(item_system.saveGameState("test_save.json"));
    
    // Clear everything
    registry.clear_all_components();
    
    // Load state
    EXPECT_TRUE(item_system.loadGameState("test_save.json"));
    
    // Verify all inventory types were restored correctly
    bool found_player = false, found_cauldron = false, found_chest = false;
    
    for (Entity e : registry.inventories.entities) {
        Inventory& inv = registry.inventories.get(e);
        
        if (!registry.cauldrons.has(e) && !registry.chests.has(e)) {
            // Player inventory
            found_player = true;
            EXPECT_EQ(inv.capacity, 10);
            EXPECT_EQ(inv.items.size(), 1);
            EXPECT_TRUE(registry.items.has(inv.items[0]));
            EXPECT_EQ(registry.items.get(inv.items[0]).type, ItemType::COFFEE_BEANS);
        } else if (registry.cauldrons.has(e)) {
            // Cauldron inventory
            found_cauldron = true;
            EXPECT_EQ(inv.capacity, 5);
            EXPECT_EQ(inv.items.size(), 1);
            EXPECT_TRUE(registry.items.has(inv.items[0]));
            EXPECT_TRUE(registry.ingredients.has(inv.items[0]));
            EXPECT_EQ(registry.items.get(inv.items[0]).type, ItemType::MAGICAL_FRUIT);
        } else if (registry.chests.has(e)) {
            // Chest inventory
            found_chest = true;
            EXPECT_EQ(inv.capacity, 15);
            EXPECT_EQ(inv.items.size(), 1);
            EXPECT_TRUE(registry.items.has(inv.items[0]));
            EXPECT_TRUE(registry.potions.has(inv.items[0]));
            Potion& pot = registry.potions.get(inv.items[0]);
            EXPECT_EQ(pot.effect, PotionEffect::SPEED);
            EXPECT_FLOAT_EQ(pot.quality, 0.8f);
        }
    }
    
    EXPECT_TRUE(found_player && found_cauldron && found_chest);
}