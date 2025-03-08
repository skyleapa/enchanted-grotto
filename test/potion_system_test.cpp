#include <gtest/gtest.h>
#include <climits>
#include "../src/common.hpp"
#include "../src/systems/potion_system.hpp"
#include "../src/tinyECS/registry.hpp"

class PotionSystemTest : public ::testing::Test {
protected:
    PotionSystem potion_system;
    Entity cauldron;

    void SetUp() override {
        // Reset the registry and create a new FILLED cauldron before each test
        registry.clear_all_components();
        Cauldron& cc = registry.cauldrons.emplace(cauldron);
        cc.filled = true;
        Inventory& ci = registry.inventories.emplace(cauldron);
        ci.capacity = INT_MAX;
    }

    Entity createIngredient(ItemType type, int amount, float grindLevel) {
        Entity res;
        Item& resItem = registry.items.emplace(res);
        resItem.amount = amount;
        resItem.type = type;
        Ingredient& resIng = registry.ingredients.emplace(res);
        resIng.grindLevel = grindLevel;
        return res;
    }
};

// Test recording of actions and repeat actions
TEST_F(PotionSystemTest, ActionRecording) {
    Cauldron& cc = registry.cauldrons.get(cauldron);
    Inventory& ci = registry.inventories.get(cauldron);
    ASSERT_EQ(cc.actions.size(), 0);
    ASSERT_EQ(ci.items.size(), 0);

    // A cauldron with no actions should not be updated
    potion_system.updateCauldrons(10000);
    EXPECT_EQ(cc.actions.size(), 0);
    EXPECT_EQ(cc.timeElapsed, 0);

    // Now record a heat action
    PotionSystem::changeHeat(cauldron, 100);
    EXPECT_EQ(cc.actions.size(), 1);
    EXPECT_EQ(cc.actions[0].type, ActionType::MODIFY_HEAT);
    EXPECT_EQ(cc.actions[0].value, 100);

    // Then a wait action
    potion_system.updateCauldrons(DEFAULT_WAIT);
    EXPECT_EQ(cc.actions.size(), 2);
    EXPECT_EQ(cc.actions[1].type, ActionType::WAIT);
    EXPECT_EQ(cc.actions[1].value, 1);

    // Then another wait action, which should be concatenated into the last one
    potion_system.updateCauldrons(DEFAULT_WAIT);
    EXPECT_EQ(cc.actions.size(), 2);
    EXPECT_EQ(cc.actions[1].value, 2);

    // Then a stir action, which shouldn't do anything since the cauldron is empty
    PotionSystem::stirCauldron(cauldron, 100);
    EXPECT_EQ(cc.actions.size(), 2);

    // Add an example item
    Entity coffee_bean = createIngredient(ItemType::COFFEE_BEANS, 1, 0.5f);
    PotionSystem::addIngredient(cauldron, coffee_bean);
    EXPECT_EQ(cc.actions.size(), 3);
    EXPECT_EQ(cc.actions[2].type, ActionType::ADD_INGREDIENT);
    EXPECT_EQ(cc.actions[2].value, 0);
    EXPECT_EQ(ci.items.size(), 1);
    EXPECT_EQ(ci.items[0].id(), coffee_bean.id());

    // Add a second example item of the same type
    Entity coffee_bean2 = createIngredient(ItemType::COFFEE_BEANS, 1, 0.5f);
    PotionSystem::addIngredient(cauldron, coffee_bean2);
    EXPECT_EQ(cc.actions.size(), 3);
    EXPECT_EQ(ci.items.size(), 1);
    Item& coffeeItem = registry.items.get(coffee_bean);
    EXPECT_EQ(coffeeItem.amount, 2);
}


// Test bottling a cauldron with no ingredients
TEST_F(PotionSystemTest, DefaultPotion) {
    Cauldron& cc = registry.cauldrons.get(cauldron);
    Potion res = PotionSystem::bottlePotion(cauldron);
    EXPECT_EQ(res.effect, PotionEffect::WATER);
    EXPECT_FALSE(cc.filled);
}

// Test bottling a failed potion
TEST_F(PotionSystemTest, FailedPotion) {
    // Only coffee matches no potion
    Entity coffee_bean = createIngredient(ItemType::COFFEE_BEANS, 1, 0.5f);
    PotionSystem::addIngredient(cauldron, coffee_bean);
    Potion res = PotionSystem::bottlePotion(cauldron);
    EXPECT_EQ(res.effect, PotionEffect::FAILED);
}

