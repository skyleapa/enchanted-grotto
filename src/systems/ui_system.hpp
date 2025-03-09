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

// Main UI system class
class UISystem {
public:
    UISystem();
    ~UISystem();

    // Static instance for callbacks
    static UISystem* s_instance;

    bool init(GLFWwindow* window, RenderSystem* renderer);
    void step(float elapsed_ms);
    void draw();

    // Input event handlers (called from GLFW callbacks)
    void handleKeyEvent(int key, int scancode, int action, int mods);
    void handleMouseMoveEvent(double x, double y);
    void handleMouseButtonEvent(int button, int action, int mods);
    void handleTextInput(unsigned int codepoint);

    // Static callbacks for GLFW
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double x, double y);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void charCallback(GLFWwindow* window, unsigned int codepoint);

    // FPS counter
    void updateFPS(float elapsed_ms);
    float getFPS() const { return m_current_fps; }

    // Inventory bar methods
    void createInventoryBar();
    void updateInventoryBar();
    void selectInventorySlot(int slot);
    int getSelectedSlot() const { return m_selected_slot; }

    // Cauldron methods
    bool openCauldron(Entity cauldron);
    void updateCauldronUI();
    void closeCauldron();
    bool isCauldronOpen();
    bool isCauldronOpen(Entity cauldron);
    Entity getOpenedCauldron();
    void ladleFollowMouse(double x, double y);

    // tutorial
    void updateTutorial();

private:
    GLFWwindow* m_window;
    RenderSystem* m_renderer;

    Rml::Context* m_context;

    // UI document
    Rml::ElementDocument* m_document;

    // State tracking
    bool m_initialized;

    // Convert GLFW key to RmlUi key
    Rml::Input::KeyIdentifier convertKey(int key);
    int getKeyModifiers();

    // FPS counter variables
    float m_frame_times[60] = { 0 }; // Store last 60 frame times
    int m_frame_time_index = 0;
    float m_frame_time_sum = 0;
    float m_current_fps = 0;
    float m_fps_update_timer = 0;
    Rml::ElementDocument* m_fps_document = nullptr;

    // Inventory bar variables
    Rml::ElementDocument* m_inventory_document = nullptr;
    int m_selected_slot = 0;
    int m_hotbar_size = 8;

    // Cauldron variables
    Rml::ElementDocument* m_cauldron_document = nullptr;
    Entity openedCauldron;
    Rml::Element* heldLadle = nullptr;

    // Base texture folder is data/textures
    const std::unordered_map<ItemType, std::string> ITEM_TEXTURES = {
        {ItemType::COFFEE_BEANS, "interactables/coffee_bean.png"},
        {ItemType::MAGICAL_FRUIT, "interactables/magical_fruit.png"},
    };

    // Tutorial variables
    Rml::ElementDocument* m_tutorial_document = nullptr;

    const std::unordered_map<int, std::tuple<std::string, std::string, std::string>> tutorial_steps = {
        { (int)TUTORIAL::MOVEMENT, {
            "60%", "40%", "Welcome to Enchanted Grotto! Move around with the WASD keys. Press T to toggle the tutorial at any time or N to skip the tutorial step."
        }},
        { (int)TUTORIAL::COLLECT_FRUITS, {
            "80%", "85%", "Collect 3 Magical Fruits by pressing F when near fruit trees."
        }},
        { (int)TUTORIAL::ATTACK_ENEMY, {
            "25%", "35%", "Select the fruits by clicking on the corresponding inventory slot. Defeat the enemy by left-clicking to throw fruits from your inventory. If you touch the enemy you will die!"
        }},
        { (int)TUTORIAL::ENTER_GROTTO, {
            "55%", "10%", "Nice work! Now enter the grotto by pressing F at the entrance."
        }},
        { (int)TUTORIAL::INTERACT_CAULDRON, {
            "70%", "45%", "Open the potion-making menu by pressing F in front of the cauldron."
        }},
        { (int)TUTORIAL::STIR, {
            "85%", "90%", "Click on the ladle and bring it to the cauldron. Click and drag to make a circle in the cauldron to stir."
        }}
    };

};