#pragma once

// OpenGL includes in correct order
#include <gl3w/glcorearb.h>  // Core OpenGL types
#include <gl3w/gl3w.h>       // OpenGL loader
#include <GLFW/glfw3.h>      // GLFW

// RmlUi include
#include <RmlUi/Core.h>
#include "../tinyECS/components.hpp"

// Registry
#include "../tinyECS/registry.hpp"
#include "potion_system.hpp"

#include <optional>

// Forward declarations
class RenderSystem;

struct TextQueueItem {
    std::string text;
    float displayDuration; // in seconds
    float elapsedTime; // in seconds, tracks how long the text has been displayed
};

// Main UI system class
class UISystem {
public:
    UISystem();
    ~UISystem();

    // Static instance for callbacks
    static UISystem* s_instance;

    bool init(GLFWwindow* window, RenderSystem* renderer);
    void updateWindowSize(float scale);
    void step(float elapsed_ms);
    void draw();

    // Input event handlers (called mostly from worldsystem)
    void handleKeyEvent(int key, int scancode, int action, int mods);
    void handleMouseMoveEvent(double x, double y);
    void handleMouseButtonEvent(int button, int action, int mods);
    void handleScrollWheelEvent(double xoffset, double yoffset);
    void handleTextInput(unsigned int codepoint);

    // Static callbacks for GLFW
    static void charCallback(GLFWwindow* window, unsigned int codepoint);

    // FPS counter
    void updateFPS(float elapsed_ms);
    float getFPS() const { return m_current_fps; }

    // Inventory bar methods
    void createInventoryBar();
    void updateInventoryBar();
    void updateInventoryText(float elapsed_ms);
    void updatePotionInfo();
    void selectInventorySlot(int slot);
    int getSelectedSlot();
    int getSlotFromId(std::string id);

    // Cauldron methods
    bool openCauldron(Entity cauldron, bool play_sound);
    void updateCauldronUI();
    void closeCauldron(bool play_sound);
    bool isCauldronOpen();
    bool isCauldronOpen(Entity cauldron);
    Entity getOpenedCauldron();
    void setOpenedCauldron(Entity new_cauldron);
    void cauldronDragUpdate(bool isDown);

    // Recipe book methods
    bool openRecipeBook(Entity recipe_book);
    void updateRecipeBookUI();
    void closeRecipeBook();
    bool isRecipeBookOpen();
    Entity getOpenedRecipeBook();
    void setOpenedRecipeBook(Entity new_recipe_book);
    void navigateRecipeBook(bool next_page);

    // Recipe book index for saving/loading
    int current_recipe_index = 0;

    // Mortar and pestle
    bool openMortarPestle(Entity mortar, bool play_sound);
    bool isMortarPestleOpen();
    void closeMortarPestle(bool play_sound);
    Entity getOpenedMortarPestle();
    void setOpenedMortarPestle(Entity new_mortar_pestle);
    Rml::Element* getHeldPestle() { return heldPestle; }

    // Chest menu methods
    bool openChestMenu(Entity chest);
    void updateChestUI();
    bool isChestMenuOpen();
    void closeChestMenu();
    Entity getOpenedChest();
    void setOpenedChest(Entity new_chest);

    // tutorial
    void updateTutorial();

    // textboxes
    void updateTextboxes();
    std::unordered_map<int, Textbox> textboxes;

    void createRmlUITextbox(int id, std::string text, vec2 pos);
    void removeRmlUITextbox(int id);

    // Health bar methods
    void createHealthBar();
    void updateHealthBar();

    // Effects bar method
    void createEffectsBar();
    void updateEffectsBar();

    // Text that introduces a new biome when you first enter it, will fade in and out
    void createScreenText(const std::string& text, float displayDuration);
    void handleQueuedText(float elapsed_ms);
    std::queue<TextQueueItem> textQueue;
    float fadeDuration = 3.0f; // You can adjust this as needed
    float fadeOutTime = 3.0f; // Duration for text fade out

    // Info bar
    void createInfoBar();

    // Check if any UI elements are open/being clicked
    bool isClickOnUIElement();

