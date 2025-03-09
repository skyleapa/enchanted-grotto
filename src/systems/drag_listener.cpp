#include "drag_listener.hpp"
#include "potion_system.hpp"
#include <iostream>
#include <sstream>
#include <RmlUi/Core/Element.h>

static DragListener drag_listener;
UISystem* DragListener::m_ui_system;

// Registers an element as being a container of draggable elements.
void DragListener::RegisterDraggableElement(Rml::Element* element) {
	element->AddEventListener("dragstart", &drag_listener);
	element->AddEventListener("drag", &drag_listener);
	element->AddEventListener("dragend", &drag_listener);
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
	int clippedLow = max(res, -MAX_DEGREE);
	return min(clippedLow, MAX_DEGREE);
}

int DragListener::getHeatLevel(float degree) {
	return (int) ((degree + MAX_DEGREE) / 1.2f);
}

float DragListener::getCurrentDegree(Rml::Element* heatknob) {
	std::string heatTrans = heatknob->GetProperty(Rml::PropertyId::Transform)->Get<Rml::String>();
	std::string rotateStr = heatTrans.substr(heatTrans.find("rotate"));
	return std::stof(rotateStr.substr(7, rotateStr.find("deg")));
}

void DragListener::setHeatDegree(Rml::Element* heatknob, float degree) {
	std::string heatTrans = heatknob->GetProperty(Rml::PropertyId::Transform)->Get<Rml::String>();
	std::string before = heatTrans.substr(0, heatTrans.find("rotate"));
	std::stringstream s;
	s << before << "rotate(" << std::to_string(degree) << "deg)";
	heatknob->SetProperty("transform", s.str());
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
	bool a, b, c, d = false;
	for (auto& polar : stirCoords) {
		if (polar.first > MAX_STIR_RADIUS || polar.first < MIN_STIR_RADIUS) {
			continue;
		}

		float angle = polar.second;
		if (!a && angle > 0 && angle < M_PI/2) {
			a = true;
		} else if (!b && angle > M_PI/2 && angle < M_PI) {
			b = true;
		} else if (!c && angle > -M_PI && angle < -M_PI/2) {
			c = true;
		} else if (!d && angle < 0 && angle > -M_PI/2) {
			d = true;
		}

		if (a && b && c && d) {
			PotionSystem::stirCauldron(m_ui_system->getOpenedCauldron());
			std::cout << "Recorded a successful ladle stir" << std::endl;
			break;
		}
	}

	// Reset stircoords no matter if a good stir was recorded or not, so player
	// can't get 1 stir through multiple circles
	std::pair<float, float> last = stirCoords[size - 1];
	stirCoords.clear();
	stirCoords.push_back(last);
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
	}

	if (event == "drag") {
		if (cur->GetId() == "heat") {
			float curDegree = getCurrentDegree(cur);
			float newDegree = getHeatDegree(mouseCoords, curDegree);
			setHeatDegree(cur, newDegree);
			lastCoords = mouseCoords;
			return;
		}

		// Where all the magic happens
		if (cur->GetId() == "ladle" && stirCoords.size()) {
			stirCoords.push_back(getPolarCoordinates(mouseCoords));
			checkCompletedStir();
			return;
		}
	}

	if (event == "dragend") {
		if (cur->GetId() == "heat") {
			float curDegree = getCurrentDegree(cur);
			int heatLevel = getHeatLevel(curDegree);
			PotionSystem::changeHeat(m_ui_system->getOpenedCauldron(), heatLevel);
			return;
		}	

		if (cur->GetId() == "ladle") {
			cur->SetProperty("decorator", "image(\"interactables/spoon_on_table.png\" flip-vertical contain)");
			stirCoords.clear();
			return;
		}
	}
}