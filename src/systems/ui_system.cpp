#include "ui_system.hpp"
#include "render_system.hpp"
#include "potion_system.hpp"
#include "drag_listener.hpp"
#include "item_system.hpp"
#include "rmlui_system_interface.hpp"
#include "rmlui_render_interface.hpp"
#include "sound_system.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <tuple>
#include <sstream>

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

		// Get content scale for Retina displays
		float content_scale = 1.0f;

		int frame_buffer_width_px, frame_buffer_height_px;
		glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px);
		if (frame_buffer_width_px != WINDOW_WIDTH_PX) {
			// Retina display, scale up to 2x
			content_scale = (float)frame_buffer_width_px / WINDOW_WIDTH_PX;
		}

		render_interface.SetContentScale(content_scale);
		Rml::SetSystemInterface(&system_interface);
		Rml::SetRenderInterface(&render_interface);
		if (!Rml::Initialise()) {
			std::cerr << "UISystem::init - Failed to initialize RmlUi" << std::endl;
			return false;
		}

		DragListener::LinkUISystem(this);

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

		// Get the actual window size (in screen coordinates)
		int window_width, window_height;
		glfwGetWindowSize(window, &window_width, &window_height);

		m_context = Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));
		if (!m_context) {
			std::cerr << "UISystem::init - Failed to create context" << std::endl;
			return false;
		}

		// Create the inventory bar
		createInventoryBar();

		// Create player's health bar
		createHealthBar();

		m_initialized = true;
		std::cout << "UISystem::init - Successfully initialized" << std::endl;
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in UISystem::init: " << e.what() << std::endl;
		return false;
	}
}

void UISystem::updateWindowSize(float scale)
{
	RmlUiRenderInterface* rinterface = static_cast<RmlUiRenderInterface*>(Rml::GetRenderInterface());
	rinterface->SetContentScale(scale);
}

