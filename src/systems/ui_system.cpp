#include "ui_system.hpp"
#include "render_system.hpp"
#include "rmlui_system_interface.hpp"
#include "rmlui_render_interface.hpp"
#include <iostream>
#include <vector>
#include <string>
// Global flag to indicate when UI rendering is in progress
// Used to prevent OpenGL error checking during RmlUi rendering
bool g_ui_rendering_in_progress = false;

// Static instance for callbacks
UISystem* UISystem::s_instance = nullptr;

UISystem::UISystem()
    : m_window(nullptr),
    m_renderer(nullptr),
    m_context(nullptr),
    m_document(nullptr),
    m_initialized(false)
{
    s_instance = this;
}

UISystem::~UISystem()
{
    // Clean up RmlUi resources in reverse order of creation
    if (m_context) {
        if (m_document) {
            m_document->Close();
            m_document = nullptr;
        }

        Rml::RemoveContext(m_context->GetName());
        m_context = nullptr;
    }

    Rml::Shutdown();

    s_instance = nullptr;
}

bool UISystem::init(GLFWwindow* window, RenderSystem* renderer)
{
    if (!window || !renderer) {
        std::cerr << "UISystem::init - Invalid window or renderer" << std::endl;
        return false;
    }

    m_window = window;
    m_renderer = renderer;

    std::cout << "UISystem::init - Starting initialization" << std::endl;

    try {
        static RmlUiSystemInterface system_interface(window);
        static RmlUiRenderInterface render_interface;

        Rml::SetSystemInterface(&system_interface);
        Rml::SetRenderInterface(&render_interface);

        if (!Rml::Initialise()) {
            std::cerr << "UISystem::init - Failed to initialize RmlUi" << std::endl;
            return false;
        }

        std::cout << "UISystem::init - RmlUi initialized successfully" << std::endl;

        // Load font (definitely not excessive)
        std::vector<std::string> font_paths = {
            "/ext/data/fonts/OpenSans-Regular.ttf",
            "../ext/data/fonts/OpenSans-Regular.ttf",
            "\\ext\\data\\fonts\\OpenSans-Regular.ttf",
            "..\\ext\\data\\fonts\\OpenSans-Regular.ttf",
            "./data/fonts/OpenSans-Regular.ttf",
            ".\\ext\\data\\fonts\\OpenSans-Regular.ttf",
            "../data/fonts/OpenSans-Regular.ttf",
            "data/fonts/OpenSans-Regular.ttf"
        };

        bool font_loaded = false;
        for (const auto& path : font_paths) {
            std::cout << "UISystem::init - Attempting to load font from: " << path << std::endl;
            if (Rml::LoadFontFace(path)) {
                std::cout << "UISystem::init - Successfully loaded font from: " << path << std::endl;
                font_loaded = true;
                break;
            }
            std::cerr << "UISystem::init - Failed to load font from: " << path << std::endl;
        }

        if (!font_loaded) {
            std::cerr << "UISystem::init - Failed to load font from any path" << std::endl;
            return false;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Create context for RmlUi
        std::cout << "UISystem::init - Creating context with dimensions " << width << "x" << height << std::endl;
        m_context = Rml::CreateContext("main", Rml::Vector2i(width, height));
        if (!m_context) {
            std::cerr << "UISystem::init - Failed to create context" << std::endl;
            return false;
        }

        // Simple text-only UI document for debugging
        std::cout << "UISystem::init - Creating text-only UI document" << std::endl;
        m_document = m_context->LoadDocumentFromMemory(R"(
        <rml>
        <head>
            <style>
                body { 
                    margin: 0; 
                    padding: 0; 
                    background-color: transparent;
                    pointer-events: none;
                    width: 100%;
                    height: 100%;
                }
                div.text { 
                    color: white; 
                    font-size: 16px;
                    background-color: #000000;
                    margin: 5px;
                    position: absolute;
                    top: 10px;
                    left: 10px;
                    font-family: Open Sans;
                }
            </style>
        </head>
        <body>
            <div class="text">Enchanted Grotto Text</div>
        </body>
        </rml>
    )");

        // Show the document
        m_document->Show();

        // Create the inventory bar
        createInventoryBar();

        m_initialized = true;
        std::cout << "UISystem::init - Successfully initialized" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UISystem::init: " << e.what() << std::endl;
        return false;
    }
}

void UISystem::step(float elapsed_ms)
{
    if (!m_initialized || !m_context) return;

    try {
        // Update FPS counter
        updateFPS(elapsed_ms);

        // Create FPS counter UI if it doesn't exist
        if (!m_fps_document) {
            const char* fps_rml =
                "<rml>\n"
                "<head>\n"
                "    <style>\n"
                "        body {\n"
                "            position: absolute;\n"
                "            top: 10px;\n"
                "            right: 10px;\n"
                "            font-family: Open Sans;\n"
                "            font-size: 18px;\n"
                "            font-weight: bold;\n"
                "            color: white;\n"
                "            background-color: rgba(0, 0, 0, 0.7);\n"
                "            padding: 8px 12px;\n"
                "            border-radius: 8px;\n"
                "            width: auto;\n"
                "            box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.5);\n"
                "        }\n"
                "    </style>\n"
                "</head>\n"
                "<body id=\"fps_counter\">FPS: 0</body>\n"
                "</rml>";

            m_fps_document = m_context->LoadDocumentFromMemory(fps_rml);
            if (m_fps_document) {
                m_fps_document->Show();
            }
        }

        // Update FPS display every 250ms to avoid too frequent updates
        if (m_fps_document && m_fps_update_timer >= 250.0f) {
            m_fps_update_timer = 0.0f;

            // Determine color based on FPS (green for good, yellow for ok, red for poor)
            const char* color = "#00FF00"; // Green by default (good performance)
            if (m_current_fps < 30.0f) {
                color = "#FF0000"; // Red (poor performance)
            }
            else if (m_current_fps < 55.0f) {
                color = "#FFFF00"; // Yellow (ok performance)
            }

            char fps_text[64];
            snprintf(fps_text, sizeof(fps_text), "FPS: <span style=\"color: %s;\">%.1f</span>",
                color, m_current_fps);
            m_fps_document->SetInnerRML(fps_text);
        }

        if (!m_inventory_document) {
            createInventoryBar();
        }

        // Update inventory bar
        updateInventoryBar();

        // Update RmlUi
        m_context->Update();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UISystem::step: " << e.what() << std::endl;
    }
}

void UISystem::draw()
{
    if (!m_initialized || !m_context) {
        return;
    }

    g_ui_rendering_in_progress = true;

    try {
        // Save ALL relevant OpenGL state
        GLint last_viewport[4];
        glGetIntegerv(GL_VIEWPORT, last_viewport);

        GLint last_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);

        GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
        GLboolean blend = glIsEnabled(GL_BLEND);
        GLint blend_src, blend_dst;
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blend_src);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blend_dst);

        GLint last_framebuffer;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_framebuffer);

        // Save additional OpenGL state
        GLint last_vao, last_active_texture;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vao);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);

        // Set up state for UI rendering
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        // Ensure we're rendering to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);

        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Disable depth testing for UI
        glDisable(GL_DEPTH_TEST);

        // Render UI
        m_context->Render();

        // Restore additional OpenGL state
        glBindFramebuffer(GL_FRAMEBUFFER, last_framebuffer);
        glBindVertexArray(last_vao);
        glActiveTexture(last_active_texture);

        // Restore previous OpenGL state
        glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
        glUseProgram(last_program);

        if (depth_test)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        if (blend)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);

        glBlendFunc(blend_src, blend_dst);
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UISystem::draw: " << e.what() << std::endl;
    }

    g_ui_rendering_in_progress = false;
}

