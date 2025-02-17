#include <gtest/gtest.h>
#include "../src/common.hpp"
#include "../src/item_system.hpp"
#include "../src/tinyECS/registry.hpp"

// Based on googletest docs: http://google.github.io/googletest/reference/testing.html
class ItemSystemTest : public ::testing::Test {
protected:
    ECSRegistry registry;
    ItemSystem item_system;

    ItemSystemTest() : item_system(registry) {
    }

    void SetUp() override {
        // Reset the registry before each test
        registry.clear_all_components();
    }
};

// Test item creation and initialization
TEST_F(ItemSystemTest, ItemCreation) {
    // Test basic item
    Entity basic = ItemSystem::createItem(registry, "Test Item", 1, 5);
    ASSERT_TRUE(registry.items.has(basic));
    Item& item = registry.items.get(basic);
    EXPECT_EQ(item.name, "Test Item");
    EXPECT_EQ(item.type_id, 1);
    EXPECT_EQ(item.amount, 5);
    
    // Test ingredient
    Entity ingredient = ItemSystem::createIngredient(registry, "Test Herb", 2, 3);
    ASSERT_TRUE(registry.items.has(ingredient));
    ASSERT_TRUE(registry.ingredients.has(ingredient));
    Ingredient& ing = registry.ingredients.get(ingredient);
    EXPECT_FLOAT_EQ(ing.grindLevel, 0.0f);
    
    // Test potion
    Entity potion = ItemSystem::createPotion(registry, "Health Potion", 3, 1, 30, vec3(1,0,0), 0.8f);
    ASSERT_TRUE(registry.items.has(potion));
    ASSERT_TRUE(registry.potions.has(potion));
    Potion& pot = registry.potions.get(potion);
    EXPECT_EQ(pot.effect, 1);
    EXPECT_EQ(pot.duration, 30);
    EXPECT_FLOAT_EQ(pot.quality, 0.8f);
}

// Test inventory operations
TEST_F(ItemSystemTest, InventoryOperations) {
    // Create test inventory
    Entity inv = Entity();
    Inventory& inventory = registry.inventories.emplace(inv);
    inventory.capacity = 5;
    
    // Test adding items
    Entity item1 = ItemSystem::createItem(registry, "Item 1", 1, 1);
    Entity item2 = ItemSystem::createItem(registry, "Item 2", 2, 1);
    
    EXPECT_TRUE(item_system.addItemToInventory(inv, item1));
    EXPECT_EQ(inventory.items.size(), 1);
    EXPECT_TRUE(item_system.addItemToInventory(inv, item2));
    EXPECT_EQ(inventory.items.size(), 2);
    
    // Test removing items
    EXPECT_TRUE(item_system.removeItemFromInventory(inv, item1));
    EXPECT_EQ(inventory.items.size(), 1);
    EXPECT_FALSE(item_system.removeItemFromInventory(inv, item1)); // Already removed
    
    // Test stacking
    Entity stackable1 = ItemSystem::createItem(registry, "Stack Item", 3, 5);
    Entity stackable2 = ItemSystem::createItem(registry, "Stack Item", 3, 3);
    
    EXPECT_TRUE(item_system.addItemToInventory(inv, stackable1));
    EXPECT_TRUE(item_system.addItemToInventory(inv, stackable2));
    
    // Should have merged into one stack of 8
    bool found_stack = false;
    for (Entity e : inventory.items) {
        if (registry.items.has(e)) {
            Item& item = registry.items.get(e);
            if (item.type_id == 3 && item.amount == 8) {
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
    
    Entity item = ItemSystem::createItem(registry, "Test Item", 1, 5);
    Entity potion = ItemSystem::createPotion(registry, "Test Potion", 2, 1, 30, vec3(1,0,0), 0.9f);
    Entity ingredient = ItemSystem::createIngredient(registry, "Test Herb", 3, 2);
    
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
                if (loaded_item.type_id == 1) {
                    found_basic = true;
                    EXPECT_EQ(loaded_item.amount, 5);
                }
                if (registry.potions.has(item_entity)) {
                    found_potion = true;
                    Potion& loaded_potion = registry.potions.get(item_entity);
                    EXPECT_EQ(loaded_potion.effect, 1);
                    EXPECT_FLOAT_EQ(loaded_potion.quality, 0.9f);
                }
                if (registry.ingredients.has(item_entity)) {
                    found_ingredient = true;
                    EXPECT_EQ(loaded_item.type_id, 3);
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
    
    Entity item1 = ItemSystem::createItem(registry, "Item 1", 1, 1);
    Entity item2 = ItemSystem::createItem(registry, "Item 2", 2, 1);
    
    EXPECT_TRUE(item_system.addItemToInventory(inv, item1));
    EXPECT_FALSE(item_system.addItemToInventory(inv, item2)); // Should fail, inventory full
    
    // Test invalid serialization operations
    EXPECT_FALSE(item_system.loadGameState("nonexistent_file.json"));
}

// Test entity ID continuity
TEST_F(ItemSystemTest, EntityIDContinuity) {
    // Create and save an item
    Entity item1 = ItemSystem::createItem(registry, "Test Item", 1, 1);
    unsigned int first_id = item1.id();
    
    // Save state
    EXPECT_TRUE(item_system.saveGameState("test_save.json"));
    
    // Clear everything
    registry.clear_all_components();
    
    // Load state and create a new item
    EXPECT_TRUE(item_system.loadGameState("test_save.json"));
    Entity item2 = ItemSystem::createItem(registry, "New Item", 2, 1);
    
    // The new item should have a new unique ID
    EXPECT_NE(item2.id(), first_id);
} 