void UISystem::step(float elapsed_ms)
{
	if (!m_initialized || !m_context) return;

	try {
		// Moved FPS to title but leave code here in case we bring it back to on screen

		// // Update FPS counter
		// updateFPS(elapsed_ms);

		// // Create FPS counter UI if it doesn't exist
		// if (!m_fps_document) {
		// 	const char* fps_rml =
		// 		"<rml>\n"
		// 		"<head>\n"
		// 		"    <style>\n"
		// 		"        body {\n"
		// 		"            position: absolute;\n"
		// 		"            top: 10px;\n"
		// 		"            right: 10px;\n"
		// 		"            font-family: Open Sans;\n"
		// 		"            font-size: 18px;\n"
		// 		"            font-weight: bold;\n"
		// 		"            color: white;\n"
		// 		"            background-color: rgba(0, 0, 0, 0.7);\n"
		// 		"            padding: 8px 12px;\n"
		// 		"            border-radius: 8px;\n"
		// 		"            width: auto;\n"
		// 		"            box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.5);\n"
		// 		"        }\n"
		// 		"    </style>\n"
		// 		"</head>\n"
		// 		"<body id=\"fps_counter\">FPS: 0</body>\n"
		// 		"</rml>";

		// 	m_fps_document = m_context->LoadDocumentFromMemory(fps_rml);
		// 	if (m_fps_document) {
		// 		m_fps_document->Show();
		// 	}
		// }

		// // Update FPS display every 250ms to avoid too frequent updates
		// if (m_fps_document && m_fps_update_timer >= 250.0f) {
		// 	m_fps_update_timer = 0.0f;

		// 	// Determine color based on FPS (green for good, yellow for ok, red for poor)
		// 	const char* color = "#00FF00"; // Green by default (good performance)
		// 	if (m_current_fps < 30.0f) {
		// 		color = "#FF0000"; // Red (poor performance)
		// 	}
		// 	else if (m_current_fps < 55.0f) {
		// 		color = "#FFFF00"; // Yellow (ok performance)
		// 	}

		// 	char fps_text[64];
		// 	snprintf(fps_text, sizeof(fps_text), "FPS: <span style=\"color: %s;\">%.1f</span>",
		// 		color, m_current_fps);
		// 	m_fps_document->SetInnerRML(fps_text);
		// }

		// update cauldron reference
		if (registry.cauldrons.entities.size() > 0) {
			Entity cauldron = registry.cauldrons.entities[0];
			if (openedCauldron != cauldron) openedCauldron = cauldron;
		}

		if (!m_inventory_document) {
			createInventoryBar();
		}

		if (!m_healthbar_document) {
			createHealthBar();
		}

		// Update inventory bar
		updateInventoryBar();

		// Update health bar
		updateHealthBar();

		// Display tutorial
		updateTutorial();

		// update all the textboxes
		updateTextboxes();

		// Update cauldron color
		updateCauldronUI();

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
		glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);

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
	mouse_pos_x = x;
	mouse_pos_y = y;
	updateFollowMouse();
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
	Rml::Vector2f mousePos = Rml::Vector2f(mouse_pos_x, mouse_pos_y);

	// Check clicks for inventory bar and cauldron
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
		Rml::Element* hovered = m_context->GetHoverElement();
		if (!hovered) return;

		std::string id = hovered->GetId();
		int slotId = getSlotFromId(id);

		// Check for an inventory click
		if (slotId != -1) {
			selectInventorySlot(slotId);
		}

		// Check for ladle/bottle pickup
		do {
			if (!isCauldronOpen()) {
				break;
			}

			if (id == "ladle") {
				// If we click cauldron don't drop ladle
				Rml::Element* possibleCauldron = m_context->GetElementAtPoint(mousePos, hovered);
				if (possibleCauldron && possibleCauldron->GetId() == "cauldron") {
					break;
				}

				SoundSystem::playInteractMenuSound((int)SOUND_CHANNEL::MENU, 0);

				if (heldLadle) {
					hovered->SetProperty("top", LADLE_TOP_PX);
					hovered->SetProperty("left", LADLE_LEFT_PX);
					heldLadle = nullptr;
				}
				else {
					heldLadle = hovered;
					updateFollowMouse();
				}

				break;
			}

			if (id == "bottle") {
				if (!heldBottle) {
					SoundSystem::playInteractMenuSound((int)SOUND_CHANNEL::MENU, 0); // play only when picking up bottle
					heldBottle = hovered;
					updateFollowMouse();
					break;
				}
				
				// Reset bottle position
				hovered->SetProperty("top", BOTTLE_TOP_PX);
				hovered->SetProperty("left", BOTTLE_LEFT_PX);
				heldBottle = nullptr;
				
				// Check if clicking on cauldron water to bottle potion
				Rml::Element* possibleCauldron = m_context->GetElementAtPoint(mousePos, hovered);
				if (!possibleCauldron || (possibleCauldron->GetId() != "cauldron-water" && possibleCauldron->GetId() != "cauldron")) {
					break;
				}
				
				// Create potion and add to player inventory
				Entity cauldron = getOpenedCauldron();
				Potion potion = PotionSystem::bottlePotion(cauldron);

				// Create potion item and add to player inventory
				Entity player = registry.players.entities[0];
				Entity potionItem = ItemSystem::createPotion(
					potion.effect,
					potion.duration,
					potion.color,
					potion.quality,
					potion.effectValue,
					1
				);

				// If couldn't be added to inventory then don't do anything (obviously)
				if (!ItemSystem::addItemToInventory(player, potionItem)) {
					break;
				}

				// Reset cauldron
				PotionSystem::resetCauldron(cauldron);
				m_renderer->initializeWaterBuffers(true); // Will not be necessary once empty cauldron is supported

				// stop the boiling sound and play potion bottling sound
				DragListener::is_boiling = false;
				SoundSystem::haltBoilSound();
				if (potion.quality > 0.75) {
					SoundSystem::playBottleHighQualityPotionSound((int)SOUND_CHANNEL::MENU, 0);
				} else {
					SoundSystem::playBottleSound((int)SOUND_CHANNEL::MENU, 0);
				}
			}
		} while (false);

		// Check for pestle and ingredient pickup
		do {
			if (!isMortarPestleOpen()) {
				break;
			}

			if (id == "pestle") {
				Rml::Element* possibleMortar = m_context->GetElementAtPoint(mousePos, hovered);
				if (possibleMortar && (possibleMortar->GetId() == "mortar" || possibleMortar->GetId() == "mortar_border")) {
					break;
				}

				if (heldPestle) {
					hovered->SetProperty("top", PESTLE_TOP_PX);
					hovered->SetProperty("left", PESTLE_LEFT_PX);
					heldPestle = nullptr;
				}
				else {
					heldPestle = hovered;
					updateFollowMouse();
				}

				break;
			}

			if (id == "mortar") {
				Rml::Element* possibleMortar = m_context->GetElementAtPoint(mousePos, hovered);
				if (possibleMortar && possibleMortar->GetId() == "mortar") {
					break;
				}

				Inventory& mortarInventory = registry.inventories.get(getOpenedMortarPestle());
				if (!mortarInventory.items.empty()) {
					Entity ingredient = mortarInventory.items[0];
		
					// Check it's fully grinded and pickable
					if (registry.items.has(ingredient) && registry.items.get(ingredient).isCollectable
					&& heldPestle == nullptr) {
						Entity player = registry.players.entities[0];
		
						// Move to inventory
						ItemSystem::addItemToInventory(player, ingredient);
						std::cout << "Picked up ingredient from mortar" << std::endl;

						// Clear the mortar inventory
						mortarInventory.items.clear();
						ItemSystem::destroyItem(ingredient);
					}
				}

				break;
			}
		} while (false);
	}

	// Pass the event to RmlUi
	if (action == GLFW_PRESS) {
		m_context->ProcessMouseButtonDown(rml_button, getKeyModifiers());
	}
	else if (action == GLFW_RELEASE) {
		m_context->ProcessMouseButtonUp(rml_button, getKeyModifiers());
	}
}

