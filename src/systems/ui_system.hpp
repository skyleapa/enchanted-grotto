#pragma once

// OpenGL includes in correct order
#include <gl3w/glcorearb.h>  // Core OpenGL types
#include <gl3w/gl3w.h>       // OpenGL loader
#include <GLFW/glfw3.h>      // GLFW

// RmlUi include
#include <RmlUi/Core.h>

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
    float m_frame_times[60] = {0}; // Store last 60 frame times
    int m_frame_time_index = 0;
    float m_frame_time_sum = 0;
    float m_current_fps = 0;
    float m_fps_update_timer = 0;
    Rml::ElementDocument* m_fps_document = nullptr;

    // Inventory bar variables
    Rml::ElementDocument* m_inventory_document = nullptr;
    int m_selected_slot = 0;
    int m_hotbar_size = 8;
};