// Test bottling a perfectly made speed potion
TEST_F(PotionSystemTest, PerfectSpeedPotion) {
    Entity coffee = createIngredient(ItemType::COFFEE_BEANS, 5, 1.0f);
    Entity fruit = createIngredient(ItemType::MAGICAL_FRUIT, 3, 0.0f);

    // Perfectly made potion, mwah
    PotionSystem::changeHeat(cauldron, 100);
    potion_system.updateCauldrons(DEFAULT_WAIT * 2);
    PotionSystem::addIngredient(cauldron, coffee);
    PotionSystem::addIngredient(cauldron, fruit);
    PotionSystem::stirCauldron(cauldron, 3);
    potion_system.updateCauldrons(DEFAULT_WAIT * 6);

    // Check relevant actions
    Cauldron& cc = registry.cauldrons.get(cauldron);
    EXPECT_EQ(cc.actions.size(), 6);

    // Check final potion, which should be same as in the recipe
    Recipe speedRecipe = RECIPES[0];
    Potion potion = PotionSystem::bottlePotion(cauldron);
    EXPECT_FLOAT_EQ(potion.quality, 1.0f);
    EXPECT_EQ(potion.effect, PotionEffect::SPEED);
    EXPECT_EQ(potion.color, speedRecipe.finalPotionColor);
    EXPECT_EQ(potion.duration, speedRecipe.highestQualityDuration);
    EXPECT_FLOAT_EQ(potion.effectValue, speedRecipe.highestQualityEffect);
}

// Test bottling potion that missed a step
TEST_F(PotionSystemTest, PotionMissOneStep) {
    Entity coffee = createIngredient(ItemType::COFFEE_BEANS, 5, 1.0f);
    Entity fruit = createIngredient(ItemType::MAGICAL_FRUIT, 3, 0.0f);

    PotionSystem::changeHeat(cauldron, 100);
    potion_system.updateCauldrons(DEFAULT_WAIT * 2);
    PotionSystem::addIngredient(cauldron, coffee);
    PotionSystem::addIngredient(cauldron, fruit);
    PotionSystem::stirCauldron(cauldron, 3);
    // potion_system.updateCauldrons(DEFAULT_WAIT * 6); <- Missed this step!
    
    Potion potion = PotionSystem::bottlePotion(cauldron);
    EXPECT_FLOAT_EQ(potion.quality, 5.0f / 6.0f);
}

// Test bottling potion that screwed up ingredient order
TEST_F(PotionSystemTest, PotionIncorrectIngredientOrder) {
    Entity fruit = createIngredient(ItemType::MAGICAL_FRUIT, 3, 0.0f);
    Entity coffee = createIngredient(ItemType::COFFEE_BEANS, 5, 1.0f);
    
    PotionSystem::changeHeat(cauldron, 100);
    potion_system.updateCauldrons(DEFAULT_WAIT * 2);
    PotionSystem::addIngredient(cauldron, fruit);      // <- Swapped!
    PotionSystem::addIngredient(cauldron, coffee);     // <- Swapped!
    PotionSystem::stirCauldron(cauldron, 3);
    potion_system.updateCauldrons(DEFAULT_WAIT * 6);
    
    Potion potion = PotionSystem::bottlePotion(cauldron);
    EXPECT_FLOAT_EQ(potion.quality, (6.0f - 2 * INGREDIENT_TYPE_PENALTY * POTION_DIFFICULTY) / 6.0f);
}

// Test bottling potion that screwed up a stir amount
TEST_F(PotionSystemTest, PotionIncorrectStirAmount) {
    Entity fruit = createIngredient(ItemType::MAGICAL_FRUIT, 3, 0.0f);
    Entity coffee = createIngredient(ItemType::COFFEE_BEANS, 5, 1.0f);

    PotionSystem::changeHeat(cauldron, 100);
    potion_system.updateCauldrons(DEFAULT_WAIT * 2);
    PotionSystem::addIngredient(cauldron, coffee);
    PotionSystem::addIngredient(cauldron, fruit);
    PotionSystem::stirCauldron(cauldron, 4);          // <- 1 extra stir!
    potion_system.updateCauldrons(DEFAULT_WAIT * 6);
    
    Potion potion = PotionSystem::bottlePotion(cauldron);
    EXPECT_FLOAT_EQ(potion.quality, (6.0f - STIR_PENALTY * POTION_DIFFICULTY) / 6.0f);
}