    // Enemy health bar bethods
    void createEnemyHealthBars();
    void updateEnemyHealthBarPos(Entity entity, vec2 pos);
    void updateEnemyHealth(Entity entity, float health_percentage);

    // Check if player inventory contains the required recipe ingredient
    bool playerHasIngredient(Entity playerEntity, const RecipeIngredient& recipeIngredient);

    // Reference to active animation
    Rml::Element* active_animation = nullptr;

    void startGrindAnimation();

private:
    GLFWwindow* m_window;
    RenderSystem* m_renderer;

    Rml::Context* m_context;

    // UI document
    Rml::ElementDocument* m_document;

    // State tracking
    bool m_initialized;

    // Mouse and key tracking
    double mouse_pos_x;
    double mouse_pos_y;
    bool shift_key_pressed = false;

    // Convert GLFW key to RmlUi key
    Rml::Input::KeyIdentifier convertKey(int key);
    int getKeyModifiers();

    // Get color property string from vec3 color
    std::string getImageColorProperty(vec3 color, float alpha);

    // Update items that should follow the mouse
    void updateFollowMouse();
    void followMouse(Rml::Element* e, int offsetX, int offsetY);

    // FPS counter variables
    float m_frame_times[60] = { 0 }; // Store last 60 frame times
    int m_frame_time_index = 0;
    float m_frame_time_sum = 0;
    float m_current_fps = 0;
    float m_fps_update_timer = 0;

    // Inventory bar variables
    Rml::ElementDocument* m_inventory_document = nullptr;
    int m_hotbar_size = 10;
    const int SHOW_TEXT_MS = 4000;
    const int FADE_TEXT_MS = 1000;
    int showText = 0;
    int fadeText = 0;

    // Cauldron variables
    Rml::ElementDocument* m_cauldron_document = nullptr;
    Entity openedCauldron;
    Rml::Element* heldLadle = nullptr;
    Rml::Element* heldBottle = nullptr;

    // Recipe book variables
    Rml::ElementDocument* m_recipe_book_document = nullptr;
    Entity openedRecipeBook;

    // Mortar & Pestle variables
    Rml::ElementDocument* m_mortar_document = nullptr;
    Entity openedMortar;
    Rml::Element* heldPestle = nullptr;

    // Chest variables
    Rml::ElementDocument* m_chest_document = nullptr;
    Entity openedChest;

    // Tutorial variables
    Rml::ElementDocument* m_tutorial_document = nullptr;

    // Textbox variables
    std::unordered_map<int, Rml::ElementDocument*> m_textbox_documents;  // to store multiple textboxes