// Convert GLFW key to RmlUi key
Rml::Input::KeyIdentifier UISystem::convertKey(int key)
{
    switch (key) {
    case GLFW_KEY_A: return Rml::Input::KI_A;
    case GLFW_KEY_B: return Rml::Input::KI_B;
    case GLFW_KEY_C: return Rml::Input::KI_C;
    case GLFW_KEY_D: return Rml::Input::KI_D;
    case GLFW_KEY_E: return Rml::Input::KI_E;
    case GLFW_KEY_F: return Rml::Input::KI_F;
    case GLFW_KEY_G: return Rml::Input::KI_G;
    case GLFW_KEY_H: return Rml::Input::KI_H;
    case GLFW_KEY_I: return Rml::Input::KI_I;
    case GLFW_KEY_J: return Rml::Input::KI_J;
    case GLFW_KEY_K: return Rml::Input::KI_K;
    case GLFW_KEY_L: return Rml::Input::KI_L;
    case GLFW_KEY_M: return Rml::Input::KI_M;
    case GLFW_KEY_N: return Rml::Input::KI_N;
    case GLFW_KEY_O: return Rml::Input::KI_O;
    case GLFW_KEY_P: return Rml::Input::KI_P;
    case GLFW_KEY_Q: return Rml::Input::KI_Q;
    case GLFW_KEY_R: return Rml::Input::KI_R;
    case GLFW_KEY_S: return Rml::Input::KI_S;
    case GLFW_KEY_T: return Rml::Input::KI_T;
    case GLFW_KEY_U: return Rml::Input::KI_U;
    case GLFW_KEY_V: return Rml::Input::KI_V;
    case GLFW_KEY_W: return Rml::Input::KI_W;
    case GLFW_KEY_X: return Rml::Input::KI_X;
    case GLFW_KEY_Y: return Rml::Input::KI_Y;
    case GLFW_KEY_Z: return Rml::Input::KI_Z;

    case GLFW_KEY_0: return Rml::Input::KI_0;
    case GLFW_KEY_1: return Rml::Input::KI_1;
    case GLFW_KEY_2: return Rml::Input::KI_2;
    case GLFW_KEY_3: return Rml::Input::KI_3;
    case GLFW_KEY_4: return Rml::Input::KI_4;
    case GLFW_KEY_5: return Rml::Input::KI_5;
    case GLFW_KEY_6: return Rml::Input::KI_6;
    case GLFW_KEY_7: return Rml::Input::KI_7;
    case GLFW_KEY_8: return Rml::Input::KI_8;
    case GLFW_KEY_9: return Rml::Input::KI_9;

    case GLFW_KEY_BACKSPACE: return Rml::Input::KI_BACK;
    case GLFW_KEY_TAB: return Rml::Input::KI_TAB;
    case GLFW_KEY_ENTER: return Rml::Input::KI_RETURN;
    case GLFW_KEY_ESCAPE: return Rml::Input::KI_ESCAPE;
    case GLFW_KEY_SPACE: return Rml::Input::KI_SPACE;
    case GLFW_KEY_LEFT: return Rml::Input::KI_LEFT;
    case GLFW_KEY_RIGHT: return Rml::Input::KI_RIGHT;
    case GLFW_KEY_UP: return Rml::Input::KI_UP;
    case GLFW_KEY_DOWN: return Rml::Input::KI_DOWN;

    default: return Rml::Input::KI_UNKNOWN;
    }
}