// Changing inventory slots on scroll wheel
void UISystem::handleScrollWheelEvent(double xoffset, double yoffset)
{
	int dist = (int) yoffset * -1;
	selectInventorySlot(m_selected_slot + dist);
	m_context->ProcessMouseWheel(Rml::Vector2f(xoffset, yoffset), getKeyModifiers());
}

void UISystem::charCallback(GLFWwindow* window, unsigned int codepoint)
{
	if (s_instance) {
		s_instance->handleTextInput(codepoint);
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
	if (!m_context) return;

	try {
		std::cout << "UISystem::createInventoryBar - Creating inventory bar" << std::endl;

		std::string inventory_rml = R"(
        <rml>
        <head>
            <style>
                body {
                    position: absolute;
                    bottom: 10px;
                    left: 50%;
                    margin-left: -220px;
                    width: 440px;
                    height: 44px;
                    background-color: rgba(173, 146, 132, 238);
                    border-width: 2px;
                    border-color: rgb(78, 54, 32);
                    display: block;
                    font-family: Open Sans;
                }
                
                .inventory-slot {
                    position: absolute;
                    width: 40px;
                    height: 40px;
                    display: inline-block;
                    text-align: right;
                    vertical-align: middle;
                    border-width: 2px;
                    border-color: rgb(114, 80, 76);
                    z-index: 10;
                    drag: clone;
                }
                
                .inventory-slot.selected {
                    border-width: 4px;
                    border-color: #FFD700;
                    z-index: 15;
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
			int left = i * 44;
			inventory_rml += "<div id='slot-" + std::to_string(i) + "' class='" + slot_class +
				"' style='left: " + std::to_string(left) + "px;'></div>";
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

		Rml::ElementList draggableSlots;
		m_inventory_document->GetElementsByClassName(draggableSlots, "inventory-slot");
		for (auto* el : draggableSlots) {
			DragListener::RegisterDragDropElement(el);
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
			int loc = i * 44;
			if (i == m_selected_slot) {
				slot_class += " selected";
				int selectedLoc = loc - 2;
				slot_element->SetProperty("left", std::to_string(selectedLoc) + "px");
				slot_element->SetProperty("top", "-2px");
			}
			else {
				slot_element->SetProperty("left", std::to_string(loc) + "px");
				slot_element->SetProperty("top", "0");
			}

			slot_element->SetAttribute("class", slot_class);

			std::string slot_content = "";
			//"<span style='color: #FFE4B5; font-size: 12px; font-weight: bold; position: absolute; top: 2px; left: 2px;'>" +
			//std::to_string(i + 1) + "</span>";

			// Add item display
			if (i < inventory.items.size()) {
				Entity item_entity = inventory.items[i];
				if (!registry.items.has(item_entity)) {
					continue;
				}

				Item& item = registry.items.get(item_entity);
				std::string tex = ITEM_INFO.count(item.type) ? ITEM_INFO.at(item.type).texture_path : "interactables/coffee_bean.png";
				slot_content += R"(
                    <img src=")" + tex + R"(" 
                    style='
                        pointer-events: none;
                        width: 32px; 
                        height: 32px; 
                        margin: 4px; 
                        transform: scaleY(-1); 
                    )";

				// Add color and star if potion
				if (item.type == ItemType::POTION) {
					Potion& potion = registry.potions.get(item_entity);
					slot_content += "image-color: " + getImageColorProperty(potion.color, 255) + ";'/>";
					
					PotionQuality pq = PotionSystem::getNormalizedQuality(potion);
					if (!isUselessEffect(potion.effect) && pq.threshold > 0) {
						std::string star_tex = pq.star_texture_path;
						slot_content += R"(
							<div style='
								pointer-events: none; 
								position: absolute; 
								bottom: 3px;
								left: 3px;
								width: 15px;
								height: 15px;
								decorator: image(")" + star_tex + R"(" flip-vertical fill);'>
							</div>)";
					}
				} else {
					slot_content += "' />";
				}

				// Add item count if more than 1
				if (item.amount > 1) {
					slot_content += R"(
                        <div style='
                            pointer-events: none; 
                            position: absolute; 
                            bottom: 0px;
                            right: -2px;
                            color: #FFFFFF; 
                            font-size: 14px; 
                            font-weight: bold;
							font-effect: outline( 1px black );'>
                        )" + std::to_string(item.amount) + R"(
                        </div>)";
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
	while (slot < 0) {
		slot += m_hotbar_size;
	}
	slot = slot % m_hotbar_size;

	if (registry.players.entities.size() == 0) return;
	Entity entity = registry.players.entities[0];
	if (!registry.inventories.has(entity)) return;
	Inventory& inventory = registry.inventories.get(entity);

	inventory.selection = slot;
	m_selected_slot = slot;

	// Update the inventory bar to reflect the selection
	if (m_inventory_document) {
		updateInventoryBar();
	}
}

int UISystem::getSlotFromId(std::string id)
{

	if (id.find("slot-") == std::string::npos) {
		return -1;
	}

	return std::stoi(id.substr(5));
}

void UISystem::updateTutorial()
{
	if (!m_initialized || !m_context || !registry.screenStates.components[0].tutorial_step_complete)
		return;

	try {
		ScreenState& screen = registry.screenStates.components[0];

		// Clear any previous tutorial step
		if (m_tutorial_document)
		{
			m_tutorial_document->Close();
			m_tutorial_document = nullptr;
		}

		if (screen.tutorial_state != (int)TUTORIAL::WELCOME_SCREEN) {
			if (registry.welcomeScreens.entities.size() > 0) {
				Entity& welcome_screen = registry.welcomeScreens.entities[0];
				registry.remove_all_components_of(welcome_screen);
			}
		}

		// Mark tutorial step as incomplete again
		screen.tutorial_step_complete = false;
		if (screen.tutorial_state == (int)TUTORIAL::COMPLETE ||
			screen.tutorial_state == (int)TUTORIAL::WELCOME_SCREEN) return;

		// Check if the tutorial state exists in the map
		auto it = tutorial_steps.find(screen.tutorial_state);
		if (it == tutorial_steps.end())
			return; // No tutorial for this state

		// Extract positions and text
		std::string left_position = std::get<0>(it->second);
		std::string top_position = std::get<1>(it->second);
		std::string tutorial_text = std::get<2>(it->second);
		std::cout << "UISystem::showTutorial - Creating tutorial step " << screen.tutorial_state << std::endl;

		std::string tutorial_rml = R"(
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
						position: absolute;
						top: )" + top_position + R"(;
						left: )" + left_position + R"(;
						transform: translate(-50%, -50%);
						text-align: center;
						font-size: 16px;
						background-color: #ffffff;
						font-family: Open Sans;
						padding: 5px;
						width: 250px;
						white-space: normal;
						color: #000000;
					}
				</style>
			</head>
			<body>
				<div class="text">)" + tutorial_text + R"(</div>
			</body>
			</rml>
		)";

		m_tutorial_document = m_context->LoadDocumentFromMemory(tutorial_rml.c_str());
		if (m_tutorial_document) {
			m_tutorial_document->Show();
			std::cout << "UISystem::updateTutorial - Tutorial step " << screen.tutorial_state << " created" << std::endl;
		}
		else {
			std::cerr << "UISystem::updateTutorial - Tutorial step " << screen.tutorial_state << " failed to be created" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in UISystem::updateTutorial: " << e.what() << std::endl;
	}
}

void UISystem::updateTextboxes()
{
	if (!m_initialized || !m_context)
		return;

	// Create new textboxes that should be visible
	for (const auto& [id, textbox] : textboxes)
	{
		if (m_textbox_documents.find(id) == m_textbox_documents.end())
		{
			createRmlUITextbox(id, textbox.text, textbox.pos);
		}
	}

	// Remove textboxes that are no longer needed
	std::vector<int> toRemove;
	for (const auto& [id, document] : m_textbox_documents)
	{
		if (textboxes.find(id) == textboxes.end())
		{
			toRemove.push_back(id);
		}
	}
	for (int id : toRemove)
	{
		removeRmlUITextbox(id);
	}

	textboxes.clear();
}

void UISystem::createRmlUITextbox(int id, std::string text, vec2 pos)
{
	if (!m_initialized || !m_context)
		return;

	try {
		std::string top_position = std::to_string(pos.y) + "px";
		std::string left_position = std::to_string(pos.x) + "px";

		std::string textbox_rml = R"(
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
						position: absolute;
						top: )" + top_position + R"(;
						left: )" + left_position + R"(;
						/* transform: translate(-50%, -50%); */
						text-align: center;
						font-size: 14px;
						background-color: #ffffffcc;
						font-family: Open Sans;
						padding: 5px;
						width: auto;
						max-width: 150px;
						white-space: normal;
						color: #000000;
    					border-radius: 5px;
					}
				</style>
			</head>
			<body>
				<div class="text">)" + text + R"(</div>
			</body>
			</rml>
		)";

		Rml::ElementDocument* document = m_context->LoadDocumentFromMemory(textbox_rml.c_str());
		if (document) {
			document->Show();
			m_textbox_documents[id] = document;  // Store the document in the map
			// std::cout << "UISystem::createTextbox created for id: " << id << std::endl;
		}
		else {
			std::cerr << "UISystem::createTextbox failed to be created" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in UISystem::createTextbox: " << e.what() << std::endl;
	}
}

void UISystem::removeRmlUITextbox(int id)
{
	if (!m_initialized || !m_context)
		return;

	auto it = m_textbox_documents.find(id);
	if (it != m_textbox_documents.end())
	{
		Rml::ElementDocument* document = it->second;
		document->Close();
		m_context->UnloadDocument(document);
		m_textbox_documents.erase(it);

		// std::cout << "UISystem::removeRmlUITextbox removed textbox with ID: " << id << std::endl;
	}
	else
	{
		std::cerr << "UISystem::removeRmlUITextbox called, but no document exists for ID: " << id << std::endl;
	}
}

bool UISystem::openCauldron(Entity cauldron)
{
	if (!m_initialized || !m_context) return false;
	if (m_cauldron_document) {
		m_cauldron_document->Show();
		SoundSystem::playInteractMenuSound((int)SOUND_CHANNEL::MENU, 0);
		if (DragListener::is_boiling) SoundSystem::continueBoilSound((int)SOUND_CHANNEL::BOILING, -1); // continue boiling if it was boiling before leaving menu
		return true;
	}

	// TODO: Use shader decorator to apply fluid effects
	try {
		std::cout << "UISystem::createCauldronUI - Creating cauldron UI" << std::endl;
		std::string cauldron_rml = R"(
        <rml>
        <head>
            <style>
                body {
                    position: absolute;
                    display: flex;
                    top: 25px;
                    left: 50%;
                    margin-left: -528px;
                    width: 1057px;
                    height: 550px;
                    decorator: image("interactables/cauldron_background.png" flip-vertical fill);
                }

                #heat {
                    position: relative;
                    width: 124px;
                    height: 100px;
                    top: 365px;
                    left: 108px;
                    decorator: image("interactables/heat_arrow.png" flip-vertical scale-none center bottom);
                    transform-origin: center 95% 0;
                    transform: scale(0.75) rotate(-60deg);
                    drag: drag;
                }

                #cauldron-water {
                    position: relative;
                    width: )" + std::to_string(CAULDRON_D) + R"(px;
                    height: )" + std::to_string(CAULDRON_D) + R"(px;
                    height: 316px;
                    top: 114px;
                    left: 243px;
                }

                #cauldron {
                    position: absolute;
                    width: )" + std::to_string(CAULDRON_D) + R"(px;
                    height: )" + std::to_string(CAULDRON_D) + R"(px;
                    top: 84px;
                    left: 406px;
                }

                #ladle {
                    position: absolute;
                    width: 132px;
                    height: 246px;
                    top: )" + LADLE_TOP_PX + R"(;
                    left: )" + LADLE_LEFT_PX + R"(;
                    decorator: image("interactables/spoon_on_table.png" contain);
                    drag: drag;
                }

                #bottle {
                    position: absolute;
                    width: 60px;
                    height: 100px;
                    top: )" + BOTTLE_TOP_PX + R"(;
                    left: )" + BOTTLE_LEFT_PX + R"(;
                    decorator: image("interactables/potion_bottle.png" contain);
                    transform: rotate(180deg) scale(1.2);
                    cursor: pointer;
                }
            </style>
        </head>
        <body>
            <div id="heat"></div>
            <div id="cauldron-water"></div>
            <div id="cauldron"></div>
            <div id="ladle"></div>
            <div id="bottle"></div>
        </body>
        </rml>
        )";

		m_cauldron_document = m_context->LoadDocumentFromMemory(cauldron_rml.c_str());
		if (!m_cauldron_document) {
			std::cerr << "UISystem::openCauldron - Failed to open cauldron" << std::endl;
			return false;
		}

		DragListener::RegisterDraggableElement(m_cauldron_document->GetElementById("heat"));
		DragListener::RegisterDraggableElement(m_cauldron_document->GetElementById("ladle"));
		DragListener::RegisterDragDropElement(m_cauldron_document->GetElementById("cauldron-water"));
		DragListener::RegisterDragDropElement(m_cauldron_document->GetElementById("cauldron"));
		m_cauldron_document->Show();
		SoundSystem::playInteractMenuSound((int)SOUND_CHANNEL::MENU, 0);
		openedCauldron = cauldron;
		registry.cauldrons.get(cauldron).filled = true;
		std::cout << "UISystem::openCauldron - Cauldron created successfully" << std::endl;
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in UISystem::openCauldron: " << e.what() << std::endl;
		return false;
	}
}