    std::unordered_map<int, std::tuple<std::string, std::string, std::string, std::optional<std::tuple<std::string, std::string, std::string, std::string, std::string>>>> tutorial_steps = {
        { (int)TUTORIAL::WELCOME_SCREEN, {
            }},
        { (int)TUTORIAL::TOGGLE_TUTORIAL, {
            "660px", "90px", "This tutorial will teach you to brew your first potion! Press T to toggle the tutorial or N to skip ahead. Press N to continue", std::nullopt
        }},
        { (int)TUTORIAL::TOOLS, {
            "660px", "75px", "The grotto holds essential tools for potion making: a chest, recipe book, mortar & pestle and cauldron! Press N to continue",
            std::make_optional(std::make_tuple("625px", "350px", "tools_tutorial.png", "1250px", "700px"))
        }},
        { (int)TUTORIAL::RECIPE_BOOK, {
            "660px", "90px", "Head to the lectern using the WASD keys and press F to open the recipe book. You can also use R to open the recipe book at any time.",
            std::make_optional(std::make_tuple("210px", "100px", "arrow_down.png", "100px", "100px"))
        }},
        { (int)TUTORIAL::FLIP_PAGE, {
            "570px", "55px", "Flip to the recipe page for \"Potion of Harming\".", std::nullopt
        }},
        { (int)TUTORIAL::EXIT_GROTTO, {
            "570px", "55px", "We're missing a couple of ingredients! Exit the recipe book menu with F and leave the grotto from the bottom right exit.", std::nullopt
        }},
        { (int)TUTORIAL::COLLECT_ITEMS, {
            "305px", "202px", "Welcome to the forest! There's a heavy fog of corruption... Collect 2 storm bark and 1 blightleaf.",
            std::make_optional(std::make_tuple("305px", "300px", "tutorial_ingredients.png", "100px", "100px"))
        }},
        { (int)TUTORIAL::ENTER_GROTTO, {
            "305px", "202px", "Great, now head back to the grotto.", std::nullopt
        }},
        { (int)TUTORIAL::MORTAR_PESTLE, {
            "660px", "90px", "Go to the mortar and pestle and press F to open the menu",
            std::make_optional(std::make_tuple("420px", "125px", "arrow_down.png", "100px", "100px"))
        }},
        { (int)TUTORIAL::GRIND_BARK, {
            "272px", "200px", "Drag a storm bark from your inventory into the mortar and pestle. Then pickup the pestle and grind the bark!",
            std::make_optional(std::make_tuple("272px", "332px", "grind_tutorial.png", "280px", "85px"))
        }},
        { (int)TUTORIAL::INTERACT_CAULDRON, {
            "284px", "190px", "Now you have Storm Sap! Click it to pick it up. Exit the menu with F and go to use your cauldron.", std::nullopt
        }},
        { (int)TUTORIAL::SET_HEAT, {
            "210px", "300px", "Drag the heat dial to high. Press R to reference the recipe if needed.",
            std::make_optional(std::make_tuple("125px", "452px", "arrow_right.png", "100px", "100px"))
        }},
        { (int)TUTORIAL::ADD_INGREDIENTS, {
            "210px", "300px", "Add in 1 blightleaf, 1 storm bark and 1 storm sap by dragging it from your inventory into the cauldron.", std::nullopt
        }},
        { (int)TUTORIAL::STIR, {
            "210px", "300px", "Pick up the ladle by clicking and dragging it in the cauldron. Stir 3 times, a successful stir will flash and play a whoosh sound.", std::nullopt
        }},
        { (int)TUTORIAL::WAIT, {
            "210px", "300px", "Wait for 10 seconds for the potion to develop. You can use the timer that begins turning once you start brewing.",
            std::make_optional(std::make_tuple("125px", "156px", "arrow_right.png", "100px", "100px"))
        }},
        { (int)TUTORIAL::BOTTLE, {
            "210px", "300px", "Drag the bottle to the cauldron and left-click to bottle your potion. Match the recipe color for better quality and effect.",
            std::make_optional(std::make_tuple("1145px", "466px", "arrow_left.png", "100px", "100px"))
        }},
        { (int)TUTORIAL::THROW_POTION, {
            "660px", "90px", "Exit the cauldron menu by clicking F. Throw your damage potion at an enemy with left click. Consume potions with right click. Press N to continue.", std::nullopt
        }},
        { (int)TUTORIAL::POTION_EFFECT, {
            "660px", "90px", "Your player health is the green bar on the bottom right. Consumed potion effects appear on the top right. Good luck saving the grotto! Press N to end the tutorial.",
            std::make_optional(std::make_tuple("625px", "350px", "effect_health_tutorial.png", "1250px", "700px"))
        }}
    };

    const std::string LADLE_LEFT_PX = "866px";
    const std::string LADLE_TOP_PX = "45px";
    const std::string BOTTLE_LEFT_PX = "904px";
    const std::string BOTTLE_TOP_PX = "395px";

    const std::string PESTLE_LEFT_PX = "800px";
    const std::string PESTLE_TOP_PX = "300px";

    // Helper functions for recipe book
    std::string getRecipeHtml(int recipe_index);
    std::string getRecipeStepsText(const Recipe& recipe);
    std::string getRecipeIngredientsText(const Recipe& recipe);
    std::string getIngredientName(RecipeIngredient ing);

    // Healthbar variables
    Rml::ElementDocument* m_healthbar_document = nullptr;

    // Effects bar variables
    Rml::ElementDocument* m_effectsbar_document = nullptr;

    // Document for creating biome text
    Rml::ElementDocument* m_biome_text_document = nullptr;
    int m_effectsbar_size = 4;

    // Enemy healthbar variables
    std::unordered_map<int, Rml::ElementDocument*> enemy_healthbars = {}; // int is entity id
    // Info bar
    Rml::ElementDocument* m_info_document = nullptr;
};