int UISystem::getKeyModifiers()
{
    int modifiers = 0;

    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        modifiers |= Rml::Input::KM_SHIFT;

    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(m_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
        modifiers |= Rml::Input::KM_CTRL;

    if (glfwGetKey(m_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(m_window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
        modifiers |= Rml::Input::KM_ALT;

    return modifiers;
}


void UISystem::handleKeyEvent(int key, int scancode, int action, int mods)
{
    if (!m_initialized || !m_context) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        m_context->ProcessKeyDown(convertKey(key), getKeyModifiers());
    }
    else {
        m_context->ProcessKeyUp(convertKey(key), getKeyModifiers());
    }

    // Handle number keys for inventory selection (1-9)
    if (action == GLFW_PRESS && key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
        int slot = key - GLFW_KEY_1; // Convert key to 0-based slot index
        selectInventorySlot(slot);
    }
}

void UISystem::handleTextInput(unsigned int codepoint)
{
    if (!m_initialized || !m_context) return;

    m_context->ProcessTextInput((Rml::Character)codepoint);
}

void UISystem::handleMouseMoveEvent(double x, double y)
{
    if (!m_initialized || !m_context) return;

    m_context->ProcessMouseMove((int)x, (int)y, getKeyModifiers());
}

void UISystem::handleMouseButtonEvent(int button, int action, int mods)
{
    if (!m_initialized || !m_context) return;

    // Convert GLFW mouse button to RmlUi mouse button
    int rml_button = 0;
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        rml_button = 0;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        rml_button = 1;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        rml_button = 2;
        break;
    default:
        return;
    }

    // Get mouse position
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);

    // Check if the click is on the inventory bar
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        // Calculate inventory bar position (this is kinda broken rn)
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        // Inventory bar is centered at the bottom
        float bar_width = 450.0f;
        float bar_height = 60.0f;
        float bar_x = (width - bar_width) / 2.0f;
        float bar_y = height - bar_height - 20.0f; // 20px from bottom

        // Check if click is within the inventory bar area
        if (x >= bar_x && x <= bar_x + bar_width &&
            y >= bar_y && y <= bar_y + bar_height) {

            // Calculate which slot was clicked
            float slot_width = bar_width / m_hotbar_size;
            int slot = (int)((x - bar_x) / slot_width);

            if (slot >= 0 && slot < m_hotbar_size) {
                selectInventorySlot(slot);
                std::cout << "Selected inventory slot: " << slot << std::endl;
            }
        }
    }

    // Pass the event to RmlUi
    if (action == GLFW_PRESS) {
        m_context->ProcessMouseButtonDown(rml_button, getKeyModifiers());
    }
    else if (action == GLFW_RELEASE) {
        m_context->ProcessMouseButtonUp(rml_button, getKeyModifiers());
    }
}

