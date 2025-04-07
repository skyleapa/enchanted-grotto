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

    // Tutorial variables
    Rml::ElementDocument* m_tutorial_document = nullptr;

    // Textbox variables
    std::unordered_map<int, Rml::ElementDocument*> m_textbox_documents;  // to store multiple textboxes

    const std::unordered_map<int, std::tuple<std::string, std::string, std::string>> tutorial_steps = {
        { (int)TUTORIAL::WELCOME_SCREEN, {
            }},
        { (int)TUTORIAL::TOGGLE_TUTORIAL, {
            "820px", "466px", "Let's make your first potion! Toggle the tutorial with T at any time or N to skip to the tutorial step. Try pressing N now!"
        }},
        { (int)TUTORIAL::RECIPE_BOOK, {
            "211px", "110px", "Head to the lectern using the WASD keys and press F to open the recipe book. You can also use R to access the recipe book at any time."
        }},
        { (int)TUTORIAL::FLIP_PAGE, {
            "790px", "455px", "Flip to the recipe page for \"Potion of Harming\"."
        }},
        { (int)TUTORIAL::EXIT_GROTTO, {
            "790px", "455px", "Looks like we're missing a couple of ingredients! Exit the recipe book menu by pressing F and leave the grotto from the exit in the bottom right."
        }},
        { (int)TUTORIAL::COLLECT_ITEMS, {
            "25%", "35%", "Welcome to the forest! Collect 2 storm bark and 1 blightleaf for your potion, you may have to explore the area to find ingredients."
        }},
        { (int)TUTORIAL::ENTER_GROTTO, {
            "611px", "200px", "Great, now head back into the grotto."
        }},
        { (int)TUTORIAL::MORTAR_PESTLE, {
            "422px", "125px", "Go to the mortar and pestle and press F to open the menu"
        }},
        { (int)TUTORIAL::GRIND_BARK, {
            "284px", "190px", "Insert a storm bark by dragging it into the mortar and pestle from your inventory. Then pickup the pestle and start pounding that bark!"
        }},
        { (int)TUTORIAL::INTERACT_CAULDRON, {
            "284px", "190px", "Now you have Storm Sap! Click it in the mortar and pestle to pick it up. Exit the menu with F and go to your cauldron. Press F to open the cauldron menu."
        }},
        { (int)TUTORIAL::SET_HEAT, {
            "210px", "300px", "Get started by dragging the heat dial to high. If you forget the recipe, you can always exit and take a peek, the cauldron state is saved."
        }},
        { (int)TUTORIAL::ADD_INGREDIENTS, {
            "210px", "300px", "Now add in 1 storm sap, 1 blightleaf and 1 storm bark by dragging it from your inventory into the cauldron."
        }},
        { (int)TUTORIAL::STIR, {
            "210px", "300px", "Pick up the ladle by clicking and dragging it in the cauldron. Stir 3 times, a successful stir will flash and play a whoosh sound."
        }},
        { (int)TUTORIAL::WAIT, {
            "210px", "300px", "Wait for 5 seconds for the potion to develop. You can use the timer that begins turning once you start brewing."
        }},
        { (int)TUTORIAL::BOTTLE, {
            "210px", "300px", "Now bottle your potion by dragging the bottle to the cauldron and left clicking. The closer the color to the recipe book, the better the quality and potion effect."
        }},
        { (int)TUTORIAL::THROW_POTION, {
            "210px", "300px", "Exit the cauldron menu by clicking F. You can try throwing your potion at enemies with left click. Consume potions by right clicking on the potion in your inventory."
        }},
        { (int)TUTORIAL::POTION_EFFECT, {
            "1020px", "336px", "You can see your player health with the green bar in the bottom right, and when you consume a potion, it will be displayed in the top right box. Good luck saving the grotto!"
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