void UISystem::updateCauldronUI() {
	if (!m_cauldron_document || !isCauldronOpen()) {
		return;
	}

	// Update heatknob rotation
	int heat = registry.cauldrons.get(openedCauldron).heatLevel;
	float degree = heat * (MAX_KNOB_DEGREE * 2 / 100.f) - MAX_KNOB_DEGREE;
	Rml::Element* heatknob = m_cauldron_document->GetElementById("heat");
	std::string heatTrans = heatknob->GetProperty(Rml::PropertyId::Transform)->Get<Rml::String>();
	std::string before = heatTrans.substr(0, heatTrans.find("rotate"));
	std::stringstream s;
	s << before << "rotate(" << std::to_string(degree) << "deg)";
	heatknob->SetProperty("transform", s.str());
}

std::string UISystem::getImageColorProperty(vec3 color, float alpha) {
	std::stringstream s;
	s << "rgba(" << color.x << "," << color.y << "," << color.z << "," << alpha << ")";
	return s.str();
}

bool UISystem::isCauldronOpen() {
	return m_cauldron_document && m_cauldron_document->IsVisible();
}

bool UISystem::isCauldronOpen(Entity cauldron) {
	return isCauldronOpen() && cauldron == openedCauldron;
}

void UISystem::closeCauldron()
{
	if (isCauldronOpen()) {
		m_cauldron_document->Hide();
		SoundSystem::haltBoilSound();
		SoundSystem::playInteractMenuSound((int)SOUND_CHANNEL::MENU, 0);
		// handle exit menu tutorial
		if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::EXIT_MENU) {
			ScreenState& screen = registry.screenStates.components[0];
			screen.tutorial_step_complete = true;
			screen.tutorial_state += 1;
		}
	}
}