// Static callbacks
void UISystem::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (s_instance) {
        s_instance->handleKeyEvent(key, scancode, action, mods);
    }
}

void UISystem::charCallback(GLFWwindow* window, unsigned int codepoint)
{
    if (s_instance) {
        s_instance->handleTextInput(codepoint);
    }
}

void UISystem::cursorPosCallback(GLFWwindow* window, double x, double y)
{
    if (s_instance) {
        s_instance->handleMouseMoveEvent(x, y);
    }
}

void UISystem::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (s_instance) {
        s_instance->handleMouseButtonEvent(button, action, mods);
    }
}

void UISystem::updateFPS(float elapsed_ms)
{
    m_frame_time_sum -= m_frame_times[m_frame_time_index];
    m_frame_times[m_frame_time_index] = elapsed_ms;
    m_frame_time_sum += elapsed_ms;

    m_frame_time_index = (m_frame_time_index + 1) % 60;

    // Calculate average FPS over the last 60 frames
    float avg_frame_time = m_frame_time_sum / 60.0f;
    if (avg_frame_time > 0) {
        m_current_fps = 1000.0f / avg_frame_time;
    }

    // Update timer for display refresh
    m_fps_update_timer += elapsed_ms;
}

void UISystem::createInventoryBar()
{
    if (!m_initialized || !m_context) return;

    try {
        std::cout << "UISystem::createInventoryBar - Creating inventory bar" << std::endl;

        std::string inventory_rml = R"(
        <rml>
        <head>
            <style>
                body {
                    position: absolute;
                    bottom: 20px;
                    left: 50%;
                    margin-left: -225px;
                    width: 450px;
                    height: 60px;
                    background-color: #8B5A2B;
                    border-width: 4px;
                    border-color: #4E3620;
                    padding: 5px;
                    display: block;
                    pointer-events: none;
                    z-index: 100;
                    font-family: Open Sans;
                }
                
                .inventory-slot {
                    width: 40px;
                    height: 40px;
                    margin: 5px;
                    background-color: rgba(60, 40, 20, 0.7);
                    display: inline-block;
                    text-align: center;
                    vertical-align: middle;
                    border-width: 2px;
                    border-color: #5D4037;
                    position: relative;
                }
                
                .inventory-slot.selected {
                    background-color: rgba(120, 80, 40, 0.8);
                    border-width: 2px;
                    border-color: #FFD700;
                }
                
                .inventory-slot .item-count {
                    position: absolute;
                    bottom: 2px;
                    right: 5px;
                    color: #FFFFFF;
                    font-family: Open Sans;
                    font-size: 14px;
                    font-weight: bold;
                }
            </style>
        </head>
        <body id="inventory-bar">
        )";

        // Add inventory slots
        for (int i = 0; i < m_hotbar_size; i++) {
            std::string slot_class = "inventory-slot";
            if (i == m_selected_slot) {
                slot_class += " selected";
            }

            // Number near each slot
            inventory_rml += "<div id='slot-" + std::to_string(i) + "' class='" + slot_class + "'>" +
                "<span style='color: #FFE4B5; font-size: 12px; font-weight: bold; position: absolute; top: 2px; left: 2px;'>" +
                std::to_string(i + 1) + "</span></div>";
        }

        inventory_rml += "</body></rml>";

        m_inventory_document = m_context->LoadDocumentFromMemory(inventory_rml.c_str());
        if (m_inventory_document) {
            m_inventory_document->Show();
            std::cout << "UISystem::createInventoryBar - Inventory bar created successfully" << std::endl;
        }
        else {
            std::cerr << "UISystem::createInventoryBar - Failed to create inventory document" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UISystem::createInventoryBar: " << e.what() << std::endl;
    }
}

