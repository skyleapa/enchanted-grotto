#include "drag_listener.hpp"
#include "potion_system.hpp"
#include "item_system.hpp"
#include "sound_system.hpp"
#include "common.hpp"
#include <iostream>
#include <sstream>
#include <RmlUi/Core/Element.h>

static DragListener drag_listener;
UISystem* DragListener::m_ui_system;

void DragListener::RegisterDraggableElement(Rml::Element* element) {
	element->AddEventListener("dragstart", &drag_listener);
	element->AddEventListener("drag", &drag_listener);
	element->AddEventListener("dragend", &drag_listener);
}

void DragListener::RegisterDragDropElement(Rml::Element* element) {
	element->AddEventListener("dragdrop", &drag_listener);
}

float DragListener::getHeatDegree(Rml::Vector2f coords, float curDegree) {
	if (lastCoords == Rml::Vector2f(0, 0)) {
		return 0;
	}

	float dragFactor = 1.5; // decrease = more sensitive
	float xdiff = (coords.x - lastCoords.x) / dragFactor;
	float ydiff = (coords.y - lastCoords.y) / dragFactor;
	if (curDegree <= 0) {
		ydiff *= -1;
	}
	int res = abs(xdiff) > abs(ydiff) ? curDegree + xdiff : curDegree + ydiff;
	int clippedLow = max(res, -MAX_KNOB_DEGREE);
	return min(clippedLow, MAX_KNOB_DEGREE);
}

int DragListener::getHeatLevel(float degree) {
	return (int) ((degree + MAX_KNOB_DEGREE) / 1.2f);
}

float DragListener::getCurrentDegree(Rml::Element* heatknob) {
	std::string heatTrans = heatknob->GetProperty(Rml::PropertyId::Transform)->Get<Rml::String>();
	std::string rotateStr = heatTrans.substr(heatTrans.find("rotate"));
	return std::stof(rotateStr.substr(7, rotateStr.find("deg")));
}

void DragListener::setHeatDegree(float degree) {
	int heatLevel = getHeatLevel(degree);
	Entity cauldron = m_ui_system->getOpenedCauldron();
	registry.cauldrons.get(cauldron).heatLevel = heatLevel;
}

std::pair<float, float> DragListener::getPolarCoordinates(Rml::Vector2f input) {
	Rml::Vector2f cartesian = input - CAULDRON_CENTER;
	cartesian.y *= -1; // Since y starts from top-down we need to flip this
	float mag = cartesian.SquaredMagnitude();
	float angle = atan2f(cartesian.y, cartesian.x);
	return std::pair<float, float>(mag, angle);
}

void DragListener::checkCompletedStir() {
	// We know that stir has at least 2 elements
	int size = stirCoords.size();
	float curAngle = stirCoords[size - 1].second;
	float prevAngle = stirCoords[size - 2].second;
	float initialAngle = stirCoords[0].second;

	// Ignore crossings from -pi to positive pi (i.e. a large difference
	// between the two angles)
	if (abs(curAngle - prevAngle) > M_PI) {
		return;
	}

	// Check if initialAngle btwn cur and prev angle
	if (!((curAngle < initialAngle && initialAngle < prevAngle) ||
		(prevAngle < initialAngle && initialAngle < curAngle))) {
		return;
	}

	// Check vector locations, see if they have been in all the quadrants
	// a = quad 1, b = quad 2, c = quad 3, d = quad 4
	bool a = false, b = false, c = false, d = false;
	for (auto& polar : stirCoords) {
		if (polar.first > MAX_STIR_RADIUS || polar.first < MIN_STIR_RADIUS) {
			continue;
		}

		float angle = polar.second;
		if (!a && angle > 0 && angle < M_PI / 2) {
			a = true;
		}
		else if (!b && angle > M_PI / 2 && angle < M_PI) {
			b = true;
		}
		else if (!c && angle > -M_PI && angle < -M_PI / 2) {
			c = true;
		}
		else if (!d && angle < 0 && angle > -M_PI / 2) {
			d = true;
		}

		if (a && b && c && d) {
			PotionSystem::stirCauldron(m_ui_system->getOpenedCauldron());
			if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::STIR) {
				ScreenState& screen = registry.screenStates.components[0];
				screen.tutorial_step_complete = true;
				screen.tutorial_state += 1;
			}
			std::cout << "Recorded a successful ladle stir" << std::endl;
			SoundSystem::playStirSound((int) SOUND_CHANNEL::MENU, 0);
			break;
		}
	}

	// Reset stircoords no matter if a good stir was recorded or not, so player
	// can't get 1 stir through multiple circles
	std::pair<float, float> last = stirCoords[size - 1];
	stirCoords.clear();
	stirCoords.push_back(last);
}

void DragListener::endStir(Rml::Element* e) {
	m_ui_system->cauldronDragUpdate(false);
	e->SetProperty("decorator", "image(\"interactables/spoon_on_table.png\" contain)");
	stirCoords.clear();
}