Entity UISystem::getOpenedCauldron() {
	return openedCauldron;
}

void UISystem::setOpenedCauldron(Entity new_cauldron) {
	openedCauldron = new_cauldron;
}

void UISystem::cauldronDragUpdate(bool isDown) {
	m_renderer->setIsMouseDragging(isDown);
}

// Added dummy bool cause vscode was doing some strange error
void UISystem::followMouse(Rml::Element* e, bool dummy) {
	int wl = e->GetProperty("width")->GetNumericValue().number;
	int hl = e->GetProperty("height")->GetNumericValue().number;
	int ix = (int)mouse_pos_x - wl / 2 - 96;
	int iy = (int)mouse_pos_y - hl / 2 - 25;
	e->SetProperty("left", std::to_string(ix) + "px");
	e->SetProperty("top", std::to_string(iy) + "px");
}

void UISystem::updateFollowMouse() {
	if (heldLadle) {
		followMouse(heldLadle, false);
		return;
	}

	if (heldBottle) {
		followMouse(heldBottle, false);
		return;
	}

	if (heldPestle) {
		followMouse(heldPestle, false);
		return;
	}
}

bool UISystem::openMortarPestle(Entity mortar) {
    if (!m_initialized || !m_context) return false;
    if (m_mortar_document) {
        m_mortar_document->Show();
        return true;
    }

    try {
        std::cout << "UISystem::openMortarPestle - Creating mortar & pestle UI" << std::endl;
        std::string mortar_rml = R"(
        <rml>
        <head>
            <style>
                body {
                    position: absolute;
                    display: flex;
                    top: 30px;
                    left: 50%;
                    margin-left: -550px;
                    width: 1100px;
                    height: 600px;
                    decorator: image("interactables/mortar_background_border.png" flip-vertical fill);
                }
				#pestle {
					position: absolute;
					width: 150px; 
					height: 200px;
					top: 300px;
					left: 800px;
					decorator: image("interactables/pestle.png" flip-vertical fill);
					drag: drag;
					z-index: 10;
				}
				#mortar_border {
					position: absolute;
					width: 400px;
					height: 400px;
					top: 80px;
					left: 350px;
					opacity: 0;
					decorator: image("interactables/mortar_border.png" flip-vertical fill);
					z-index: 5;
				}
                #mortar {
					position: absolute;
					width: 418px;
					height: 225px;
					top: 268px;
					left: 336px;
					decorator: image("interactables/mortar_frontpiece.png" flip-vertical fill);
					z-index: 20;
				}
            </style>
        </head>
        <body>
            <div id="mortar"></div>
			<div id="mortar_border"></div>
            <div id="pestle"></div>
        </body>
        </rml>
        )";

        m_mortar_document = m_context->LoadDocumentFromMemory(mortar_rml.c_str());
        if (!m_mortar_document) {
            std::cerr << "UISystem::openMortarPestle - Failed to open UI" << std::endl;
            return false;
        }

        DragListener::RegisterDragDropElement(m_mortar_document->GetElementById("mortar"));
		DragListener::RegisterDragDropElement(m_mortar_document->GetElementById("mortar_border"));
        DragListener::RegisterDraggableElement(m_mortar_document->GetElementById("pestle"));

        m_mortar_document->Show();
        openedMortar = mortar;
        std::cout << "UISystem::openMortarPestle - Mortar & Pestle UI created successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in UISystem::openMortarPestle: " << e.what() << std::endl;
        return false;
    }
}