void UISystem::updateInventoryBar()
{
    if (!m_initialized || !m_context || !m_inventory_document) return;

    try {
        if (registry.players.entities.empty()) return;

        Entity player = registry.players.entities[0]; // Assuming there's only one player

        if (!registry.inventories.has(player)) return;

        Inventory& inventory = registry.inventories.get(player);

        // Update each slot in the inventory bar
        for (int i = 0; i < m_hotbar_size; i++) {
            std::string slot_id = "slot-" + std::to_string(i);
            Rml::Element* slot_element = m_inventory_document->GetElementById(slot_id);

            if (!slot_element) continue;

            // Update the slot class (selected or not)
            std::string slot_class = "inventory-slot";
            if (i == m_selected_slot) {
                slot_class += " selected";
            }
            slot_element->SetAttribute("class", slot_class);

            std::string slot_content = "<span style='color: #FFE4B5; font-size: 12px; font-weight: bold; position: absolute; top: 2px; left: 2px;'>" +
                std::to_string(i + 1) + "</span>";

            // Add item display
            if (i < inventory.items.size()) {
                Entity item_entity = inventory.items[i];

                if (registry.items.has(item_entity)) {
                    Item& item = registry.items.get(item_entity);

                    // TODO: Add item type to texture associations
                    // Add item display
                    if (item.type == ItemType::COFFEE_BEANS) {
                        slot_content += "<img src='interactables/coffee_bean.png' style='width: 32px; height: 32px; margin: 4px;' />";
                    }
                    else if (item.type == ItemType::MAGICAL_FRUIT) {
                        slot_content += "<img src='interactables/magical_fruit.png' style='width: 32px; height: 32px; margin: 4px;' />";
                    }
                    else if (item.type == ItemType::POTION) {
                        // TODO potion textures
                    }
                    else {
                        // TODO fallback for unknown items
                    }

                    // Add item count if more than 1
                    if (item.amount > 1) {
                        slot_content += "<div style='position: absolute; bottom: 0; right: 2px; color: #FFFFFF; font-size: 14px; font-weight: bold;'>" +
                            std::to_string(item.amount) + "</div>";
                    }
                }
            }

            slot_element->SetInnerRML(slot_content);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UISystem::updateInventoryBar: " << e.what() << std::endl;
    }
}

void UISystem::selectInventorySlot(int slot)
{
    if (slot < 0 || slot >= m_hotbar_size) return;

    m_selected_slot = slot;

    // Update the inventory bar to reflect the selection
    if (m_inventory_document) {
        updateInventoryBar();
    }
}

bool UISystem::openCauldron()
{
    if (!m_initialized || !m_context) return false;
    if (m_cauldron_document) {
        m_cauldron_document->Show();
        return true;
    }

    try {
        std::cout << "UISystem::createCauldronUI - Creating cauldron UI" << std::endl;

        std::string cauldron_rml = R"(
        <rml>
        <head>
            <style>
                img {
                    display: block;
                    margin-left: auto;
                    margin-right: auto;
                    margin-top: 25px;
                    height: 550px;
                    transform: scaleY(-1);
                }
            </style>
        </head>
        <body>
            <img src="interactables/cauldron_background.png"></img>
        </body>
        </rml>
        )";

        m_cauldron_document = m_context->LoadDocumentFromMemory(cauldron_rml.c_str());
        if (m_cauldron_document) {
            m_cauldron_document->Show();
            std::cout << "UISystem::openCauldron - Cauldron created successfully" << std::endl;
            return true;
        }
        else {
            std::cerr << "UISystem::openCauldron - Failed to open cauldron" << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UISystem::openCauldron: " << e.what() << std::endl;
        return false;
    }
}

bool UISystem::isCauldronOpen() {
    return m_cauldron_document && m_cauldron_document->IsVisible();
}

void UISystem::closeCauldron() 
{
    m_cauldron_document->Hide();
}