void DragListener::checkGrindingMotion() {
	if (pestleMotion.size() < 3) {
		return;  // Not enough movement to detect a grind action
	}

	// std::cout << "pestleX: " << pestleX << ", pestleY: " << pestleY << std::endl;

	// Check if pestle is within the mortar square
	bool inMortar = (pestleX >= MORTAR_LEFT_X && pestleX <= MORTAR_RIGHT_X) &&
		(pestleY >= MORTAR_TOP_Y && pestleY <= MORTAR_BOTTOM_Y);

	if (!inMortar) {
		//std::cout << "not in mortar" << std::endl;
		return;
	}

	//std::cout << "Pestle in mortar" << std::endl;

	// Check for alternating up and down motion
	int up = 0;
	int down = 0;
	for (float move : pestleMotion) {
		if (move > 45) down++;
		if (move < -45) up++;
	}
	// std::cout << "Grinding action detected!" << " up: " << up << ", down: " << down << std::endl;

	if (up >= 4 && down >= 4) {
		// Grinding action detected
		std::cout << "Grinding done!" << std::endl;
		PotionSystem::grindIngredient(m_ui_system->getOpenedMortarPestle());

		// Clear motion data after successful grind
		pestleMotion.clear();
	}
}

void createTempRenderRequestForItem(Entity item) {
	if (!registry.renderRequests.has(item)) {
		if (!registry.items.has(item)) {
			std::cerr << "Attempted to assign RenderRequest to a non-item entity!" << std::endl;
			return;
		}

		Item& itemComp = registry.items.get(item);

		// Find the item texture from ITEM_INFO
		auto it = ITEM_INFO.find(itemComp.type);
		if (it == ITEM_INFO.end()) {
			std::cerr << "No ITEM_INFO found for item type: " << itemComp.name << std::endl;
			return;
		}

		const ItemInfo& itemInfo = it->second;

		registry.renderRequests.insert(
			item,
			{
				itemInfo.texture,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_LAYER::ITEM
			});

		if (!registry.motions.has(item)) {
			Motion& motion = registry.motions.emplace(item);
			motion.position = { 0, 0 };
			motion.scale = itemInfo.size;
		}

		// std::cout << "RenderRequest assigned to item: " << itemComp.name << std::endl;
	}
}