bool UISystem::isMortarPestleOpen() {
	return m_mortar_document && m_mortar_document->IsVisible();
}

void UISystem::closeMortarPestle()
{
	if (isMortarPestleOpen()) {
		m_mortar_document->Hide();
	}
}

Entity UISystem::getOpenedMortarPestle() {
	return openedMortar;
}

void UISystem::setOpenedMortarPestle(Entity new_mortar_pestle) {
	openedMortar = new_mortar_pestle;
}

void UISystem::createHealthBar()
{
	if (!m_context) return;

	if (registry.players.entities.size() == 0) {
		std::cout << "UISystem::createHealthBar - No player to create health bar for" << std::endl;
		return;
	}

	try {
		std::cout << "UISystem::createHealthBar - Creating health bar" << std::endl;

		std::string healthbar_rml = R"(
			<rml>
			<head>
				<style>
					body {
						position: absolute;
						bottom: 10px;
						left: 97%;
						width: 20px;
						height: 180px;
						background-color: rgba(173, 146, 132, 238);
						border-width: 2px;
						border-color: rgb(78, 54, 32);
						display: block;
						font-family: Open Sans;
					}

					progress.vertical {
						width: 20px;
						height: 180px;
						background-color: transparent
					}

					.healthy fill {
						background-color:rgb(138, 247, 105);
					}

					.injured fill {
						background-color:rgb(246, 221, 97);
					}

					.dying fill {
						background-color:rgb(228, 103, 103);
					}

				</style>
			</head>
			<body>
				<progress id="health-bar" class="vertical" direction="top" max="1"></progress>
			</body>
			</rml>)";

		m_healthbar_document = m_context->LoadDocumentFromMemory(healthbar_rml.c_str());
		if (m_healthbar_document) {
			m_healthbar_document->Show();
			std::cout << "UISystem::createHealthBar - Health bar created successfully" << std::endl;
		}
		else {
			std::cerr << "UISystem::createHealthBar - Failed to create healthbar document" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Exception in UISystem::createHealthBar: " << e.what() << std::endl;
	}
}

void UISystem::updateHealthBar() {
	if (!m_initialized || !m_context || !m_healthbar_document) return;

	try {
		if (registry.players.entities.empty()) return;

		Entity player = registry.players.entities[0]; // Assuming there's only one player

		Rml::Element* healthbar_element = m_healthbar_document->GetElementById("health-bar");
		float hp_percentage = registry.players.components[0].health / PLAYER_MAX_HEALTH;
		healthbar_element->SetAttribute("value", hp_percentage);

		if (hp_percentage >= 0.5) {
			healthbar_element->SetAttribute("class", "vertical healthy");
		} else if (hp_percentage >= 0.2) {
			healthbar_element->SetAttribute("class", "vertical injured");
		} else {
			healthbar_element->SetAttribute("class", "vertical dying");
		}
		
	} catch (const std::exception& e) {
		std::cerr << "Exception in UISystem::updateHealthBar: " << e.what() << std::endl;
	}
}