void DragListener::ProcessEvent(Rml::Event& event) {
	Rml::Element* cur = event.GetCurrentElement();
	Rml::Vector2f mouseCoords = event.GetUnprojectedMouseScreenPos();
	if (event == "dragstart") {
		if (cur->GetId() == "heat") {
			lastCoords = mouseCoords;
			return;
		}

		if (cur->GetId() == "ladle") {
			Rml::Context* context = cur->GetContext();
			Rml::Element* possibleCauldron = context->GetElementAtPoint(mouseCoords, cur);
			if (!possibleCauldron || possibleCauldron->GetId() != "cauldron") {
				event.StopImmediatePropagation();
				return;
			}

			cur->SetProperty("decorator", "image(\"interactables/spoon_in_hand.png\" flip-vertical contain)");
			stirCoords.push_back(getPolarCoordinates(mouseCoords));
			return;
		}

		if (cur->GetId() == "pestle") {
			Rml::Context* context = cur->GetContext();
			Rml::Element* possibleMortar = context->GetElementAtPoint(mouseCoords, cur);
			if (!possibleMortar || possibleMortar->GetId() != "mortar") {
				event.StopImmediatePropagation();
				return;
			}
	
			pestleY = mouseCoords.y;
			pestleX = mouseCoords.x;
			pestleMotion.clear();
			return;
		}
	}

	if (event == "drag") {
		if (cur->GetId() == "heat") {
			float curDegree = getCurrentDegree(cur);
			float newDegree = getHeatDegree(mouseCoords, curDegree);
			setHeatDegree(newDegree);
			if (registry.screenStates.components[0].tutorial_state == (int)TUTORIAL::SET_HEAT) {
				if (newDegree == 60) { // indicating max rotation
					ScreenState& screen = registry.screenStates.components[0];
					screen.tutorial_step_complete = true;
					screen.tutorial_state += 1;
				}
			}
			lastCoords = mouseCoords;
			if (!is_heat_changing) {
				is_heat_changing = true;
				SoundSystem::haltGeneralSound();
				if (registry.cauldrons.get(m_ui_system->getOpenedCauldron()).is_boiling) SoundSystem::continueBoilSound((int) SOUND_CHANNEL::BOILING, -1); // continue boiling if it was already boiling
				SoundSystem::playInteractMenuSound((int) SOUND_CHANNEL::MENU, -1); // play infinitely until dragging is finished
			}
			return;
		}

		// Where all the magic happens
		if (cur->GetId() == "ladle" && stirCoords.size()) {
			m_ui_system->cauldronDragUpdate(true);
			auto coords = getPolarCoordinates(mouseCoords);
			if (coords.first > MAX_STIR_RADIUS) {
				endStir(cur);
				return;
			}
			
			stirCoords.push_back(coords);
			checkCompletedStir();
			return;
		}

		if (cur->GetId() == "pestle") {
			float deltaY = mouseCoords.y - pestleY;
			pestleX = mouseCoords.x;
			pestleY = mouseCoords.y;
	
			pestleMotion.push_back(deltaY);
	
			// Check if enough vertical movement happened
			checkGrindingMotion();
			return;
		}
	}

	if (event == "dragend") {
		if (cur->GetId() == "heat") {
			float curDegree = getCurrentDegree(cur);
			int heatLevel = getHeatLevel(curDegree);
			PotionSystem::changeHeat(m_ui_system->getOpenedCauldron(), heatLevel);
			is_heat_changing = false;
			// play turn dial sound to signify completion of drag and start boiling
			if (heatLevel == 0) {
				SoundSystem::haltBoilSound(); // no boiling if setting temperature back to off
				SoundSystem::haltGeneralSound();
				registry.cauldrons.get(m_ui_system->getOpenedCauldron()).is_boiling = false;
			} else {
				// stsart boiling or continue boiling
				if (registry.cauldrons.get(m_ui_system->getOpenedCauldron()).is_boiling) SoundSystem::continueBoilSound((int) SOUND_CHANNEL::BOILING, -1);
				else {
					registry.cauldrons.get(m_ui_system->getOpenedCauldron()).is_boiling = true;
					SoundSystem::playBoilSound((int) SOUND_CHANNEL::BOILING, -1);
				}
			}
			SoundSystem::playTurnDialSound((int) SOUND_CHANNEL::MENU, 0);
			return;
		}

		if (cur->GetId() == "ladle" && stirCoords.size()) {
			endStir(cur);
			return;
		}

		if (cur->GetId() == "pestle") {
			pestleMotion.clear();
			return;
		}
	}

	if (event == "dragdrop") {
		// If item is dragged on a slot, swap items
		int slot = m_ui_system->getSlotFromId(cur->GetId());
		int selected = m_ui_system->getSelectedSlot();
		Entity player = registry.players.entities[0];
		if (slot != -1) {
			ItemSystem::swapItems(player, slot, selected);
			m_ui_system->updateInventoryBar();
			return;
		}

		if (!m_ui_system->isCauldronOpen() && !m_ui_system->isMortarPestleOpen()) {
			return;
		}

		// If item is dragged onto cauldron, insert 1 of that ingredient
		if (cur->GetId() == "cauldron" || cur->GetId() == "cauldron-water") {
			Inventory& pinv = registry.inventories.get(player);
			if (selected >= pinv.items.size()) {
				return;
			}

			// need to allow both ingredients and potions to be added to the cauldron
			Entity item = pinv.items[selected];
			if (!registry.ingredients.has(item) && !registry.potions.has(item)) {
				return;
			}

			// FOR NOW don't let water be dumped inside the cauldron
			// Change this when fillable cauldron gets added
			if (registry.potions.has(item) && registry.potions.get(item).effect == PotionEffect::WATER) {
				return;
			}

			Item& invItem = registry.items.get(item);
			Entity copy = ItemSystem::copyItem(item);
			invItem.amount -= 1;
			if (invItem.amount <= 0) {
				ItemSystem::removeItemFromInventory(player, item);
			}
			registry.items.get(copy).amount = 1;
			SoundSystem::playDropInCauldronSound((int) SOUND_CHANNEL::MENU, 0);
			PotionSystem::addIngredient(m_ui_system->getOpenedCauldron(), copy);
			return;
		}

		// If item is dragged onto mortar, insert 1 of that ingredient
		if (cur->GetId() == "mortar") {
			Inventory& pinv = registry.inventories.get(player);
			if (selected >= pinv.items.size()) {
				return;
			}

			Entity item = pinv.items[selected];
			if (!registry.ingredients.has(item)) {
				return;
			}
			
			Item& invItem = registry.items.get(item);

			Ingredient& curItem = registry.ingredients.get(item);
			if (curItem.grindLevel == -1.0 || curItem.grindLevel == 1.0f) {
				std::cerr << "Item is not grindable or already grinded" << std::endl;
				return;
			}

			Entity copy = ItemSystem::copyItem(item);
			invItem.amount -= 1;
			if (invItem.amount <= 0) {
				ItemSystem::removeItemFromInventory(player, item);
			}
			registry.items.get(copy).amount = 1;
			std::cout << "Added ingredient: " << invItem.name << " to mortar" << std::endl;

			createTempRenderRequestForItem(copy);

			PotionSystem::storeIngredientInMortar(m_ui_system->getOpenedMortarPestle(), copy);
			return;
		}
	}
}